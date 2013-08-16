/*
 * cooper-nathans calculation
 * @author tweber
 * @date 01-may-2013
 *
 * @desc This is a reimplementation in C++ of the file rc_cnmat.m of the
 *    			rescal5 package by Zinkin, McMorrow, Tennant, Farhi, and Wildes:
 *    			http://www.ill.eu/en/instruments-support/computing-for-science/cs-software/all-software/matlab-ill/rescal-for-matlab/
 */

#include "cn.h"
#include "../../helper/linalg.h"
#include "../../helper/math.h"

#include <string>
#include <iostream>

#include <boost/units/io.hpp>
#include "formulas.h"


typedef units::quantity<units::si::plane_angle> angle;
typedef units::quantity<units::si::wavenumber> wavenumber;
typedef units::quantity<units::si::energy> energy;
typedef units::quantity<units::si::length> length;

/*
 * this is a 1:1 C++ reimplementation of 'rc_int' from 'mcresplot' and 'rescal5'
 * integrate over row/column iIdx
 */
ublas::matrix<double> gauss_int(const ublas::matrix<double>& mat, unsigned int iIdx)
{
	unsigned int iSize = mat.size1();
	ublas::vector<double> b(iSize);

	for(unsigned int i=0; i<iSize; ++i)
		b[i] = mat(i,iIdx) + mat(iIdx,i);

	b = remove_elem(b, iIdx);
	ublas::matrix<double> m = remove_elems(mat, iIdx);
	ublas::matrix<double> bb = outer_prod(b,b);

	double d = mat(iIdx, iIdx);
	m = m - 0.25/d * bb;
	return m;
}


