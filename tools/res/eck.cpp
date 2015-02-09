/*
 * eckold calculation
 * @author tweber
 * @date feb-2015
 * @copyright GPLv2
 *
 * @desc see: [eck14] G. Eckold and O. Sobolev, NIM A 752, pp. 54-64 (2014)
 */

#include "eck.h"

#include "tlibs/math/linalg.h"
#include "tlibs/math/math.h"
#include "ellipse.h"

#include <string>
#include <iostream>
#include <boost/units/io.hpp>

typedef ublas::matrix<double> t_mat;
typedef ublas::vector<double> t_vec;

using tl::angle; using tl::wavenumber; using tl::energy; using tl::length;
static const auto cm = tl::cm;
static const auto secs = tl::seconds;
static const auto angs = tl::angstrom;
static const auto rads = tl::radians;
static const auto meV = tl::one_meV;

CNResults calc_eck(EckParams& eck)
{
	eck.mono_mosaic_v = 9999./(180.*60.) * rads;
	eck.ana_mosaic_v = 9999./(180.*60.) * rads;
	eck.pos_x = 0.*cm;
	eck.pos_y = 0.*cm;
	eck.pos_z = 0.*cm;

	if(!eck.bMonoIsCurvedH) eck.mono_curvh = 99999. * cm;
	if(!eck.bMonoIsCurvedV) eck.mono_curvv = 99999. * cm;
	if(!eck.bAnaIsCurvedH) eck.ana_curvh = 99999. * cm;
	if(!eck.bAnaIsCurvedV) eck.ana_curvv = 99999. * cm;

	angle twotheta = eck.twotheta;
	angle thetaa = eck.thetaa;
	angle thetam = eck.thetam;
	angle ki_Q = eck.angle_ki_Q;
	angle kf_Q = eck.angle_kf_Q;

	if(eck.dsample_sense < 0) { twotheta = -twotheta; /*ki_Q = -ki_Q; kf_Q = -kf_Q;*/ }
	if(eck.dana_sense < 0) thetaa = -thetaa;
	if(eck.dmono_sense < 0) thetam = -thetam;


	//std::cout << "2theta = " << double(twotheta/rads/M_PI*180.) << " deg"<< std::endl;
	//std::cout << "kiQ = " << double(ki_Q/rads/M_PI*180.) << " deg"<< std::endl;



	CNResults res;

	res.Q_avg.resize(4);
	res.Q_avg[0] = eck.Q*angs;
	res.Q_avg[1] = 0.;
	res.Q_avg[2] = 0.;
	res.Q_avg[3] = eck.E/meV;


	//--------------------------------------------------------------------------
	// mono part
	
	// A matrix: formula 26 in [eck14]
	t_mat A = ublas::identity_matrix<double>(3);
	A(0,0) = 4.*std::log(2.)/(eck.ki*angs*eck.ki*angs) * units::tan(thetam)*units::tan(thetam) *
		( (2./(eck.coll_h_pre_mono/rads))*(2./(eck.coll_h_pre_mono/rads)) +
		  (2.*eck.dist_src_mono/eck.src_w)*(2.*eck.dist_src_mono/eck.src_w) +
		  (1./(eck.mono_mosaic/rads))*(1./(eck.mono_mosaic/rads))
		);
	A(0,1) = A(1,0) = 4.*std::log(2.)/(eck.ki*angs*eck.ki*angs) * units::tan(thetam) *
		( 2.*(1./(eck.coll_h_pre_mono/rads))*(1./(eck.coll_h_pre_mono/rads)) +
		  2.*eck.dist_src_mono*(eck.dist_src_mono-eck.dist_mono_sample)/(eck.src_w*eck.src_w) +
		  (1./(eck.mono_mosaic/rads))*(1./(eck.mono_mosaic/rads)) *
			(1. - eck.dist_mono_sample/(eck.mono_curvh*std::fabs(units::sin(thetam))))
		);
	A(1,1) = 4.*std::log(2.)/(eck.ki*angs*eck.ki*angs) *
		( (1./(eck.coll_h_pre_mono/rads))*(1./(eck.coll_h_pre_mono/rads)) +
		  (1./(eck.coll_h_pre_sample/rads))*(1./(eck.coll_h_pre_sample/rads)) +
		  ((eck.dist_src_mono-eck.dist_mono_sample)/eck.src_w)*((eck.dist_src_mono-eck.dist_mono_sample)/eck.src_w) +
		  (eck.dist_mono_sample/(eck.mono_w*std::fabs(units::sin(thetam))))*(eck.dist_mono_sample/(eck.mono_w*std::fabs(units::sin(thetam)))) +
		  (1./(eck.mono_mosaic/rads))*(1./(eck.mono_mosaic/rads)) *
			(1. - eck.dist_mono_sample/(eck.mono_curvh*std::fabs(units::sin(thetam)))) *
			(1. - eck.dist_mono_sample/(eck.mono_curvh*std::fabs(units::sin(thetam))))
		);

	// Av matrix: formula 38 in [eck14]
	t_mat Av(2,2);
	Av(0,0) = 4.*std::log(2.)/(eck.ki*angs*eck.ki*angs) *
		( (1./(eck.coll_v_pre_sample/rads))*(1./(eck.coll_v_pre_sample/rads)) +
		  (eck.dist_mono_sample/eck.src_h)*(eck.dist_mono_sample/eck.src_h) +
		  (eck.dist_mono_sample/eck.mono_h)*(eck.dist_mono_sample/eck.mono_h) +
		  (1./(2.*eck.mono_mosaic_v/rads*units::sin(thetam)))*(1./(2.*eck.mono_mosaic_v/rads*units::sin(thetam))) +
		  eck.dist_mono_sample/(eck.mono_mosaic_v/rads*eck.mono_mosaic_v/rads*eck.mono_curvv*std::fabs(units::sin(thetam)))
		);
	Av(0,1) = Av(1,0) = 4.*std::log(2.)/(eck.ki*angs*eck.ki*angs) *
		( eck.dist_src_mono*eck.dist_mono_sample/(eck.src_h*eck.src_h) -
		  (1./(2.*eck.mono_mosaic_v/rads*units::sin(thetam)))*(1./(2.*eck.mono_mosaic_v/rads*units::sin(thetam))) +
		  eck.dist_mono_sample/(2.*eck.mono_mosaic_v/rads*eck.mono_mosaic_v/rads*eck.mono_curvv*std::fabs(units::sin(thetam)))
		);
	Av(1,1) = 4.*std::log(2.)/(eck.ki*angs*eck.ki*angs) *
		( (1./(eck.coll_v_pre_mono/rads))*(1./(eck.coll_v_pre_mono/rads)) +
		  (eck.dist_src_mono/eck.src_h)*(eck.dist_src_mono/eck.src_h) +
		  (1./(2.*eck.mono_mosaic_v/rads*units::sin(thetam)))*(1./(2.*eck.mono_mosaic_v/rads*units::sin(thetam)))
		);


	// B vector: formula 27 in [eck14]
	t_vec B(3);
	B(0) = 8.*std::log(2.)*eck.pos_y/(eck.ki*angs) * units::tan(thetam) *
		( 2.*eck.dist_src_mono/(eck.src_w*eck.src_w) +
		  1./((eck.mono_mosaic/rads)*(eck.mono_mosaic/rads) * eck.mono_curvh*std::fabs(units::sin(thetam)))
		);
	B(1) = 8.*std::log(2.)*eck.pos_y/(eck.ki*angs) *
		( -eck.dist_mono_sample/(eck.mono_w*std::fabs(units::sin(thetam)) * eck.mono_w*std::fabs(units::sin(thetam))) +
		  (1./((eck.mono_mosaic/rads)*(eck.mono_mosaic/rads) * eck.mono_curvh*std::fabs(units::sin(thetam)))) *
			(1. - eck.dist_mono_sample/(eck.mono_curvh*std::fabs(units::sin(thetam)))) +
		  (eck.dist_src_mono-eck.dist_mono_sample)/(eck.src_w*eck.src_w)
		);

	// Bv vector: formula 39 in [eck14]
	t_vec Bv(2);
	Bv(0) = 4.*std::log(2.)*eck.pos_z / (eck.ki*angs) *
		( -2.*eck.dist_mono_sample / (eck.src_h*eck.src_h) +		// typo in paper?
		  1./(eck.mono_mosaic_v/rads*eck.mono_mosaic_v/rads*eck.mono_curvv*std::fabs(units::sin(thetam))) -
		  2.*eck.dist_mono_sample/(eck.mono_mosaic_v/rads*eck.mono_mosaic_v/rads*eck.mono_curvv*eck.mono_curvv) -
		  2.*eck.dist_mono_sample / (eck.mono_h*eck.mono_h)			// typo in paper?
		);
	Bv(1) = 4.*std::log(2.)*eck.pos_z / (eck.ki*angs) *
		( -2.*eck.dist_mono_sample / (eck.src_h*eck.src_h) -		// typo in paper?
		  1./(eck.mono_mosaic_v/rads*eck.mono_mosaic_v/rads*eck.mono_curvv*std::fabs(units::sin(thetam)))
		);


	// C scalar: formula 28 in [eck14]
	double C = 4.*std::log(2.)*eck.pos_y*eck.pos_y *
		( 1./(eck.src_w*eck.src_w) +
		  1./(eck.mono_w*std::fabs(units::sin(thetam)) * eck.mono_w*std::fabs(units::sin(thetam))) +
		  1./(eck.mono_mosaic/rads * eck.mono_curvh*std::fabs(units::sin(thetam)) * eck.mono_mosaic/rads * eck.mono_curvh*std::fabs(units::sin(thetam)))
		);

	// Cv scalar: formula 40 in [eck14]
	double Cv = 4.*std::log(2.)*eck.pos_z*eck.pos_z *
		( 1./(eck.src_h*eck.src_h) +
		  1./(eck.mono_h*eck.mono_h) +
		  1./((eck.mono_mosaic_v/rads * eck.mono_curvv) * (eck.mono_mosaic_v/rads * eck.mono_curvv))
		);


	// z components, [eck14], equ. 42
	A(2,2) = Av(0,0) - Av(0,1)*Av(0,1)/Av(1,1);
	B[2] = Bv[0] - Bv[1]*Av(0,1)/Av(1,1);
	double D = Cv - 0.25*Bv[1]/Av(1,1);
	
	
	/*std::cout << "A = " << A << std::endl;
	std::cout << "Av = " << Av << std::endl;
	std::cout << "B = " << B << std::endl;
	std::cout << "Bv = " << Bv << std::endl;
	std::cout << "C = " << C << std::endl;
	std::cout << "Cv = " << Cv << std::endl;
	std::cout << "D = " << D << std::endl;*/
	//--------------------------------------------------------------------------



	//--------------------------------------------------------------------------
	// ana part

	// E matrix: formula 45 in [eck14]
	t_mat E = ublas::identity_matrix<double>(3);
	E(0,0) = 4.*std::log(2.)/(eck.kf*angs*eck.kf*angs) * units::tan(thetaa)*units::tan(thetaa) *
		( (2./(eck.coll_h_post_ana/rads))*(2./(eck.coll_h_post_ana/rads)) +
		  (2.*eck.dist_ana_det/eck.det_w)*(2.*eck.dist_ana_det/eck.det_w) +
		  (1./(eck.ana_mosaic/rads))*(1./(eck.ana_mosaic/rads))
		);
	E(0,1) = E(1,0) = 4.*std::log(2.)/(eck.kf*angs*eck.kf*angs) * units::tan(-thetaa) *
		( 2.*(1./(eck.coll_h_post_ana/rads))*(1./(eck.coll_h_post_ana/rads)) +
		  2.*eck.dist_ana_det*(eck.dist_ana_det-eck.dist_sample_ana)/(eck.det_w*eck.det_w) +
		  (1./(eck.ana_mosaic/rads))*(1./(eck.ana_mosaic/rads)) *
			(1. - eck.dist_sample_ana/(eck.ana_curvh*std::fabs(units::sin(-thetaa))))
		);
	E(1,1) = 4.*std::log(2.)/(eck.kf*angs*eck.kf*angs) *
		( (1./(eck.coll_h_post_ana/rads))*(1./(eck.coll_h_post_ana/rads)) +
		  (1./(eck.coll_h_post_sample/rads))*(1./(eck.coll_h_post_sample/rads)) +
		  ((eck.dist_ana_det-eck.dist_sample_ana)/eck.det_w)*((eck.dist_ana_det-eck.dist_sample_ana)/eck.det_w) +
		  (eck.dist_sample_ana/(eck.ana_w*std::fabs(units::sin(-thetaa))))*(eck.dist_sample_ana/(eck.ana_w*std::fabs(units::sin(-thetaa)))) +
		  (1./(eck.ana_mosaic/rads))*(1./(eck.ana_mosaic/rads)) *
			(1. - eck.dist_sample_ana/(eck.ana_curvh*std::fabs(units::sin(-thetaa)))) *
			(1. - eck.dist_sample_ana/(eck.ana_curvh*std::fabs(units::sin(-thetaa))))
		);

	// Ev matrix
	t_mat Ev(2,2);
	Ev(0,0) = 4.*std::log(2.)/(eck.kf*angs*eck.kf*angs) *
		( (1./(eck.coll_v_post_sample/rads))*(1./(eck.coll_v_post_sample/rads)) +
		  (eck.dist_sample_ana/eck.det_h)*(eck.dist_sample_ana/eck.det_h) +
		  (eck.dist_sample_ana/eck.ana_h)*(eck.dist_sample_ana/eck.ana_h) +
		  (1./(2.*eck.ana_mosaic_v/rads*units::sin(-thetaa)))*(1./(2.*eck.ana_mosaic_v/rads*units::sin(-thetaa))) +
		  eck.dist_sample_ana/(eck.ana_mosaic_v/rads*eck.ana_mosaic_v/rads*eck.ana_curvv*std::fabs(units::sin(-thetaa)))
		);
	Ev(0,1) = Ev(1,0) = 4.*std::log(2.)/(eck.kf*angs*eck.kf*angs) *
		( eck.dist_ana_det*eck.dist_sample_ana/(eck.det_h*eck.det_h) -
		  (1./(2.*eck.ana_mosaic_v/rads*units::sin(-thetaa)))*(1./(2.*eck.ana_mosaic_v/rads*units::sin(-thetaa))) +
		  eck.dist_sample_ana/(2.*eck.ana_mosaic_v/rads*eck.ana_mosaic_v/rads*eck.ana_curvv*std::fabs(units::sin(-thetaa)))
		);
	Ev(1,1) = 4.*std::log(2.)/(eck.kf*angs*eck.kf*angs) *
		( (1./(eck.coll_v_post_ana/rads))*(1./(eck.coll_v_post_ana/rads)) +
		  (eck.dist_ana_det/eck.det_h)*(eck.dist_ana_det/eck.det_h) +
		  (1./(2.*eck.ana_mosaic_v/rads*units::sin(-thetaa)))*(1./(2.*eck.ana_mosaic_v/rads*units::sin(-thetaa)))
		);

	// equ 43 in [eck14]
	length pos_y2 = -eck.pos_x*units::sin(twotheta)
					+eck.pos_y*units::sin(twotheta);

	// F vector: formula 46 in [eck14]		-> signs in paper correct?
	t_vec F(3);
	F(0) = 8.*std::log(2.)*pos_y2/(eck.kf*angs) * units::tan(-thetaa) *
		( 2.*eck.dist_ana_det/(eck.det_w*eck.det_w) +
		  1./((eck.ana_mosaic/rads)*(eck.ana_mosaic/rads) * eck.ana_curvh*std::fabs(units::sin(-thetaa)))
		);
	F(1) = 8.*std::log(2.)*pos_y2/(eck.kf*angs) *
		( -eck.dist_sample_ana/(eck.ana_w*std::fabs(units::sin(-thetaa)) * eck.ana_w*std::fabs(units::sin(-thetaa))) +
		  (1./((eck.ana_mosaic/rads)*(eck.ana_mosaic/rads) * eck.ana_curvh*std::fabs(units::sin(-thetaa)))) *
			(1. - eck.dist_sample_ana/(eck.ana_curvh*std::fabs(units::sin(-thetaa)))) +
		  (eck.dist_ana_det-eck.dist_sample_ana)/(eck.det_w*eck.det_w)
		);

	// Fv vector
	t_vec Fv(2);
	Fv(0) = 4.*std::log(2.)*eck.pos_z / (eck.kf*angs) *
		( -2.*eck.dist_sample_ana / (eck.det_h*eck.det_h) +			// typo in paper?
		  1./(eck.ana_mosaic_v/rads*eck.ana_mosaic_v/rads*eck.ana_curvv*std::fabs(units::sin(-thetaa))) -
		  2.*eck.dist_sample_ana/(eck.ana_mosaic_v/rads*eck.ana_mosaic_v/rads*eck.ana_curvv*eck.ana_curvv) -
		  2.*eck.dist_sample_ana / (eck.ana_h*eck.ana_h)			// typo in paper?
		);
	Fv(1) = 4.*std::log(2.)*eck.pos_z / (eck.kf*angs) *
		( -2.*eck.dist_sample_ana / (eck.det_h*eck.det_h) -			// typo in paper?
		  1./(eck.ana_mosaic_v/rads*eck.ana_mosaic_v/rads*eck.ana_curvv*std::fabs(units::sin(-thetaa)))
		);


	// G scalar: formula 47 in [eck14]
	double G = 4.*std::log(2.)*pos_y2*pos_y2 *
		( 1./(eck.det_w*eck.det_w) +
		  1./(eck.ana_w*std::fabs(units::sin(-thetaa)) * eck.ana_w*std::fabs(units::sin(-thetaa))) +
		  1./(eck.ana_mosaic/rads * eck.ana_curvh*std::fabs(units::sin(-thetaa)) * eck.ana_mosaic/rads * eck.ana_curvh*std::fabs(units::sin(-thetaa)))
		);

	// Gv scalar
	double Gv = 4.*std::log(2.)*eck.pos_z*eck.pos_z *
		( 1./(eck.det_h*eck.det_h) +
		  1./(eck.ana_h*eck.ana_h) +
		  1./((eck.ana_mosaic_v/rads * eck.ana_curvv) * (eck.ana_mosaic_v/rads * eck.ana_curvv))
		);

	// z components
	E(2,2) = Ev(0,0) - Ev(0,1)*Ev(0,1)/Ev(1,1);
	F[2] = Fv[0] - Fv[1]*Ev(0,1)/Ev(1,1);
	double H = Gv - 0.25*Fv[1]/Ev(1,1);
	
	
	/*std::cout << "E = " << E << std::endl;
	std::cout << "Ev = " << Ev << std::endl;
	std::cout << "F = " << F << std::endl;
	std::cout << "Fv = " << Fv << std::endl;
	std::cout << "G = " << G << std::endl;
	std::cout << "Gv = " << Gv << std::endl;
	std::cout << "H = " << H << std::endl;*/
	//--------------------------------------------------------------------------


	// equ 4 & equ 53 in [eck14]
	double s0 = (eck.ki*eck.ki - eck.kf*eck.kf) / (2. * eck.Q*eck.Q);
	wavenumber kperp = units::sqrt(units::abs(eck.Q*eck.Q*(0.5 + s0)*(0.5 + s0) -
												eck.ki*eck.ki));
	if(twotheta/rads >= 0.)
		kperp = -kperp;
									
	//std::cout << "s0 = " << s0 << std::endl;
	//std::cout << "kperp = " << kperp << std::endl;

	// trafo, equ 52 in [eck14]
	t_mat T = ublas::identity_matrix<double>(6);
	T(0,3) = T(1,4) = T(2,5) = -1;
	T(3,0) = codata::hbar*codata::hbar*eck.Q/codata::m_n * (0.5 + s0)	/ meV / angs;
	T(3,3) = codata::hbar*codata::hbar*eck.Q/codata::m_n * (0.5 - s0)	/ meV / angs;
	T(3,1) = codata::hbar*codata::hbar*kperp/codata::m_n				/ meV / angs;
	T(3,4) = -1. * codata::hbar*codata::hbar*kperp/codata::m_n			/ meV / angs;
	T(4,1) = T(5,2) = 0.5 - s0;
	T(4,4) = T(5,5) = 0.5 + s0;

	t_mat Tinv;
	if(!tl::inverse(T, Tinv))
	{
		res.bOk = false;
		res.strErr = "Matrix T cannot be inverted.";
		return res;
	}
	
	//std::cout << "T = " << T << std::endl;
	//std::cout << "Tinv = " << Tinv << std::endl;

	// equ 54 in [eck14]
	t_mat Dalph = tl::rotation_matrix_3d_z(ki_Q/rads);
	//t_mat Dalphphi = tl::rotation_matrix_3d_z((ki_Q + twotheta)/rads);
	t_mat Dalphphi = tl::rotation_matrix_3d_z(kf_Q/rads);
	t_mat Arot = tl::transform(A, Dalph, 1);
	t_mat Erot = tl::transform(E, Dalphphi, 1);

	t_mat matAE = ublas::zero_matrix<double>(6,6);
	tl::submatrix_copy(matAE, Arot, 0,0);
	tl::submatrix_copy(matAE, Erot, 3,3);
	//std::cout << "AE = " << matAE << std::endl;

	//t_mat U1 = tl::transform(matAE, T, 0);
	t_mat matAETinv = ublas::prod(matAE, Tinv);
	t_mat U1 = ublas::prod(T, matAETinv);
	//std::cout << "U1 = " << U1 << std::endl;


	t_vec vecBF = ublas::zero_vector<double>(6);
	t_vec vecBrot = ublas::prod(Dalph, B);
	t_vec vecFrot = ublas::prod(Dalphphi, B);
	tl::subvector_copy(vecBF, vecBrot, 0);
	tl::subvector_copy(vecBF, vecFrot, 3);
	t_vec V1 = ublas::prod(vecBF, Tinv);


	double W1 = C+D+G+H;


	//--------------------------------------------------------------------------
	// integrate last 2 vars -> equs 57 & 58 in [eck14]
	
	//t_mat U = ellipsoid_gauss_int(U1, 5);
	//U = ellipsoid_gauss_int(U, 4);


	t_mat U2(U1.size1()-1, U1.size2()-1);
	t_vec V2(V1.size()-1);
	double W2 = W1 - 0.25*V1[5]/U1(5,5);

	for(int i=0; i<5; ++i)
	{
		for(int j=0; j<5; ++j)
			U2(i,j) = U1(i,j) - U1(i,5)*U1(j,5)/U1(5,5);

		V2[i] = V1[i] - V1[5]*U1(i,5)/U1(5,5);
	}


	t_mat U(U2.size1()-1, U2.size2()-1);
	t_vec V(V2.size()-1);
	double W = W2 - 0.25*V2[4]/U2(4,4);

	for(int i=0; i<4; ++i)
	{
		for(int j=0; j<4; ++j)
			U(i,j) = U2(i,j) - U2(i,4)*U2(j,4)/U2(4,4);

		V[i] = V2[i] - V2[4]*U2(i,4)/U2(4,4);
	}
	
	/*std::cerr << "U = " << U << std::endl;
	std::cerr << "V = " << V << std::endl;
	std::cerr << "W = " << W << std::endl;*/
	//--------------------------------------------------------------------------



	res.reso = 0.5*U*tl::SIGMA2FWHM*tl::SIGMA2FWHM;

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
