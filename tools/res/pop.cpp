/*
 * popovici calculation
 * @author tweber
 * @date 08-may-2013
 *
 * @desc This is a reimplementation in C++ of the file rc_popma.m of the
 *    			rescal5 package by Zinkin, McMorrow, Tennant, Farhi, and Wildes:
 *    			http://www.ill.eu/en/instruments-support/computing-for-science/cs-software/all-software/matlab-ill/rescal-for-matlab/
 */

#include "pop.h"
#include "../../helper/linalg.h"
#include "../../helper/math.h"
#include "../../helper/neutrons.hpp"

#include <string>
#include <iostream>
#include <boost/units/io.hpp>

typedef units::quantity<units::si::plane_angle> angle;
typedef units::quantity<units::si::wavenumber> wavenumber;
typedef units::quantity<units::si::energy> energy;
typedef units::quantity<units::si::length> length;

static const units::quantity<units::si::length> cm = 0.01 * units::si::meter;


CNResults calc_pop(PopParams& pop)
{
	const units::quantity<units::si::plane_angle> mono_mosaic_spread = pop.mono_mosaic;
	const units::quantity<units::si::plane_angle> ana_mosaic_spread = pop.ana_mosaic;
	const units::quantity<units::si::plane_angle> sample_mosaic_spread = pop.sample_mosaic;

	CNResults res;
	if(!calc_cn_angles(pop, res))
		return res;

	length lam = 2.*M_PI / pop.ki;
	angle phi = units::atan2(-pop.kf * units::sin(2.*res.thetas), pop.ki-pop.kf*units::cos(2.*res.thetas));

	if(pop.bGuide)
	{
		pop.coll_h_pre_mono = lam*(pop.guide_div_h/angstrom);
		pop.coll_v_pre_mono = lam*(pop.guide_div_v/angstrom);
	}

	ublas::matrix<double> G = ublas::zero_matrix<double>(8,8);
	G(0,0) = 1./(pop.coll_h_pre_mono*pop.coll_h_pre_mono /units::si::radians/units::si::radians);
	G(1,1) = 1./(pop.coll_h_pre_sample*pop.coll_h_pre_sample /units::si::radians/units::si::radians);
	G(2,2) = 1./(pop.coll_v_pre_mono*pop.coll_v_pre_mono /units::si::radians/units::si::radians);
	G(3,3) = 1./(pop.coll_v_pre_sample*pop.coll_v_pre_sample /units::si::radians/units::si::radians);
	G(4,4) = 1./(pop.coll_h_post_sample*pop.coll_h_post_sample /units::si::radians/units::si::radians);
	G(5,5) = 1./(pop.coll_h_post_ana*pop.coll_h_post_ana /units::si::radians/units::si::radians);
	G(6,6) = 1./(pop.coll_v_post_sample*pop.coll_v_post_sample /units::si::radians/units::si::radians);
	G(7,7) = 1./(pop.coll_v_post_ana*pop.coll_v_post_ana /units::si::radians/units::si::radians);


	ublas::matrix<double> F = ublas::zero_matrix<double>(4,4);
	F(0,0) = 1./(pop.mono_mosaic*pop.mono_mosaic /units::si::radians/units::si::radians);
	F(1,1) = 1./(mono_mosaic_spread*mono_mosaic_spread /units::si::radians/units::si::radians);
	F(2,2) = 1./(pop.ana_mosaic*pop.ana_mosaic /units::si::radians/units::si::radians);
	F(3,3) = 1./(ana_mosaic_spread*ana_mosaic_spread /units::si::radians/units::si::radians);

	ublas::matrix<double> C = ublas::zero_matrix<double>(4,8);
	C(2,5) = C(2,4) = C(0,1) = C(0,0) = 0.5;
	C(1,2) = 0.5/units::sin(res.thetam);
	C(1,3) = -0.5/units::sin(res.thetam);
	C(3,6) = 0.5/units::sin(res.thetaa);
	C(3,7) = -0.5/units::sin(res.thetaa);

	ublas::matrix<double> A = ublas::zero_matrix<double>(6,8);
	A(0,0) = 0.5 * pop.ki*angstrom * units::cos(res.thetam)/units::sin(res.thetam);
	A(0,1) = -0.5 * pop.ki*angstrom * units::cos(res.thetam)/units::sin(res.thetam);
	A(2,3) = A(1,1) = pop.ki * angstrom;
	A(3,4) = 0.5 * pop.kf*angstrom * units::cos(res.thetaa)/units::sin(res.thetaa);
	A(3,5) = -0.5 * pop.kf*angstrom * units::cos(res.thetaa)/units::sin(res.thetaa);
	A(5,6) = A(4,4) = pop.kf * angstrom;

	ublas::matrix<double> B = ublas::zero_matrix<double>(4,6);
	B(0,0) = units::cos(phi);
	B(0,1) = units::sin(phi);
	B(0,3) = -units::cos(phi - 2.*res.thetas);
	B(0,4) = -units::sin(phi - 2.*res.thetas);
	B(1,0) = -units::sin(phi);
	B(1,1) = units::cos(phi);
	B(1,3) = units::sin(phi - 2.*res.thetas);
	B(1,4) = -units::cos(phi - 2.*res.thetas);
	B(2,2) = 1.;
	B(2,5) = -1.;
	B(3,0) = 2.*pop.ki*angstrom * KSQ2E;
	B(3,3) = -2.*pop.kf*angstrom * KSQ2E;



	ublas::matrix<double> S1I = ublas::zero_matrix<double>(3,3);
	S1I(0,0) = 1./12. * pop.mono_thick*pop.mono_thick /cm/cm;
	S1I(1,1) = 1./12. * pop.mono_w*pop.mono_w /cm/cm;
	S1I(2,2) = 1./12. * pop.mono_h*pop.mono_h /cm/cm;

	ublas::matrix<double> S3I = ublas::zero_matrix<double>(3,3);
	S3I(0,0) = 1./12. * pop.ana_thick*pop.ana_thick /cm/cm;
	S3I(1,1) = 1./12. * pop.ana_w*pop.ana_w /cm/cm;
	S3I(2,2) = 1./12. * pop.ana_h*pop.ana_h /cm/cm;


	double dMult = 1./12.;
	if(!pop.bSampleCub) dMult = 1./16.;

	ublas::matrix<double> S2I = ublas::zero_matrix<double>(3,3);
	S2I(0,0) = dMult * pop.sample_w_perpq *pop.sample_w_perpq /cm/cm;
	S2I(1,1) = dMult * pop.sample_w_q*pop.sample_w_q /cm/cm;
	S2I(2,2) = 1./12. * pop.sample_h*pop.sample_h /cm/cm;


	dMult = 1./12.;
	if(!pop.bSrcRect) dMult = 1./16.;

	ublas::matrix<double> SI = ublas::zero_matrix<double>(13,13);
	SI(0,0) = dMult * pop.src_w*pop.src_w /cm/cm;
	SI(1,1) = dMult * pop.src_h*pop.src_h /cm/cm;
	submatrix_copy(SI, S1I, 2, 2);
	submatrix_copy(SI, S2I, 5, 5);
	submatrix_copy(SI, S3I, 8, 8);


	dMult = 1./12.;
	if(!pop.bDetRect) dMult = 1./16.;

	SI(11,11) = dMult * pop.det_w*pop.det_w /cm/cm;
	SI(12,12) = dMult * pop.det_h*pop.det_h /cm/cm;

	SI *= SIGMA2FWHM*SIGMA2FWHM;

	ublas::matrix<double> S;
	if(!::inverse(SI, S))
	{
		res.bOk = false;
		res.strErr = "Matrix cannot be inverted.";
		return res;
	}


	double dCurvMonoH=0., dCurvMonoV=0., dCurvAnaH=0., dCurvAnaV=0.;
	if(pop.bMonoIsCurvedH) dCurvMonoH = 1./(pop.mono_curvh/cm) * pop.dmono_sense;
	if(pop.bMonoIsCurvedV) dCurvMonoV = 1./(pop.mono_curvv/cm) * pop.dmono_sense;
	if(pop.bAnaIsCurvedH) dCurvAnaH = 1./(pop.ana_curvh/cm) * pop.dana_sense;
	if(pop.bAnaIsCurvedV) dCurvAnaV = 1./(pop.ana_curvv/cm) * pop.dana_sense;

	ublas::matrix<double> T = ublas::zero_matrix<double>(4,13);
	T(0,0) = -0.5 / (pop.dist_src_mono / cm);
	T(0,2) = 0.5 * units::cos(res.thetam) *
				(1./(pop.dist_mono_sample/cm) -
				 1./(pop.dist_src_mono/cm));
	T(0,3) = 0.5 * units::sin(res.thetam) *
				(1./(pop.dist_src_mono/cm) +
				 1./(pop.dist_mono_sample/cm) -
				 2.*dCurvMonoH/(units::sin(res.thetam)));
	T(0,5) = 0.5 * units::sin(res.thetas) / (pop.dist_mono_sample/cm);
	T(0,6) = 0.5 * units::cos(res.thetas)/(pop.dist_mono_sample/cm);
	T(1,1) = -0.5/(pop.dist_src_mono/cm * units::sin(res.thetam));
	T(1,4) = 0.5 * (1./(pop.dist_src_mono/cm) +
						1./(pop.dist_mono_sample/cm) -
						2.*units::sin(res.thetam)*dCurvMonoV)
					/ (units::sin(res.thetam));
	T(1,7) = -0.5/(pop.dist_mono_sample/cm * units::sin(res.thetam));
	T(2,5) = 0.5*units::sin(res.thetas) / (pop.dist_sample_ana/cm);
	T(2,6) = -0.5*units::cos(res.thetas) / (pop.dist_sample_ana/cm);
	T(2,8) = 0.5*units::cos(res.thetaa) * (1./(pop.dist_ana_det/cm) -
													1/(pop.dist_sample_ana/cm));
	T(2,9) = 0.5*units::sin(res.thetaa) * (
					1./(pop.dist_sample_ana/cm) +
					1./(pop.dist_ana_det/cm) -
					2.*dCurvAnaH / (units::sin(res.thetaa)));
	T(2,11) = 0.5/(pop.dist_ana_det/cm);
	T(3,7) = -0.5/(pop.dist_sample_ana/cm*units::sin(res.thetaa));
	T(3,10) = 0.5*(1./(pop.dist_sample_ana/cm) +
					1./(pop.dist_ana_det/cm) -
					2.*units::sin(res.thetaa)*dCurvAnaV)
					/ (units::sin(res.thetaa));
	T(3,12) = -0.5/(pop.dist_ana_det/cm*units::sin(res.thetaa));


	ublas::matrix<double> D = ublas::zero_matrix<double>(8,13);
	D(0,0) = -1. / (pop.dist_src_mono/cm);
	D(0,2) = -cos(res.thetam) / (pop.dist_src_mono/cm);
	D(0,3) = sin(res.thetam) / (pop.dist_src_mono/cm);
	D(1,2) = cos(res.thetam) / (pop.dist_mono_sample/cm);
	D(1,3) = sin(res.thetam) / (pop.dist_mono_sample/cm);
	D(1,5) = sin(res.thetas) / (pop.dist_mono_sample/cm);
	D(1,6) = cos(res.thetas) / (pop.dist_mono_sample/cm);
	D(2,1) = -1. / (pop.dist_src_mono/cm);
	D(2,4) = 1. / (pop.dist_src_mono/cm);
	D(3,4) = -1. / (pop.dist_mono_sample/cm);
	D(3,7) = 1. / (pop.dist_mono_sample/cm);
	D(4,5) = sin(res.thetas) / (pop.dist_sample_ana/cm);
	D(4,6) = -cos(res.thetas) / (pop.dist_sample_ana/cm);
	D(4,8) = -cos(res.thetaa) / (pop.dist_sample_ana/cm);
	D(4,9) = sin(res.thetaa) / (pop.dist_sample_ana/cm);
	D(5,8) = cos(res.thetaa) / (pop.dist_ana_det/cm);
	D(5,9) = sin(res.thetaa) / (pop.dist_ana_det/cm);
	D(5,11) = 1. / (pop.dist_ana_det/cm);
	D(6,7) = -1. / (pop.dist_sample_ana/cm);
	D(6,10) = 1. / (pop.dist_sample_ana/cm);
	D(7,10) = -1. / (pop.dist_ana_det/cm);
	D(7,12) = 1. / (pop.dist_ana_det/cm);


	ublas::matrix<double> FT = ublas::prod(F,T);
	ublas::matrix<double> M0 = S + ublas::prod(ublas::trans(T),FT);
	ublas::matrix<double> M0i;
	if(!::inverse(M0, M0i))
	{
		res.bOk = false;
		res.strErr = "Matrix M0 cannot be inverted.";
		return res;
	}

	ublas::matrix<double> M0iD = ublas::prod(M0i, ublas::trans(D));
	ublas::matrix<double> M1 =ublas::prod(D, M0iD);
	ublas::matrix<double> M1i;
	if(!::inverse(M1, M1i))
	{
		res.bOk = false;
		res.strErr = "Matrix M1 cannot be inverted.";
		return res;
	}

	ublas::matrix<double> M2 = M1i + G;
	ublas::matrix<double> M2i;
	if(!::inverse(M2, M2i))
	{
		res.bOk = false;
		res.strErr = "Matrix M2 cannot be inverted.";
		return res;
	}

	ublas::matrix<double> BA = ublas::prod(B,A);
	ublas::matrix<double> ABt = ublas::prod(ublas::trans(A), ublas::trans(B));
	ublas::matrix<double> M2iABt = ublas::prod(M2i, ABt);
	ublas::matrix<double> MI = ublas::prod(BA, M2iABt);

	MI(1,1) += pop.Q*pop.Q*angstrom*angstrom * pop.sample_mosaic*pop.sample_mosaic /units::si::radians/units::si::radians;
	MI(2,2) += pop.Q*pop.Q*angstrom*angstrom * sample_mosaic_spread*sample_mosaic_spread /units::si::radians/units::si::radians;

	ublas::matrix<double> M;
	if(!::inverse(MI, M))
	{
		res.bOk = false;
		res.strErr = "Covariance matrix cannot be inverted.";
		return res;
	}
	res.reso = M*SIGMA2FWHM*SIGMA2FWHM;

	calc_bragg_widths(pop, res);
	calc_cn_vol(pop, res);

	if(std::isnan(res.dR0) || isnan(res.reso))
	{
		res.strErr = "Invalid result.";
		res.bOk = false;
		return res;
	}

	res.bOk = 1;
	return res;
}