CNResults calc_cn(CNParams& cn)
{
	CNResults res;

	if(!calc_cn_angles(cn, res))
		return res;

	// ------------------------------------------------------------------------------------------------
	// transformation matrix

	double dSi = cn.dsample_sense*units::sin(res.angle_ki_Q);
	double dCi = units::cos(res.angle_ki_Q);

	ublas::matrix<double> Ti(2,2);
	Ti(0,0)=dCi; Ti(0,1)=-dSi;
	Ti(1,0)=dSi; Ti(1,1)=dCi;


	double dSf = cn.dsample_sense*units::sin(res.angle_kf_Q);
	double dCf = units::cos(res.angle_kf_Q);

	ublas::matrix<double> Tf(2,2);
	Tf(0,0)=dCf; Tf(0,1)=-dSf;
	Tf(1,0)=dSf; Tf(1,1)=dCf;
	ublas::matrix<double> Tfm = -Tf;

	ublas::matrix<double> U = ublas::zero_matrix<double>(6,6);
	submatrix_copy(U, Ti, 0, 0);
	submatrix_copy(U, Tfm, 0, 3);
	U(2,2)=1.; U(2,5)=-1.;
	U(3,0)=2.*cn.ki*angstrom*KSQ2E;
	U(3,3)=-2.*cn.kf*angstrom*KSQ2E;
	U(4,0)=1.; U(5,2)=1.;

	ublas::matrix<double> V(6,6);
	if(!::inverse(U, V))
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
	pm[0] = units::tan(res.thetam);
	pm[1] = 1.;
	pm /= cn.ki*angstrom * cn.mono_mosaic/units::si::radians;

	ublas::vector<double> pa(2);
	pa[0] = -units::tan(res.thetaa);
	pa[1] = 1.;
	pa /= cn.kf*angstrom * cn.ana_mosaic/units::si::radians;

	ublas::vector<double> palf0(2);
	palf0[0] = 2.*units::tan(res.thetam);
	palf0[1] = 1.;
	palf0 /= (cn.ki*angstrom * cn.coll_h_pre_mono/units::si::radians);

	ublas::vector<double> palf1(2);
	palf1[0] = 0;
	palf1[1] = 1.;
	palf1 /= (cn.ki*angstrom * cn.coll_h_pre_sample/units::si::radians);

	ublas::vector<double> palf2(2);
	palf2[0] = -2.*units::tan(res.thetaa);
	palf2[1] = 1.;
	palf2 /= (cn.kf*angstrom * cn.coll_h_post_ana/units::si::radians);

	ublas::vector<double> palf3(2);
	palf3[0] = 0;
	palf3[1] = 1.;
	palf3 /= (cn.kf*angstrom * cn.coll_h_post_sample/units::si::radians);

	ublas::matrix<double> m01(2,2);
	m01 = ublas::outer_prod(pm,pm) + ublas::outer_prod(palf0,palf0) + ublas::outer_prod(palf1,palf1);
	ublas::matrix<double> m34(2,2);
	m34 = ublas::outer_prod(pa,pa) + ublas::outer_prod(palf2,palf2) + ublas::outer_prod(palf3,palf3);

	ublas::matrix<double> M = ublas::zero_matrix<double>(6,6);
	submatrix_copy(M, m01, 0, 0);
	submatrix_copy(M, m34, 3, 3);

	M(2,2) = 1./(cn.ki*cn.ki * angstrom*angstrom) *
					(
							1./(cn.coll_v_pre_sample*cn.coll_v_pre_sample/units::si::radians/units::si::radians) +
							1./((2.*units::sin(res.thetam)*cn.mono_mosaic/units::si::radians)*(2.*units::sin(res.thetam)*cn.mono_mosaic/units::si::radians) + cn.coll_v_pre_mono*cn.coll_v_pre_mono/units::si::radians/units::si::radians)
					);
	M(5,5) = 1./(cn.kf*cn.kf * angstrom*angstrom) *
					(
							1./(cn.coll_v_post_sample*cn.coll_v_post_sample/units::si::radians/units::si::radians) +
							1./((2.*units::sin(res.thetaa)*cn.ana_mosaic/units::si::radians)*(2.*units::sin(res.thetaa)*cn.ana_mosaic/units::si::radians) + cn.coll_v_post_ana*cn.coll_v_post_ana/units::si::radians/units::si::radians)
					);
	// ------------------------------------------------------------------------------------------------

	ublas::matrix<double> M1 = ublas::prod(M, V);
	ublas::matrix<double> N = ublas::prod(ublas::trans(V), M1);

	N = ::gauss_int(N, 5);
	N = ::gauss_int(N, 4);

	ublas::vector<double> vec1 = ::get_column<ublas::vector<double> >(N, 1);
	ublas::matrix<double> NP = N - ublas::outer_prod(vec1,vec1)
													/(1./((cn.sample_mosaic/units::si::radians * cn.Q*angstrom)
													*(cn.sample_mosaic/units::si::radians * cn.Q*angstrom))
														+ N(1,1));
	NP(2,2) = N(2,2);
	NP *= SIGMA2FWHM*SIGMA2FWHM;

	res.reso = NP;

	calc_bragg_widths(cn, res);
	calc_cn_vol(cn, res);

	if(std::isnan(res.dR0) || isnan(res.reso))
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
		res.dBraggFWHMs[i] = SIGMA2FWHM/sqrt(res.reso(i,i));
}


bool calc_cn_angles(CNParams& cn, CNResults& res)
{
	bool bImag=0;
	wavenumber E_as_k = E2k(cn.E, bImag);
	double dSign = 1.;
	if(bImag) dSign = -1.;

	if(cn.bki_fix)
		cn.kf = units::sqrt(cn.ki*cn.ki - dSign*E_as_k*E_as_k);
	else
		cn.ki = units::sqrt(cn.kf*cn.kf + dSign*E_as_k*E_as_k);

	res.angle_ki_Q = get_angle_ki_Q(cn.ki, cn.kf, cn.Q);
	res.angle_kf_Q = get_angle_kf_Q(cn.ki, cn.kf, cn.Q);

	res.thetaa = cn.dana_sense*units::asin(M_PI/(cn.ana_d*cn.kf));
	res.thetam = cn.dmono_sense*units::asin(M_PI/(cn.mono_d*cn.ki));

	/*std::cout << "k_i = " << cn.ki << ", "
	  	  	  << "k_f = " << cn.kf << ", "
	  		  << "theta_m = " << res.thetam << ", "
			  << "theta_a = " << res.thetaa << ", "
			  << "lam_i = " << k2lam(cn.ki)
			  << std::endl;*/

	units::quantity<units::si::dimensionless> dttCos = (cn.ki*cn.ki + cn.kf*cn.kf - cn.Q*cn.Q)/(2.*cn.ki*cn.kf);
	if(units::abs(dttCos) > 1)
	{
		res.bOk = false;
		res.strErr = "Scattering triangle not closed.";
		return false;
	}
	res.twotheta = cn.dsample_sense*units::acos(dttCos);
	res.thetas = res.twotheta/2.;
	//get_twotheta(cn.ki, cn.kf, cn.Q, res.twotheta);


	ublas::vector<double> vecKi(2);
	vecKi[0] = 1.; vecKi[1] = 0.;
	ublas::matrix<double> rot_kikf = rotation_matrix_2d(res.twotheta/units::si::radians);
	ublas::vector<double> vecKf = ublas::prod(rot_kikf, vecKi) * (cn.kf/cn.ki);

	ublas::vector<double> vecQ = vecKf - vecKi;

	res.Q_avg.resize(4);
	res.Q_avg[0] = vecQ[0];
	res.Q_avg[1] = vecQ[1];
	res.Q_avg[2] = 0.;
	res.Q_avg[3] = cn.E / one_meV;

	return true;
}


