/*
 * implementation of the eckold-sobolev algo
 *
 * @author tweber
 * @date feb-2015
 * @copyright GPLv2
 *
 * @desc algorithm: [eck14] G. Eckold and O. Sobolev, NIM A 752, pp. 54-64 (2014)
 */

#include "eck.h"

#include "tlibs/math/linalg.h"
#include "tlibs/math/math.h"
#include "ellipse.h"

#include <tuple>
#include <future>
#include <string>
#include <iostream>
#include <boost/units/pow.hpp>
#include <boost/units/io.hpp>

typedef double t_real;
typedef ublas::matrix<t_real> t_mat;
typedef ublas::vector<t_real> t_vec;

using tl::angle; using tl::wavenumber; using tl::energy; using tl::length;
static const auto cm = tl::cm;
static const auto secs = tl::seconds;
static const auto angs = tl::angstrom;
static const auto rads = tl::radians;
static const auto meV = tl::meV;


static std::tuple<t_mat, t_vec, t_real, t_real, t_real>
get_mono_vals(const length& src_w, const length& src_h,
			const length& mono_w, const length& mono_h,
			const length& dist_src_mono, const length& dist_mono_sample,
			const wavenumber& ki, const angle& thetam,
			const angle& coll_h_pre_mono, const angle& coll_h_pre_sample,
			const angle& coll_v_pre_mono, const angle& coll_v_pre_sample,
			const angle& mono_mosaic, const angle& mono_mosaic_v,
			const length& mono_curvh, const length& mono_curvv,
			const length& pos_x , const length& pos_y, const length& pos_z,
			t_real dRefl)
{
	// A matrix: formula 26 in [eck14]
	t_mat A = ublas::identity_matrix<t_real>(3);
	A(0,0) = 4.*std::log(2.)/(ki*angs*ki*angs) * units::tan(thetam)*units::tan(thetam) *
		( units::pow<2>(2./(coll_h_pre_mono/rads)) +
		  units::pow<2>(2.*dist_src_mono/src_w) +
		  units::pow<2>(1./(mono_mosaic/rads))
		);
	A(0,1) = A(1,0) = 4.*std::log(2.)/(ki*angs*ki*angs) * units::tan(thetam) *
		( 2.*units::pow<2>(1./(coll_h_pre_mono/rads)) +
		  2.*dist_src_mono*(dist_src_mono-dist_mono_sample)/(src_w*src_w) +
		  units::pow<2>(1./(mono_mosaic/rads)) *
			(1. - dist_mono_sample/(mono_curvh*std::fabs(units::sin(thetam))))
		);
	A(1,1) = 4.*std::log(2.)/(ki*angs*ki*angs) *
		( units::pow<2>(1./(coll_h_pre_mono/rads)) +
		  units::pow<2>(1./(coll_h_pre_sample/rads)) +
		  units::pow<2>((dist_src_mono-dist_mono_sample)/src_w) +
		  units::pow<2>(dist_mono_sample/(mono_w*std::fabs(units::sin(thetam)))) +
		  units::pow<2>(1./(mono_mosaic/rads)) *
			units::pow<2>(1. - dist_mono_sample/(mono_curvh*std::fabs(units::sin(thetam))))
		);

	// Av matrix: formula 38 in [eck14]
	t_mat Av(2,2);
	Av(0,0) = 4.*std::log(2.)/(ki*angs*ki*angs) *
		( units::pow<2>(1./(coll_v_pre_sample/rads)) +
		  units::pow<2>(dist_mono_sample/src_h) +
		  units::pow<2>(dist_mono_sample/mono_h) +
		  units::pow<2>(1./(2.*mono_mosaic_v/rads*units::sin(thetam))) +
		  dist_mono_sample/(mono_mosaic_v/rads*mono_mosaic_v/rads*mono_curvv*std::fabs(units::sin(thetam)))
		);
	Av(0,1) = Av(1,0) = 4.*std::log(2.)/(ki*angs*ki*angs) *
		( dist_src_mono*dist_mono_sample/(src_h*src_h) -
		  units::pow<2>(1./(2.*mono_mosaic_v/rads*units::sin(thetam))) +
		  dist_mono_sample/(2.*mono_mosaic_v/rads*mono_mosaic_v/rads*mono_curvv*std::fabs(units::sin(thetam)))
		);
	Av(1,1) = 4.*std::log(2.)/(ki*angs*ki*angs) *
		( units::pow<2>(1./(coll_v_pre_mono/rads)) +
		  units::pow<2>(dist_src_mono/src_h) +
		  units::pow<2>(1./(2.*mono_mosaic_v/rads*units::sin(thetam)))
		);


	// B vector: formula 27 in [eck14]
	t_vec B(3);
	B(0) = 8.*std::log(2.)*pos_y/(ki*angs) * units::tan(thetam) *
		( 2.*dist_src_mono/(src_w*src_w) +
		  1./((mono_mosaic/rads)*(mono_mosaic/rads) * mono_curvh*std::fabs(units::sin(thetam)))
		);
	B(1) = 8.*std::log(2.)*pos_y/(ki*angs) *
		( -dist_mono_sample/(mono_w*std::fabs(units::sin(thetam)) * mono_w*std::fabs(units::sin(thetam))) +
		  (1./((mono_mosaic/rads)*(mono_mosaic/rads) * mono_curvh*std::fabs(units::sin(thetam)))) *
			(1. - dist_mono_sample/(mono_curvh*std::fabs(units::sin(thetam)))) +
		  (dist_src_mono-dist_mono_sample)/(src_w*src_w)
		);

	// Bv vector: formula 39 in [eck14]
	t_vec Bv(2);
	Bv(0) = 4.*std::log(2.)*pos_z / (ki*angs) *
		( -2.*dist_mono_sample / (src_h*src_h) +		// typo in paper?
		  1./(mono_mosaic_v/rads*mono_mosaic_v/rads*mono_curvv*std::fabs(units::sin(thetam))) -
		  2.*dist_mono_sample/(mono_mosaic_v/rads*mono_mosaic_v/rads*mono_curvv*mono_curvv) -
		  2.*dist_mono_sample / (mono_h*mono_h)			// typo in paper?
		);
	Bv(1) = 4.*std::log(2.)*pos_z / (ki*angs) *
		( -2.*dist_mono_sample / (src_h*src_h) -		// typo in paper?
		  1./(mono_mosaic_v/rads*mono_mosaic_v/rads*mono_curvv*std::fabs(units::sin(thetam)))
		);


	// C scalar: formula 28 in [eck14]
	t_real C = 4.*std::log(2.)*pos_y*pos_y *
		( 1./(src_w*src_w) +
		  units::pow<2>(1./(mono_w*std::fabs(units::sin(thetam)))) +
		  units::pow<2>(1./(mono_mosaic/rads * mono_curvh*std::fabs(units::sin(thetam))))
		);

	// Cv scalar: formula 40 in [eck14]
	t_real Cv = 4.*std::log(2.)*pos_z*pos_z *
		( 1./(src_h*src_h) +
		  1./(mono_h*mono_h) +
		  units::pow<2>(1./((mono_mosaic_v/rads * mono_curvv)))
		);


	// z components, [eck14], equ. 42
	A(2,2) = Av(0,0) - Av(0,1)*Av(0,1)/Av(1,1);
	B[2] = Bv[0] - Bv[1]*Av(0,1)/Av(1,1);
	t_real D = Cv - 0.25*Bv[1]/Av(1,1);


	// [eck14], equ. 54
	t_real refl = dRefl * std::sqrt(M_PI/Av(1,1));


	return std::make_tuple(A, B, C, D, refl);
}


