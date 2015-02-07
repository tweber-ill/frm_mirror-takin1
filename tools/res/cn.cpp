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

using tl::angle; using tl::wavenumber; using tl::energy; using tl::length;
static const auto angs = tl::angstrom;
static const auto rads = tl::radians;
static const auto meV = tl::one_meV;


CNResults calc_cn(CNParams& cn)
{
	CNResults res;

	res.Q_avg.resize(4);
	res.Q_avg[0] = cn.Q * angs;
	res.Q_avg[1] = 0.;
	res.Q_avg[2] = 0.;
	res.Q_avg[3] = cn.E / meV;


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
	U(3,0)=2.*cn.ki * angs * tl::KSQ2E;
	U(3,3)=-2.*cn.kf * angs * tl::KSQ2E;
	U(4,0)=1.; U(5,2)=1.;

	ublas::matrix<double> V(6,6);
	if(!tl::inverse(U, V))
	{
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
	pm /= cn.ki * angs * cn.mono_mosaic/rads;

	ublas::vector<double> pa(2);
	pa[0] = -units::tan(cn.thetaa);
	pa[1] = 1.;
	pa /= cn.kf * angs * cn.ana_mosaic/rads;

	ublas::vector<double> palf0(2);
	palf0[0] = 2.*units::tan(cn.thetam);
	palf0[1] = 1.;
	palf0 /= (cn.ki*angs * cn.coll_h_pre_mono/rads);

	ublas::vector<double> palf1(2);
	palf1[0] = 0;
	palf1[1] = 1.;
	palf1 /= (cn.ki*angs * cn.coll_h_pre_sample/rads);

	ublas::vector<double> palf2(2);
	palf2[0] = -2.*units::tan(cn.thetaa);
	palf2[1] = 1.;
	palf2 /= (cn.kf*angs * cn.coll_h_post_ana/rads);

	ublas::vector<double> palf3(2);
	palf3[0] = 0;
	palf3[1] = 1.;
	palf3 /= (cn.kf*angs * cn.coll_h_post_sample/rads);

	ublas::matrix<double> m01(2,2);
	m01 = ublas::outer_prod(pm,pm) +
			ublas::outer_prod(palf0,palf0) +
			ublas::outer_prod(palf1,palf1);
	ublas::matrix<double> m34(2,2);
	m34 = ublas::outer_prod(pa,pa) +
			ublas::outer_prod(palf2,palf2) +
			ublas::outer_prod(palf3,palf3);

	ublas::matrix<double> M = ublas::zero_matrix<double>(6,6);
	tl::submatrix_copy(M, m01, 0, 0);
	tl::submatrix_copy(M, m34, 3, 3);

	M(2,2) = 1./(cn.ki*cn.ki * angs*angs) *
		(
			1./(cn.coll_v_pre_sample*cn.coll_v_pre_sample/rads/rads) +
			1./((2.*units::sin(cn.thetam)*cn.mono_mosaic/rads)*(2.*units::sin(cn.thetam)*cn.mono_mosaic/rads) +
				cn.coll_v_pre_mono*cn.coll_v_pre_mono/rads/rads)
		);
	M(5,5) = 1./(cn.kf*cn.kf * angs*angs) *
		(
			1./(cn.coll_v_post_sample*cn.coll_v_post_sample/rads/rads) +
			1./((2.*units::sin(cn.thetaa)*cn.ana_mosaic/rads)*(2.*units::sin(cn.thetaa)*cn.ana_mosaic/rads) +
				cn.coll_v_post_ana*cn.coll_v_post_ana/rads/rads)
		);
	// ------------------------------------------------------------------------------------------------

	ublas::matrix<double> M1 = ublas::prod(M, V);
	ublas::matrix<double> N = ublas::prod(ublas::trans(V), M1);

	N = ellipsoid_gauss_int(N, 5);
	N = ellipsoid_gauss_int(N, 4);

	ublas::vector<double> vec1 = tl::get_column<ublas::vector<double> >(N, 1);
	ublas::matrix<double> NP = N - ublas::outer_prod(vec1,vec1)
										/(1./((cn.sample_mosaic/rads * cn.Q*angs)
										*(cn.sample_mosaic/rads * cn.Q*angs))
											+ N(1,1));
	NP(2,2) = N(2,2);
	NP *= tl::SIGMA2FWHM*tl::SIGMA2FWHM;

	res.reso = NP;


	res.dR0 = 0.;	// TODO
	res.dResVol = tl::get_ellipsoid_volume(res.reso);

	// Bragg widths
	for(unsigned int i=0; i<4; ++i)
		res.dBraggFWHMs[i] = tl::SIGMA2FWHM/sqrt(res.reso(i,i));

	if(tl::is_nan_or_inf(res.dR0) || tl::is_nan_or_inf(res.reso))
	{
		res.strErr = "Invalid result.";
		res.bOk = false;
		return res;
	}

	res.bOk = true;
	return res;
}