void calc_cn_vol(CNParams& cn, CNResults& res)
{
	// TODO: Look in Shirane for factor
	const double dFactor = 15.75;

	if(cn.bConstMon)
	{
		res.dR0_vi = 1.;
	}
	else
	{
		res.dR0_vi = cn.dmono_refl * std::pow(cn.ki*angstrom, 3.);
		res.dR0_vi *= units::cos(res.thetam)/units::sin(res.thetam);
		res.dR0_vi *= cn.coll_v_pre_mono * cn.coll_v_pre_sample * cn.coll_h_pre_mono * cn.coll_h_pre_sample
								/ units::si::radians/units::si::radians/units::si::radians/units::si::radians;
		res.dR0_vi *= dFactor * cn.mono_mosaic / units::si::radians;

		res.dR0_vi /= std::sqrt(
						std::pow(2.*units::sin(res.thetam) * cn.mono_mosaic/units::si::radians, 2.)
						+ std::pow(cn.coll_v_pre_mono/units::si::radians, 2.)
						+ std::pow(cn.coll_v_pre_sample/units::si::radians, 2.)
						);
		res.dR0_vi /= std::sqrt(
						std::pow(cn.coll_h_pre_mono/units::si::radians, 2.)
						+ std::pow(cn.coll_h_pre_sample/units::si::radians, 2.)
						+ std::pow(2.*cn.mono_mosaic/units::si::radians, 2.)
						);
		res.dR0_vi = fabs(res.dR0_vi);
	}

	res.dR0_vf = cn.dana_effic * std::pow(cn.kf*angstrom, 3.);
	res.dR0_vf *= units::cos(res.thetaa)/units::sin(res.thetaa);
	res.dR0_vf *= cn.coll_v_post_sample * cn.coll_v_post_ana * cn.coll_h_post_sample * cn.coll_h_post_ana
							/ units::si::radians/units::si::radians/units::si::radians/units::si::radians;
	res.dR0_vf *= dFactor * cn.ana_mosaic / units::si::radians;

	res.dR0_vf /= std::sqrt(
					std::pow(2.*units::sin(res.thetaa) * cn.ana_mosaic/units::si::radians, 2.)
					+ std::pow(cn.coll_v_post_sample/units::si::radians, 2.)
					+ std::pow(cn.coll_v_post_ana/units::si::radians, 2.)
					);

	res.dR0_vf /= std::sqrt(
					std::pow(cn.coll_h_post_sample/units::si::radians, 2.)
					+ std::pow(cn.coll_h_post_ana/units::si::radians, 2.)
					+ std::pow(2.*cn.ana_mosaic/units::si::radians, 2.)
					);
	res.dR0_vf = fabs(res.dR0_vf);

	const double dResDet = determinant(res.reso);
	res.dR0 = res.dR0_vi*res.dR0_vf*std::sqrt(dResDet) / (2.*M_PI * 2.*M_PI);
	res.dR0 /= cn.sample_mosaic/units::si::radians *
					std::sqrt(
							1./std::pow(cn.sample_mosaic/units::si::radians, 2.)
							+ std::pow(cn.Q*angstrom, 2.)
								* res.reso(1,1)/(SIGMA2FWHM*SIGMA2FWHM)
							);
	res.dR0 = fabs(res.dR0);
}
