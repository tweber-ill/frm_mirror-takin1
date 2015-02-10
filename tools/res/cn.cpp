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

typedef double t_real;
typedef ublas::matrix<t_real> t_mat;
typedef ublas::vector<t_real> t_vec;

using tl::angle; using tl::wavenumber; using tl::energy; using tl::length;
static const auto angs = tl::angstrom;
static const auto rads = tl::radians;
static const auto meV = tl::one_meV;


CNResults calc_cn(const CNParams& cn)
{
	CNResults res;

	res.Q_avg.resize(4);
	res.Q_avg[0] = cn.Q * angs;
	res.Q_avg[1] = 0.;
	res.Q_avg[2] = 0.;
	res.Q_avg[3] = cn.E / meV;


	// -------------------------------------------------------------------------
	// transformation matrix
	
	angle twotheta = cn.twotheta;
	angle thetaa = cn.thetaa;
	angle thetam = cn.thetam;
	angle ki_Q = cn.angle_ki_Q;
	angle kf_Q = cn.angle_kf_Q;
	//kf_Q = twotheta + ki_Q;

	if(cn.dsample_sense < 0) 
	{ 
		twotheta = -twotheta; 
		ki_Q = -ki_Q; 
		kf_Q = -kf_Q; 
	}
	if(cn.dana_sense < 0) thetaa = -thetaa;
	if(cn.dmono_sense < 0) thetam = -thetam;
	

	t_mat Ti = tl::rotation_matrix_2d(ki_Q/rads);
	t_mat Tf = -tl::rotation_matrix_2d(kf_Q/rads);

	t_mat U = ublas::zero_matrix<t_real>(6,6);
	tl::submatrix_copy(U, Ti, 0, 0);
	tl::submatrix_copy(U, Tf, 0, 3);
	U(2,2) = 1.; U(2,5) = -1.;
	U(3,0) = 2.*cn.ki * angs * tl::KSQ2E;
	U(3,3) = -2.*cn.kf * angs * tl::KSQ2E;
	U(4,0) = 1.; U(5,2) = 1.;

	t_mat V(6,6);
	if(!tl::inverse(U, V))
	{
		res.bOk = false;
		res.strErr = "Transformation matrix cannot be inverted.";
		return res;
	}

	// -------------------------------------------------------------------------


	// -------------------------------------------------------------------------
	// resolution matrix

	t_vec pm(2);
	pm[0] = units::tan(thetam);
	pm[1] = 1.;
	pm /= cn.ki * angs * cn.mono_mosaic/rads;

	t_vec pa(2);
	pa[0] = -units::tan(thetaa);
	pa[1] = 1.;
	pa /= cn.kf * angs * cn.ana_mosaic/rads;

	t_vec palf0(2);
	palf0[0] = 2.*units::tan(thetam);
	palf0[1] = 1.;
	palf0 /= (cn.ki*angs * cn.coll_h_pre_mono/rads);

	t_vec palf1(2);
	palf1[0] = 0;
	palf1[1] = 1.;
	palf1 /= (cn.ki*angs * cn.coll_h_pre_sample/rads);

	t_vec palf2(2);
	palf2[0] = -2.*units::tan(thetaa);
	palf2[1] = 1.;
	palf2 /= (cn.kf*angs * cn.coll_h_post_ana/rads);

	t_vec palf3(2);
	palf3[0] = 0;
	palf3[1] = 1.;
	palf3 /= (cn.kf*angs * cn.coll_h_post_sample/rads);

	t_mat m01(2,2);
	m01 = ublas::outer_prod(pm,pm) +
			ublas::outer_prod(palf0,palf0) +
			ublas::outer_prod(palf1,palf1);
	t_mat m34(2,2);
	m34 = ublas::outer_prod(pa,pa) +
			ublas::outer_prod(palf2,palf2) +
			ublas::outer_prod(palf3,palf3);

	t_mat M = ublas::zero_matrix<t_real>(6,6);
	tl::submatrix_copy(M, m01, 0, 0);
	tl::submatrix_copy(M, m34, 3, 3);

	M(2,2) = 1./(cn.ki*cn.ki * angs*angs) *
		(
			1./(cn.coll_v_pre_sample*cn.coll_v_pre_sample/rads/rads) +
			1./((2.*units::sin(thetam)*cn.mono_mosaic/rads)*(2.*units::sin(thetam)*cn.mono_mosaic/rads) +
				cn.coll_v_pre_mono*cn.coll_v_pre_mono/rads/rads)
		);
	M(5,5) = 1./(cn.kf*cn.kf * angs*angs) *
		(
			1./(cn.coll_v_post_sample*cn.coll_v_post_sample/rads/rads) +
			1./((2.*units::sin(thetaa)*cn.ana_mosaic/rads)*(2.*units::sin(thetaa)*cn.ana_mosaic/rads) +
				cn.coll_v_post_ana*cn.coll_v_post_ana/rads/rads)
		);
	// -------------------------------------------------------------------------

	t_mat M1 = ublas::prod(M, V);
	t_mat N = ublas::prod(ublas::trans(V), M1);

	N = ellipsoid_gauss_int(N, 5);
	N = ellipsoid_gauss_int(N, 4);

	t_vec vec1 = tl::get_column<t_vec>(N, 1);
	t_mat NP = N - ublas::outer_prod(vec1,vec1)
										/(1./((cn.sample_mosaic/rads * cn.Q*angs)
										*(cn.sample_mosaic/rads * cn.Q*angs))
											+ N(1,1));
	NP(2,2) = N(2,2);
	NP *= tl::SIGMA2FWHM*tl::SIGMA2FWHM;

	res.reso = NP;
	
	// -------------------------------------------------------------------------


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
