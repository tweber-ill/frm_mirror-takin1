/*
 * cooper-nathans calculation
 * @author tweber
 * @date 01-may-2013
 * @copyright GPLv2
 *
 * @desc This is a reimplementation in C++ of the file rc_cnmat.m of the
 *		rescal5 package by Zinkin, McMorrow, Tennant, Farhi, and Wildes:
 *		http://www.ill.eu/en/instruments-support/computing-for-science/cs-software/all-software/matlab-ill/rescal-for-matlab/
 * @desc see: [cn67] M. J. Cooper and R. Nathans, Acta Cryst. 23, 357 (1967)
 */

#include "cn.h"
#include "ellipse.h"
#include "tlibs/math/linalg.h"
#include "tlibs/math/geo.h"
#include "tlibs/math/math.h"
#include "tlibs/helper/log.h"

#include <string>
#include <iostream>

#include <boost/units/io.hpp>


typedef units::quantity<units::si::plane_angle> angle;
typedef units::quantity<units::si::wavenumber> wavenumber;
typedef units::quantity<units::si::energy> energy;
typedef units::quantity<units::si::length> length;


CNResults calc_cn(CNParams& cn)
{
	CNResults res;

	if(!calc_tas_angles(cn, res))
		return res;

	// ------------------------------------------------------------------------------------------------
	// transformation matrix

	double dSi = cn.dsample_sense*units::sin(cn.angle_ki_Q);
	double dCi = units::cos(cn.angle_ki_Q);

	ublas::matrix<double> Ti(2,2);
	Ti(0,0)=dCi; Ti(0,1)=-dSi;
	Ti(1,0)=dSi; Ti(1,1)=dCi;


	double dSf = cn.dsample_sense*units::sin(cn.angle_kf_Q);
	double dCf = units::cos(cn.angle_kf_Q);

	ublas::matrix<double> Tf(2,2);
	Tf(0,0)=dCf; Tf(0,1)=-dSf;
	Tf(1,0)=dSf; Tf(1,1)=dCf;
	ublas::matrix<double> Tfm = -Tf;

	ublas::matrix<double> U = ublas::zero_matrix<double>(6,6);
	tl::submatrix_copy(U, Ti, 0, 0);
	tl::submatrix_copy(U, Tfm, 0, 3);
	U(2,2)=1.; U(2,5)=-1.;
	U(3,0)=2.*cn.ki*tl::angstrom*tl::KSQ2E;
	U(3,3)=-2.*cn.kf*tl::angstrom*tl::KSQ2E;
	U(4,0)=1.; U(5,2)=1.;

	ublas::matrix<double> V(6,6);
	if(!tl::inverse(U, V))
	{
		using namespace ublas;
		res.bOk = false;
		res.strErr = "Transformation matrix cannot be inverted.";
		return res;
	}

	// ------------------------------------------------------------------------------------------------



	// ------------------------------------------------------------------------------------------------
	// resolution matrix

	ublas::vector<double> pm(2);
	pm[0] = units::tan(cn.thetam);
	pm[1] = 1.;
	pm /= cn.ki*tl::angstrom * cn.mono_mosaic/units::si::radians;

	ublas::vector<double> pa(2);
	pa[0] = -units::tan(cn.thetaa);
	pa[1] = 1.;
	pa /= cn.kf*tl::angstrom * cn.ana_mosaic/units::si::radians;

	ublas::vector<double> palf0(2);
	palf0[0] = 2.*units::tan(cn.thetam);
	palf0[1] = 1.;
	palf0 /= (cn.ki*tl::angstrom * cn.coll_h_pre_mono/units::si::radians);

	ublas::vector<double> palf1(2);
	palf1[0] = 0;
	palf1[1] = 1.;
	palf1 /= (cn.ki*tl::angstrom * cn.coll_h_pre_sample/units::si::radians);

	ublas::vector<double> palf2(2);
	palf2[0] = -2.*units::tan(cn.thetaa);
	palf2[1] = 1.;
	palf2 /= (cn.kf*tl::angstrom * cn.coll_h_post_ana/units::si::radians);

	ublas::vector<double> palf3(2);
	palf3[0] = 0;
	palf3[1] = 1.;
	palf3 /= (cn.kf*tl::angstrom * cn.coll_h_post_sample/units::si::radians);

	ublas::matrix<double> m01(2,2);
	m01 = ublas::outer_prod(pm,pm) + ublas::outer_prod(palf0,palf0) + ublas::outer_prod(palf1,palf1);
	ublas::matrix<double> m34(2,2);
	m34 = ublas::outer_prod(pa,pa) + ublas::outer_prod(palf2,palf2) + ublas::outer_prod(palf3,palf3);

	ublas::matrix<double> M = ublas::zero_matrix<double>(6,6);
	tl::submatrix_copy(M, m01, 0, 0);
	tl::submatrix_copy(M, m34, 3, 3);

	M(2,2) = 1./(cn.ki*cn.ki * tl::angstrom*tl::angstrom) *
					(
							1./(cn.coll_v_pre_sample*cn.coll_v_pre_sample/units::si::radians/units::si::radians) +
							1./((2.*units::sin(cn.thetam)*cn.mono_mosaic/units::si::radians)*(2.*units::sin(cn.thetam)*cn.mono_mosaic/units::si::radians) + cn.coll_v_pre_mono*cn.coll_v_pre_mono/units::si::radians/units::si::radians)
					);
	M(5,5) = 1./(cn.kf*cn.kf * tl::angstrom*tl::angstrom) *
					(
							1./(cn.coll_v_post_sample*cn.coll_v_post_sample/units::si::radians/units::si::radians) +
							1./((2.*units::sin(cn.thetaa)*cn.ana_mosaic/units::si::radians)*(2.*units::sin(cn.thetaa)*cn.ana_mosaic/units::si::radians) + cn.coll_v_post_ana*cn.coll_v_post_ana/units::si::radians/units::si::radians)
					);
	// ------------------------------------------------------------------------------------------------

	ublas::matrix<double> M1 = ublas::prod(M, V);
	ublas::matrix<double> N = ublas::prod(ublas::trans(V), M1);

	N = ellipsoid_gauss_int(N, 5);
	N = ellipsoid_gauss_int(N, 4);

	ublas::vector<double> vec1 = tl::get_column<ublas::vector<double> >(N, 1);
	ublas::matrix<double> NP = N - ublas::outer_prod(vec1,vec1)
										/(1./((cn.sample_mosaic/units::si::radians * cn.Q*tl::angstrom)
										*(cn.sample_mosaic/units::si::radians * cn.Q*tl::angstrom))
											+ N(1,1));
	NP(2,2) = N(2,2);
	NP *= tl::SIGMA2FWHM*tl::SIGMA2FWHM;

	res.reso = NP;


	res.dR0 = 0.;	// TODO
	res.dResVol = tl::get_ellipsoid_volume(res.reso);

	calc_bragg_widths(cn, res);

	if(tl::is_nan_or_inf(res.dR0) || tl::is_nan_or_inf(res.reso))
	{
		res.strErr = "Invalid result.";
		res.bOk = false;
		return res;
	}

	res.bOk = true;
	return res;
}