CNResults calc_eck(const EckParams& eck)
{
	length mono_curvh = eck.mono_curvh * eck.dmono_sense;
	length mono_curvv = eck.mono_curvv * eck.dmono_sense;
	length ana_curvh = eck.ana_curvh * eck.dana_sense;
	length ana_curvv = eck.ana_curvv * eck.dana_sense;

	if(!eck.bMonoIsCurvedH) mono_curvh = 99999. * cm * eck.dmono_sense;
	if(!eck.bMonoIsCurvedV) mono_curvv = 99999. * cm * eck.dmono_sense;
	if(!eck.bAnaIsCurvedH) ana_curvh = 99999. * cm * eck.dana_sense;
	if(!eck.bAnaIsCurvedV) ana_curvv = 99999. * cm * eck.dana_sense;

	angle twotheta = eck.twotheta;
	angle thetaa = eck.thetaa;
	angle thetam = eck.thetam;
	angle ki_Q = eck.angle_ki_Q;
	angle kf_Q = eck.angle_kf_Q;
	//kf_Q = ki_Q + twotheta;

	if(eck.dsample_sense < 0) { twotheta = -twotheta; ki_Q = -ki_Q; kf_Q = -kf_Q; }

	//std::cout << "thetaM = " << t_real(thetam/rads/M_PI*180.) << " deg"<< std::endl;
	//std::cout << "thetaA = " << t_real(thetaa/rads/M_PI*180.) << " deg"<< std::endl;

	//std::cout << "ki = " << double(eck.ki*angs) << ", kf = " << double(eck.kf*angs) << std::endl;
	//std::cout << "Q = " << double(eck.Q*angs) << ", E = " << double(eck.E/meV) << std::endl;

	//std::cout << "kiQ = " << t_real(ki_Q/rads/M_PI*180.) << " deg"<< std::endl;
	//std::cout << "kfQ = " << t_real(kf_Q/rads/M_PI*180.) << " deg"<< std::endl;
	//std::cout << "2theta = " << t_real(twotheta/rads/M_PI*180.) << " deg"<< std::endl;


	CNResults res;

	res.Q_avg.resize(4);
	res.Q_avg[0] = eck.Q*angs;
	res.Q_avg[1] = 0.;
	res.Q_avg[2] = 0.;
	res.Q_avg[3] = eck.E/meV;


	//--------------------------------------------------------------------------
	// mono part

	std::launch lpol = /*std::launch::deferred |*/ std::launch::async;
	std::future<std::tuple<t_mat, t_vec, t_real, t_real, t_real>> futMono
		= std::async(lpol, get_mono_vals,
					eck.src_w, eck.src_h,
					eck.mono_w, eck.mono_h,
					eck.dist_src_mono, eck.dist_mono_sample,
					eck.ki, thetam,
					eck.coll_h_pre_mono, eck.coll_h_pre_sample,
					eck.coll_v_pre_mono, eck.coll_v_pre_sample,
					eck.mono_mosaic, eck.mono_mosaic_v,
					mono_curvh, mono_curvv,
					eck.pos_x , eck.pos_y, eck.pos_z,
					eck.dmono_refl);

	//--------------------------------------------------------------------------


	//--------------------------------------------------------------------------
	// ana part

	// equ 43 in [eck14]
	length pos_y2 = -eck.pos_x*units::sin(twotheta)
					+eck.pos_y*units::cos(twotheta);
	std::future<std::tuple<t_mat, t_vec, t_real, t_real, t_real>> futAna
		= std::async(lpol, get_mono_vals,
					eck.det_w, eck.det_h,
					eck.ana_w, eck.ana_h,
					eck.dist_ana_det, eck.dist_sample_ana,
					eck.kf, -thetaa,
					eck.coll_h_post_ana, eck.coll_h_post_sample,
					eck.coll_v_post_ana, eck.coll_v_post_sample,
					eck.ana_mosaic, eck.ana_mosaic_v,
					ana_curvh, ana_curvv,
					eck.pos_x, pos_y2, eck.pos_z,
					eck.dana_effic);

	//--------------------------------------------------------------------------
	// get mono & ana results

	std::tuple<t_mat, t_vec, t_real, t_real, t_real> tupMono = futMono.get();
	const t_mat& A = std::get<0>(tupMono);
	const t_vec& B = std::get<1>(tupMono);
	const t_real& C = std::get<2>(tupMono);
	const t_real& D = std::get<3>(tupMono);
	const t_real& dReflM = std::get<4>(tupMono);

	std::tuple<t_mat, t_vec, t_real, t_real, t_real> tupAna = futAna.get();
	const t_mat& E = std::get<0>(tupAna);
	const t_vec& F = std::get<1>(tupAna);
	const t_real& G = std::get<2>(tupAna);
	const t_real& H = std::get<3>(tupAna);
	const t_real& dReflA = std::get<4>(tupAna);

	/*std::cout << "A = " << A << std::endl;
	std::cout << "B = " << B << std::endl;
	std::cout << "C = " << C << std::endl;
	std::cout << "D = " << D << std::endl;
	std::cout << "RM = " << dReflM << std::endl;

	std::cout << "E = " << E << std::endl;
	std::cout << "F = " << F << std::endl;
	std::cout << "G = " << G << std::endl;
	std::cout << "H = " << H << std::endl;
	std::cout << "RA = " << dReflA << std::endl;*/

	//--------------------------------------------------------------------------


	// equ 4 & equ 53 in [eck14]
	t_real s0 = (eck.ki*eck.ki - eck.kf*eck.kf) / (2. * eck.Q*eck.Q);
	wavenumber kperp = units::sqrt(units::abs(eck.Q*eck.Q*(0.5 + s0)*(0.5 + s0) -
												eck.ki*eck.ki));
	if(eck.dsample_sense*twotheta/rads < 0.)
		kperp = -kperp;

	//std::cout << "s0 = " << s0 << std::endl;
	//std::cout << "kperp = " << t_real(kperp*angs) << " / A" << std::endl;

	// trafo, equ 52 in [eck14]
	t_mat T = ublas::identity_matrix<t_real>(6);
	T(0,3) = T(1,4) = T(2,5) = -1.;
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
	t_mat Dalph_i = tl::rotation_matrix_3d_z(-ki_Q/rads);
	t_mat Dalph_f = tl::rotation_matrix_3d_z(-kf_Q/rads);
	t_mat Arot = tl::transform(A, Dalph_i, 1);
	t_mat Erot = tl::transform(E, Dalph_f, 1);

	t_mat matAE = ublas::zero_matrix<t_real>(6,6);
	tl::submatrix_copy(matAE, Arot, 0,0);
	tl::submatrix_copy(matAE, Erot, 3,3);
	//std::cout << "AE = " << matAE << std::endl;

	// U1 matrix
	t_mat U1 = tl::transform(matAE, Tinv, 1);	// typo in paper in quadric trafo in equ 54 (top)?
	//std::cout << "U1 = " << U1 << std::endl;

	// V1 vector
	t_vec vecBF = ublas::zero_vector<t_real>(6);
	t_vec vecBrot = ublas::prod(ublas::trans(Dalph_i), B);
	t_vec vecFrot = ublas::prod(ublas::trans(Dalph_f), F);
	tl::subvector_copy(vecBF, vecBrot, 0);
	tl::subvector_copy(vecBF, vecFrot, 3);
	t_vec V1 = ublas::prod(vecBF, Tinv);


	t_real W1 = C+D+G+H;
	t_real Z1 = dReflM*dReflA;


	//--------------------------------------------------------------------------
	// integrate last 2 vars -> equs 57 & 58 in [eck14]

	t_mat U2 = ellipsoid_gauss_int(U1, 5);
	t_mat U = ellipsoid_gauss_int(U2, 4);

	t_vec V2 = ellipsoid_gauss_int(V1, U1, 5);
	t_vec V = ellipsoid_gauss_int(V2, U2, 4);

	t_real W2 = W1 - 0.25*V1[5]/U1(5,5);
	t_real W = W2 - 0.25*V2[4]/U2(4,4);

	t_real Z2 = Z1*std::sqrt(M_PI/std::fabs(U1(5,5)));
	t_real Z = Z2*std::sqrt(M_PI/std::fabs(U2(4,4)));

	/*std::cout << "U = " << U << std::endl;
	std::cout << "V = " << V << std::endl;
	std::cout << "W = " << W << std::endl;
	std::cout << "Z = " << Z << std::endl;*/
	//--------------------------------------------------------------------------


	// quadratic part of quadric (matrix U)
	res.reso = 0.5*0.5*U*tl::SIGMA2FWHM*tl::SIGMA2FWHM;
	// linear (vector V) and constant (scalar W) part of quadric
	res.reso_v = V;
	res.reso_s = W;

	// prefactor and volume
	res.dR0 = Z;
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
