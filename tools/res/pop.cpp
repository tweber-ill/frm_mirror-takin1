/*
 * popovici calculation
 * @author tweber
 * @date 08-may-2013
 * @copyright GPLv2
 *
 * @desc This is a reimplementation in C++ of the file rc_popma.m of the
 *		rescal5 package by Zinkin, McMorrow, Tennant, Farhi, and Wildes:
 *		http://www.ill.eu/en/instruments-support/computing-for-science/cs-software/all-software/matlab-ill/rescal-for-matlab/
 * @desc see: [pop75] M. Popovici, Acta Cryst. A 31, 507 (1975)
 */

#include "pop.h"
#include "tlibs/math/linalg.h"
#include "tlibs/math/math.h"

#include <string>
#include <iostream>
#include <boost/units/io.hpp>

typedef double t_real;
typedef ublas::matrix<t_real> t_mat;
typedef ublas::vector<t_real> t_vec;

using tl::angle; using tl::wavenumber; using tl::energy; using tl::length;
static const auto cm = tl::cm;
static const auto angs = tl::angstrom;
static const auto rads = tl::radians;
static const auto meV = tl::one_meV;


CNResults calc_pop(const PopParams& pop)
{
	CNResults res;

	res.Q_avg.resize(4);
	res.Q_avg[0] = pop.Q*angs;
	res.Q_avg[1] = 0.;
	res.Q_avg[2] = 0.;
	res.Q_avg[3] = pop.E / meV;


	length lam = tl::k2lam(pop.ki);
	angle twotheta = pop.twotheta;
	angle thetaa = pop.thetaa;
	angle thetam = pop.thetam;
	angle ki_Q = pop.angle_ki_Q;
	angle kf_Q = pop.angle_kf_Q;
	//kf_Q = twotheta + ki_Q;
	
	if(pop.dsample_sense < 0) 
	{ 
		twotheta = -twotheta; 
		ki_Q = -ki_Q; 
		kf_Q = -kf_Q; 
	}
	if(pop.dana_sense < 0) thetaa = -thetaa;
	if(pop.dmono_sense < 0) thetam = -thetam;

	angle coll_h_pre_mono = pop.coll_h_pre_mono;
	angle coll_v_pre_mono = pop.coll_v_pre_mono;
	
	if(pop.bGuide)
	{
		coll_h_pre_mono = lam*(pop.guide_div_h/angs);
		coll_v_pre_mono = lam*(pop.guide_div_v/angs);
	}

	// collimator covariance matrix G, [pop75], Appendix 1
	t_mat G = ublas::zero_matrix<t_real>(8,8);
	G(0,0) = 1./(coll_h_pre_mono*coll_h_pre_mono /rads/rads);
	G(1,1) = 1./(pop.coll_h_pre_sample*pop.coll_h_pre_sample /rads/rads);
	G(2,2) = 1./(coll_v_pre_mono*coll_v_pre_mono /rads/rads);
	G(3,3) = 1./(pop.coll_v_pre_sample*pop.coll_v_pre_sample /rads/rads);
	G(4,4) = 1./(pop.coll_h_post_sample*pop.coll_h_post_sample /rads/rads);
	G(5,5) = 1./(pop.coll_h_post_ana*pop.coll_h_post_ana /rads/rads);
	G(6,6) = 1./(pop.coll_v_post_sample*pop.coll_v_post_sample /rads/rads);
	G(7,7) = 1./(pop.coll_v_post_ana*pop.coll_v_post_ana /rads/rads);


	const units::quantity<units::si::plane_angle> mono_mosaic_spread = pop.mono_mosaic;
	const units::quantity<units::si::plane_angle> ana_mosaic_spread = pop.ana_mosaic;
	const units::quantity<units::si::plane_angle> sample_mosaic_spread = pop.sample_mosaic;

	// crystal mosaic covariance matrix F, [pop75], Appendix 1
	t_mat F = ublas::zero_matrix<t_real>(4,4);
	F(0,0) = 1./(pop.mono_mosaic*pop.mono_mosaic /rads/rads);
	F(1,1) = 1./(mono_mosaic_spread*mono_mosaic_spread /rads/rads);
	F(2,2) = 1./(pop.ana_mosaic*pop.ana_mosaic /rads/rads);
	F(3,3) = 1./(ana_mosaic_spread*ana_mosaic_spread /rads/rads);

	// C matrix, [pop75], Appendix 1
	t_mat C = ublas::zero_matrix<t_real>(4,8);
	C(2,5) = C(2,4) = C(0,1) = C(0,0) = 0.5;
	C(1,2) = 0.5/units::sin(thetam);
	/*C(1,3)*/C(2,2) = -0.5/units::sin(thetam);		// seems to be wrong in rescal5, Popovici says C(2,2), not C(1,3)
	C(3,6) = 0.5/units::sin(thetaa);
	C(3,7) = -0.5/units::sin(thetaa);

	// A matrix, [pop75], Appendix 1
	t_mat A = ublas::zero_matrix<t_real>(6,8);
	A(0,0) = 0.5 * pop.ki*angs * units::cos(thetam)/units::sin(thetam);
	A(0,1) = -0.5 * pop.ki*angs * units::cos(thetam)/units::sin(thetam);
	A(2,3) = A(1,1) = pop.ki * angs;
	A(3,4) = 0.5 * pop.kf*angs * units::cos(thetaa)/units::sin(thetaa);
	A(3,5) = -0.5 * pop.kf*angs * units::cos(thetaa)/units::sin(thetaa);
	A(5,6) = A(4,4) = pop.kf * angs;


	t_mat Ti = tl::rotation_matrix_2d(ki_Q/rads);
	t_mat Tf = -tl::rotation_matrix_2d(kf_Q/rads);

	// B matrix, [pop75], Appendix 1 -> U matrix in CN
	t_mat B = ublas::zero_matrix<t_real>(4,6);
	tl::submatrix_copy(B, Ti, 0, 0);
	tl::submatrix_copy(B, Tf, 0, 3);
	B(2,2) = 1.; B(2,5) = -1.;
	B(3,0) = 2.*pop.ki*angs * tl::KSQ2E;
	B(3,3) = -2.*pop.kf*angs * tl::KSQ2E;


	// S matrix, [pop75], Appendix 2
	// mono
	t_mat S1I = ublas::zero_matrix<t_real>(3,3);
	S1I(0,0) = 1./12. * pop.mono_thick*pop.mono_thick /cm/cm;
	S1I(1,1) = 1./12. * pop.mono_w*pop.mono_w /cm/cm;
	S1I(2,2) = 1./12. * pop.mono_h*pop.mono_h /cm/cm;

	// ana
	t_mat S3I = ublas::zero_matrix<t_real>(3,3);
	S3I(0,0) = 1./12. * pop.ana_thick*pop.ana_thick /cm/cm;
	S3I(1,1) = 1./12. * pop.ana_w*pop.ana_w /cm/cm;
	S3I(2,2) = 1./12. * pop.ana_h*pop.ana_h /cm/cm;


	t_real dMult = 1./12.;
	if(!pop.bSampleCub) dMult = 1./16.;

	// sample
	t_mat S2I = ublas::zero_matrix<t_real>(3,3);
	S2I(0,0) = dMult * pop.sample_w_perpq *pop.sample_w_perpq /cm/cm;
	S2I(1,1) = dMult * pop.sample_w_q*pop.sample_w_q /cm/cm;
	S2I(2,2) = 1./12. * pop.sample_h*pop.sample_h /cm/cm;


	dMult = 1./12.;
	if(!pop.bSrcRect) dMult = 1./16.;

	t_mat SI = ublas::zero_matrix<t_real>(13,13);
	SI(0,0) = dMult * pop.src_w*pop.src_w /cm/cm;
	SI(1,1) = dMult * pop.src_h*pop.src_h /cm/cm;
	tl::submatrix_copy(SI, S1I, 2, 2);
	tl::submatrix_copy(SI, S2I, 5, 5);
	tl::submatrix_copy(SI, S3I, 8, 8);


	dMult = 1./12.;
	if(!pop.bDetRect) dMult = 1./16.;

	SI(11,11) = dMult * pop.det_w*pop.det_w /cm/cm;
	SI(12,12) = dMult * pop.det_h*pop.det_h /cm/cm;

	SI *= tl::SIGMA2FWHM*tl::SIGMA2FWHM;

	t_mat S;
	if(!tl::inverse(SI, S))
	{
		res.bOk = false;
		res.strErr = "S matrix cannot be inverted.";
		return res;
	}


	t_real dCurvMonoH=0., dCurvMonoV=0., dCurvAnaH=0., dCurvAnaV=0.;
	if(pop.bMonoIsCurvedH) dCurvMonoH = 1./(pop.mono_curvh/cm) * pop.dmono_sense;
	if(pop.bMonoIsCurvedV) dCurvMonoV = 1./(pop.mono_curvv/cm) * pop.dmono_sense;
	if(pop.bAnaIsCurvedH) dCurvAnaH = 1./(pop.ana_curvh/cm) * pop.dana_sense;
	if(pop.bAnaIsCurvedV) dCurvAnaV = 1./(pop.ana_curvv/cm) * pop.dana_sense;


	// T matrix, [pop75], Appendix 2
	t_mat T = ublas::zero_matrix<t_real>(4,13);
	T(0,0) = -0.5 / (pop.dist_src_mono / cm);
	T(0,2) = 0.5 * units::cos(thetam) *
				(1./(pop.dist_mono_sample/cm) -
				 1./(pop.dist_src_mono/cm));
	T(0,3) = 0.5 * units::sin(thetam) *
				(1./(pop.dist_src_mono/cm) +
				 1./(pop.dist_mono_sample/cm) -
				 2.*dCurvMonoH/(units::sin(thetam)));
	T(0,5) = 0.5 * units::sin(0.5*twotheta) / (pop.dist_mono_sample/cm);
	T(0,6) = 0.5 * units::cos(0.5*twotheta) / (pop.dist_mono_sample/cm);
	T(1,1) = -0.5/(pop.dist_src_mono/cm * units::sin(thetam));
	T(1,4) = 0.5 * (1./(pop.dist_src_mono/cm) +
						1./(pop.dist_mono_sample/cm) -
						2.*units::sin(thetam)*dCurvMonoV)
					/ (units::sin(thetam));
	T(1,7) = -0.5/(pop.dist_mono_sample/cm * units::sin(thetam));
	T(2,5) = 0.5*units::sin(0.5*twotheta) / (pop.dist_sample_ana/cm);
	T(2,6) = -0.5*units::cos(0.5*twotheta) / (pop.dist_sample_ana/cm);
	T(2,8) = 0.5*units::cos(thetaa) * (1./(pop.dist_ana_det/cm) -
						1/(pop.dist_sample_ana/cm));
	T(2,9) = 0.5*units::sin(thetaa) * (
					1./(pop.dist_sample_ana/cm) +
					1./(pop.dist_ana_det/cm) -
					2.*dCurvAnaH / (units::sin(thetaa)));
	T(2,11) = 0.5/(pop.dist_ana_det/cm);
	T(3,7) = -0.5/(pop.dist_sample_ana/cm*units::sin(thetaa));
	T(3,10) = 0.5*(1./(pop.dist_sample_ana/cm) +
					1./(pop.dist_ana_det/cm) -
					2.*units::sin(thetaa)*dCurvAnaV)
					/ (units::sin(thetaa));
	T(3,12) = -0.5/(pop.dist_ana_det/cm*units::sin(thetaa));


	// D matrix, [pop75], Appendix 2
	t_mat D = ublas::zero_matrix<t_real>(8,13);
	D(0,0) = -1. / (pop.dist_src_mono/cm);
	D(0,2) = -cos(thetam) / (pop.dist_src_mono/cm);
	D(0,3) = sin(thetam) / (pop.dist_src_mono/cm);
	D(1,2) = cos(thetam) / (pop.dist_mono_sample/cm);
	D(1,3) = sin(thetam) / (pop.dist_mono_sample/cm);
	D(1,5) = sin(0.5*twotheta) / (pop.dist_mono_sample/cm);
	D(1,6) = cos(0.5*twotheta) / (pop.dist_mono_sample/cm);
	D(2,1) = -1. / (pop.dist_src_mono/cm);
	D(2,4) = 1. / (pop.dist_src_mono/cm);
	D(3,4) = -1. / (pop.dist_mono_sample/cm);
	D(3,7) = 1. / (pop.dist_mono_sample/cm);
	D(4,5) = sin(0.5*twotheta) / (pop.dist_sample_ana/cm);
	D(4,6) = -cos(0.5*twotheta) / (pop.dist_sample_ana/cm);
	D(4,8) = -cos(thetaa) / (pop.dist_sample_ana/cm);
	D(4,9) = sin(thetaa) / (pop.dist_sample_ana/cm);
	D(5,8) = cos(thetaa) / (pop.dist_ana_det/cm);
	D(5,9) = sin(thetaa) / (pop.dist_ana_det/cm);
	D(5,11) = 1. / (pop.dist_ana_det/cm);
	D(6,7) = -1. / (pop.dist_sample_ana/cm);
	D(6,10) = 1. / (pop.dist_sample_ana/cm);
	D(7,10) = -1. / (pop.dist_ana_det/cm);
	D(7,12) = 1. / (pop.dist_ana_det/cm);


	// [pop75], equ. 20
	t_mat FT = ublas::prod(F,T);
	t_mat M0 = S + ublas::prod(ublas::trans(T),FT);
	t_mat M0i;
	if(!tl::inverse(M0, M0i))
	{
		res.bOk = false;
		res.strErr = "Matrix M0 cannot be inverted.";
		return res;
	}

	t_mat M0iD = ublas::prod(M0i, ublas::trans(D));
	t_mat M1 = ublas::prod(D, M0iD);
	t_mat M1i;
	if(!tl::inverse(M1, M1i))
	{
		res.bOk = false;
		res.strErr = "Matrix M1 cannot be inverted.";
		return res;
	}

	t_mat M2 = M1i + G;
	t_mat M2i;
	if(!tl::inverse(M2, M2i))
	{
		res.bOk = false;
		res.strErr = "Matrix M2 cannot be inverted.";
		return res;
	}

	t_mat BA = ublas::prod(B,A);
	t_mat ABt = ublas::prod(ublas::trans(A), ublas::trans(B));
	t_mat M2iABt = ublas::prod(M2i, ABt);
	t_mat MI = ublas::prod(BA, M2iABt);

	MI(1,1) += pop.Q*pop.Q*angs*angs * pop.sample_mosaic*pop.sample_mosaic /rads/rads;
	MI(2,2) += pop.Q*pop.Q*angs*angs * sample_mosaic_spread*sample_mosaic_spread /rads/rads;

	t_mat M;
	if(!tl::inverse(MI, M))
	{
		res.bOk = false;
		res.strErr = "Covariance matrix cannot be inverted.";
		return res;
	}

	
	// -------------------------------------------------------------------------
	

	res.reso = M*tl::SIGMA2FWHM*tl::SIGMA2FWHM;

	res.dR0 = 0.;
	if(pop.bCalcR0)
	{
		// resolution volume, [pop75], equ. 13a & 16
		// [D] = 1/cm, [SI] = cm^2
		t_mat DSi = ublas::prod(D, SI);
		t_mat DSiDt = ublas::prod(DSi, ublas::trans(D));
		t_mat DSiDti;
		if(!tl::inverse(DSiDt, DSiDti))
		{
			res.bOk = false;
			res.strErr = "Resolution volume cannot be calculated.";
			return res;
		}
		DSiDti += G;
		t_real dDetDSiDti = tl::determinant(DSiDti);
		t_real dP0 = pop.dmono_refl*pop.dana_effic * (2.*M_PI)*(2.*M_PI)*(2.*M_PI)*(2.*M_PI);
		dP0 /= std::sqrt(dDetDSiDti);

		// [T] = 1/cm, [F] = 1/rad^2
		t_mat TtF = ublas::prod(ublas::trans(T), F);
		t_mat TtFT = ublas::prod(TtF, T);
		t_mat K = S + TtFT;

		t_real dDetS = tl::determinant(S);
		t_real dDetF = tl::determinant(F);
		t_real dDetK = tl::determinant(K);

		res.dR0 = dP0 / (64. * M_PI*M_PI * units::sin(thetam)*units::sin(thetaa));
		res.dR0 *= std::sqrt(dDetS*dDetF/dDetK);

		// rest of the prefactors, equ. 1 in [pop75]
		res.dR0 /= (2.*M_PI*2.*M_PI);
		res.dR0 *= std::sqrt(tl::determinant(res.reso));
	}

	// placeholder: volume of ellipsoid
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