void calc_bragg_widths(CNParams& cn, CNResults& res)
{
	for(unsigned int i=0; i<4; ++i)
		res.dBraggFWHMs[i] = tl::SIGMA2FWHM/sqrt(res.reso(i,i));
}


bool calc_tas_angles(CNParams& cn, CNResults& res)
{
	try
	{
		if(cn.bCalcE)
			cn.E = tl::get_energy_transfer(cn.ki, cn.kf);
		//std::cout << "E = " << (cn.E/one_meV) << std::endl;

		// TODO: check angles and scattering senses!
		cn.angle_ki_Q = tl::get_angle_ki_Q(cn.ki, cn.kf, cn.Q, cn.dsample_sense > 0.);
		cn.angle_kf_Q = /*M_PI*units::si::radians -*/ tl::get_angle_kf_Q(cn.ki, cn.kf, cn.Q, cn.dsample_sense > 0.);
		
		cn.angle_ki_Q = units::abs(cn.angle_ki_Q);
		cn.angle_kf_Q = units::abs(cn.angle_kf_Q);


		if(cn.bCalcMonoAnaAngles)
		{
			cn.thetaa = 0.5*tl::get_mono_twotheta(cn.kf, cn.ana_d, cn.dana_sense > 0.);
			cn.thetam = 0.5*tl::get_mono_twotheta(cn.ki, cn.mono_d, cn.dmono_sense > 0.);
		}
		//std::cout << double(cn.thetaa/units::si::radians)/M_PI*180. << ", " << double(cn.thetam/units::si::radians)/M_PI*180. << std::endl;

		if(cn.bCalcSampleAngles)
			cn.twotheta = tl::get_sample_twotheta(cn.ki, cn.kf, cn.Q, cn.dsample_sense>0.);
		//std::cout << double(cn.twotheta/units::si::radians)/M_PI*180. << std::endl;

		//if(cn.bCalcSampleAngles)
		{
			angle angleKiOrient1 = -cn.angle_ki_Q /*- vec_angle(vecQ)*/;	// only valid for Q along orient1
			cn.thetas = angleKiOrient1 + M_PI*units::si::radians;
			cn.thetas -= M_PI/2. * units::si::radians;
			if(!cn.dsample_sense) cn.thetas = -cn.thetas;
		}
		
		
		/*if(std::fabs(cn.twotheta/units::si::radians) > M_PI)
		{
			cn.angle_ki_Q = -cn.angle_ki_Q;
			cn.angle_kf_Q = -cn.angle_kf_Q;
			cn.twotheta = -(2.*M_PI*units::si::radians - cn.twotheta);
		}*/
		cn.twotheta = units::abs(cn.twotheta);

		/*tl::log_info("twotheta = ", cn.twotheta/units::si::radians / M_PI*180.);
		tl::log_info("kiQ = ", cn.angle_ki_Q/units::si::radians / M_PI*180.);
		tl::log_info("kfQ = ", cn.angle_kf_Q/units::si::radians / M_PI*180.);*/


		res.Q_avg.resize(4);
		res.Q_avg[0] = cn.Q*tl::angstrom;
		res.Q_avg[1] = 0.;
		res.Q_avg[2] = 0.;
		res.Q_avg[3] = cn.E / tl::one_meV;

		//std::copy(vecQ.begin(), vecQ.end(), std::ostream_iterator<double>(std::cout, " "));
		//std::cout << std::endl;
	}
	catch(const std::exception& ex)
	{
		res.bOk = false;
		res.strErr = ex.what();
		return false;
	}

	return true;
}
