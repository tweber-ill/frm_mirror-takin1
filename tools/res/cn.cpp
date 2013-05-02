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

#include <string>
#include <iostream>

#include <boost/units/io.hpp>
#include "formulas.h"


#define one_meV (1e-3 * codata::e * units::si::volts)
#define ang (1e-10 * units::si::meter)
static const double KSQ2E = (codata::hbar*codata::hbar / (2.*codata::m_n)) / one_meV / (ang*ang);
static const double E2KSQ = 1./KSQ2E;

static const double SIGMA2FWHM = 2.*sqrt(2.*log(2.));


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

        double d = mat(iIdx, iIdx);

        ublas::matrix<double> bb(iSize-1, iSize-1);
        bb = outer_prod(b,b);

        m = m - 1./(4.*d)*bb;
        return m;
}


CNResults calc_cn(CNParams& cn)
{
	typedef units::quantity<units::si::plane_angle> angle;
	typedef units::quantity<units::si::wavenumber> wavenumber;
	typedef units::quantity<units::si::energy> energy;
	typedef units::quantity<units::si::length> length;

	const units::quantity<units::si::length> angstrom = 1e-10 * units::si::meter;

	CNResults res;

	if(cn.bki_fix)
		cn.kf = units::sqrt(cn.ki*cn.ki - E2k(cn.E)*E2k(cn.E));
	else
		cn.ki = units::sqrt(cn.kf*cn.kf + E2k(cn.E)*E2k(cn.E));

	if(!::get_twotheta(cn.ki, cn.kf, cn.Q, res.twotheta))
	{
		res.bOk = false;
		res.strErr = "Scattering triangle not closed.";
		return res;
	}

	// ------------------------------------------------------------------------------------------------
	// transformation matrix

	res.angle_ki_Q = get_angle_ki_Q(cn.ki, cn.kf, cn.Q);
	res.angle_kf_Q = get_angle_kf_Q(cn.ki, cn.kf, cn.Q);


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


	ublas::matrix<double> U = ublas::zero_matrix<double>(6,6);
	U(0,0)=Ti(0,0); U(0,1)=Ti(0,1);
	U(1,0)=Ti(1,0); U(1,1)=Ti(1,1);

	U(0,3)=-Tf(0,0); U(0,4)=-Tf(0,1);
	U(1,3)=-Tf(1,0); U(1,4)=-Tf(1,1);

	U(2,2)=1.; U(2,5)=-1.;

	U(3,0)=2.*cn.ki*angstrom*KSQ2E;
	U(3,3)=-2.*cn.kf*angstrom*KSQ2E;
	U(4,0)=1.; U(5,2)=1.;

	ublas::matrix<double> V(6,6);
	if(!::inverse(U, V))
	{
		res.bOk = false;
		res.strErr = "Transformation matrix cannot be inverted.";
	}

	//std::cout << "U=" << U << std::endl;
	//std::cout << "V=" << V << std::endl;

	// ------------------------------------------------------------------------------------------------



	// ------------------------------------------------------------------------------------------------
	// resolution matrix

	angle thetaa = units::asin(M_PI/(cn.ana_d*cn.kf));
	angle thetam = units::asin(M_PI/(cn.mono_d*cn.ki));


	ublas::vector<double> pm(2);
	pm[0] = cn.dmono_sense*units::tan(thetam);
	pm[1] = 1.;
	pm /= cn.ki*angstrom * cn.mono_mosaic/units::si::radians;

	ublas::vector<double> pa(2);
	pa[0] = -cn.dana_sense*units::tan(thetaa);
	pa[1] = 1.;
	pa /= cn.kf*angstrom * cn.ana_mosaic/units::si::radians;

	ublas::vector<double> palf0(2);
	palf0[0] = 2.*cn.dmono_sense*units::tan(thetam);
	palf0[1] = 1.;
	palf0 /= (cn.ki*angstrom * cn.coll_h_pre_mono/units::si::radians);

	ublas::vector<double> palf1(2);
	palf1[0] = 0;
	palf1[1] = 1.;
	palf1 /= (cn.ki*angstrom * cn.coll_h_pre_sample/units::si::radians);

	ublas::vector<double> palf2(2);
	palf2[0] = -2.*cn.dana_sense*units::tan(thetaa);
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
	M(0,0) = m01(0,0); M(0,1) = m01(0,1);
	M(1,0) = m01(1,0); M(1,1) = m01(1,1);
	M(3,3) = m34(0,0); M(3,4) = m34(0,1);
	M(4,3) = m34(1,0); M(4,4) = m34(1,1);

	M(2,2) = 1./(cn.ki*cn.ki * angstrom*angstrom) *
					(
							1./(cn.coll_v_pre_sample*cn.coll_v_pre_sample/units::si::radians/units::si::radians) +
							1./((2.*units::sin(thetam)*cn.mono_mosaic/units::si::radians)*(2.*units::sin(thetam)*cn.mono_mosaic/units::si::radians) + cn.coll_v_pre_mono*cn.coll_v_pre_mono/units::si::radians/units::si::radians)
					);
	M(5,5) = 1./(cn.kf*cn.kf * angstrom*angstrom) *
					(
							1./(cn.coll_v_post_sample*cn.coll_v_post_sample/units::si::radians/units::si::radians) +
							1./((2.*units::sin(thetaa)*cn.ana_mosaic/units::si::radians)*(2.*units::sin(thetaa)*cn.ana_mosaic/units::si::radians) + cn.coll_v_post_ana*cn.coll_v_post_ana/units::si::radians/units::si::radians)
					);
	// ------------------------------------------------------------------------------------------------


	ublas::matrix<double> M1 = ublas::prod(M, V);
	ublas::matrix<double> N = ublas::prod(ublas::trans(V), M1);

	N = ::gauss_int(N, 5);
	N = ::gauss_int(N, 4);

	ublas::vector<double> vec1 = ::get_column<ublas::vector<double> >(N, 1);

	ublas::matrix<double> NP = N - ublas::outer_prod(vec1,vec1);
	NP /= 1/((cn.sample_mosaic/units::si::radians * cn.Q*angstrom)*(cn.sample_mosaic/units::si::radians * cn.Q*angstrom))
			+ N(1,1);
	NP(2,2) = N(2,2);

	NP *= SIGMA2FWHM*SIGMA2FWHM;

	res.reso = NP;



	// ------------------------------------------------------------------------------------------------
	// resolution volume

	const double dFactor = 15.75;

	if(cn.bConstMon)
	{
		res.dR0_vi = 1.;
	}
	else
	{
		res.dR0_vi = cn.dmono_refl * cn.ki*cn.ki*cn.ki * angstrom*angstrom*angstrom;
		res.dR0_vi *= units::cos(thetam)/units::sin(thetam);
		res.dR0_vi *= cn.coll_v_pre_mono * cn.coll_v_pre_sample * cn.coll_h_pre_mono * cn.coll_h_pre_sample / units::si::radians/units::si::radians/units::si::radians/units::si::radians;
		res.dR0_vi *= dFactor * cn.mono_mosaic / units::si::radians;

		res.dR0_vi /= units::sqrt(
						(2.*units::sin(thetam) * cn.mono_mosaic/units::si::radians)*
						(2.*units::sin(thetam) * cn.mono_mosaic/units::si::radians)
						+ cn.coll_v_pre_mono*cn.coll_v_pre_mono / units::si::radians/units::si::radians
						+ cn.coll_v_pre_sample*cn.coll_v_pre_sample / units::si::radians/units::si::radians
						);
		res.dR0_vi /= units::sqrt(
						cn.coll_h_pre_mono*cn.coll_h_pre_mono / units::si::radians/units::si::radians
						+ cn.coll_h_pre_sample*cn.coll_h_pre_sample / units::si::radians/units::si::radians
						+ 4.*cn.mono_mosaic*cn.mono_mosaic / units::si::radians/units::si::radians
						);
	}

	res.dR0_vf = cn.dana_effic * cn.kf*cn.kf*cn.kf * angstrom*angstrom*angstrom;
	res.dR0_vf *= units::cos(thetaa)/units::sin(thetaa);
	res.dR0_vf *= cn.coll_v_post_sample * cn.coll_v_post_ana * cn.coll_h_post_sample * cn.coll_h_post_ana / units::si::radians/units::si::radians/units::si::radians/units::si::radians;
	res.dR0_vf *= dFactor * cn.ana_mosaic / units::si::radians;

	res.dR0_vf /= units::sqrt(
					(2.*units::sin(thetaa) * cn.ana_mosaic/units::si::radians)*
					(2.*units::sin(thetaa) * cn.ana_mosaic/units::si::radians)
					+ cn.coll_v_post_sample*cn.coll_v_post_sample / units::si::radians/units::si::radians
					+ cn.coll_v_post_ana*cn.coll_v_post_ana / units::si::radians/units::si::radians
					);
	res.dR0_vf /= units::sqrt(
					cn.coll_h_post_sample*cn.coll_h_post_sample / units::si::radians/units::si::radians
					+ cn.coll_h_post_ana*cn.coll_h_post_ana / units::si::radians/units::si::radians
					+ 4.*cn.ana_mosaic*cn.ana_mosaic / units::si::radians/units::si::radians
					);

	const double dResDet = /*fabs*/(determinant(res.reso));
	res.dR0 = res.dR0_vi*res.dR0_vf*::sqrt(dResDet) / (2.*M_PI * 2.*M_PI);
	res.dR0 /= cn.sample_mosaic/units::si::radians *
					units::sqrt(
							1./(cn.sample_mosaic/units::si::radians * cn.sample_mosaic/units::si::radians)
							+ cn.Q*cn.Q * angstrom*angstrom * N(1,1));

	res.bOk = true;
	return res;
}



/*
int main()
{
	CNParams cn;
	CNResults res = calc_cn(cn);

	return 0;
}
*/
