/*
 * neutron formulas
 * Author: Tobias Weber
 * Date: May 2012, 11-jul-2013
 */

#ifndef __NEUTRONS__
#define __NEUTRONS__

#include "math.h"
#include "linalg.h"
#include "exception.h"

#include <boost/units/unit.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/dimensionless_quantity.hpp>
#include <boost/units/cmath.hpp>
#include <boost/units/physical_dimensions.hpp>
#include <boost/units/systems/si.hpp>
#include <boost/units/systems/angle/degrees.hpp>
//#include <boost/units/systems/temperature/celsius.hpp>
#include <boost/units/systems/si/codata/universal_constants.hpp>
#include <boost/units/systems/si/codata/neutron_constants.hpp>
#include <boost/units/systems/si/codata/electromagnetic_constants.hpp>
#include <boost/units/systems/si/codata/physico-chemical_constants.hpp>
namespace units = boost::units;
namespace co = boost::units::si::constants::codata;

#include <boost/units/base_units/metric/angstrom.hpp>
#include <boost/units/cmath.hpp>

static const auto one_meV = 1e-3 * co::e * units::si::volts;
static const auto one_eV = co::e * units::si::volts;

static const double SIGMA2FWHM = 2.*sqrt(2.*log(2.));
static const double SIGMA2HWHM = SIGMA2FWHM/2.;
static const double HWHM2SIGMA = 1./ SIGMA2HWHM;
static const double FWHM2SIGMA = 1./ SIGMA2FWHM;

static const units::quantity<units::si::length> angstrom = 1e-10 * units::si::meter;

static const double KSQ2E = (co::hbar*co::hbar / (2.*co::m_n)) / one_meV / (angstrom*angstrom);
static const double E2KSQ = 1./KSQ2E;

//static const units::quantity<units::si::length> angstrom = 1e-10 * units::si::meter;


// --------------------------------------------------------------------------------
// de Broglie stuff
// lam = h/p
template<class Sys, class Y>
units::quantity<units::unit<units::momentum_dimension, Sys>, Y>
lam2p(const units::quantity<units::unit<units::length_dimension, Sys>, Y>& lam)
{
	return co::h / lam;
}

template<class Sys, class Y>
units::quantity<units::unit<units::length_dimension, Sys>, Y>
p2lam(const units::quantity<units::unit<units::momentum_dimension, Sys>, Y>& p)
{
	return co::h / p;
}


// lam = 2pi/k
template<class Sys, class Y>
units::quantity<units::unit<units::length_dimension, Sys>, Y>
k2lam(const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>& k)
{
	return 2.*M_PI / k;
}

template<class Sys, class Y>
units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>
lam2k(const units::quantity<units::unit<units::length_dimension, Sys>, Y>& lam)
{
	return 2.*M_PI / lam;
}

template<class Sys, class Y>
units::quantity<units::unit<units::momentum_dimension, Sys>, Y>
k2p(const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>& k)
{
	return co::hbar*k;
}

template<class Sys, class Y>
units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>
p2k(const units::quantity<units::unit<units::momentum_dimension, Sys>, Y>& p)
{
	return p/co::hbar;
}

template<class Sys, class Y>
units::quantity<units::unit<units::velocity_dimension, Sys>, Y>
k2v(const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>& k)
{
	return k2p(k) / co::m_n;
}

template<class Sys, class Y>
units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>
v2k(const units::quantity<units::unit<units::velocity_dimension, Sys>, Y>& v)
{
	return co::m_n*v/co::hbar;
}
// --------------------------------------------------------------------------------


// --------------------------------------------------------------------------------
// E = hbar*omega
template<class Sys, class Y>
units::quantity<units::unit<units::energy_dimension, Sys>, Y>
omega2E(const units::quantity<units::unit<units::frequency_dimension, Sys>, Y>& omega)
{
	return co::hbar * omega;
}

template<class Sys, class Y>
units::quantity<units::unit<units::frequency_dimension, Sys>, Y>
E2omega(const units::quantity<units::unit<units::energy_dimension, Sys>, Y>& en)
{
	return en / co::hbar;
}


template<class Sys, class Y>
units::quantity<units::unit<units::energy_dimension, Sys>, Y>
k2E(const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>& k)
{
	units::quantity<units::unit<units::momentum_dimension, Sys>, Y>
		p = co::hbar*k;
	units::quantity<units::unit<units::energy_dimension, Sys>, Y>
		E = p*p / (2.*co::m_n);
	return E;
}

template<class Sys, class Y>
units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>
E2k(const units::quantity<units::unit<units::energy_dimension, Sys>, Y>& E, bool &bImag)
{
	if(E < 0.*one_meV)
		bImag = 1;
	else
		bImag = 0;

	units::quantity<units::unit<units::momentum_dimension, Sys>, Y>
		p = units::sqrt(2.*co::m_n*units::abs(E));
	units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>
		k = p / co::hbar;
	return k;
}
// --------------------------------------------------------------------------------



// --------------------------------------------------------------------------------
// Bragg
// real: n * lam = 2d * sin(twotheta/2)
template<class Sys, class Y>
units::quantity<units::unit<units::length_dimension, Sys>, Y>
bragg_real_lam(const units::quantity<units::unit<units::length_dimension, Sys>, Y>& d,
				const units::quantity<units::unit<units::plane_angle_dimension, Sys>, Y>& twotheta,
				Y n)
{
	return 2.*d/n * sin(twotheta/2.);
}

template<class Sys, class Y>
units::quantity<units::unit<units::length_dimension, Sys>, Y>
bragg_real_d(const units::quantity<units::unit<units::length_dimension, Sys>, Y>& lam,
			const units::quantity<units::unit<units::plane_angle_dimension, Sys>, Y>& twotheta,
			Y n)
{
	return n * lam / (2.* sin(twotheta/2.));
}

template<class Sys, class Y>
units::quantity<units::unit<units::plane_angle_dimension, Sys>, Y>
bragg_real_twotheta(const units::quantity<units::unit<units::length_dimension, Sys>, Y>& d,
					const units::quantity<units::unit<units::length_dimension, Sys>, Y>& lam,
					Y n)
{
	return asin(n*lam/(2.*d)) * 2.;
}

// reciprocal: Q * lam = 4pi * sin(twotheta/2)
template<class Sys, class Y>
units::quantity<units::unit<units::plane_angle_dimension, Sys>, Y>
bragg_recip_twotheta(const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>& Q,
					const units::quantity<units::unit<units::length_dimension, Sys>, Y>& lam,
					Y n)
{
	return asin(Q*n*lam/(4.*M_PI)) * 2.;
}

template<class Sys, class Y>
units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>
bragg_recip_Q(const units::quantity<units::unit<units::length_dimension, Sys>, Y>& lam,
			const units::quantity<units::unit<units::plane_angle_dimension, Sys>, Y>& twotheta,
			Y n)
{
	return 4.*M_PI / (n*lam) * sin(twotheta/2.);
}

template<class Sys, class Y>
units::quantity<units::unit<units::length_dimension, Sys>, Y>
bragg_recip_lam(const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>& Q,
				const units::quantity<units::unit<units::plane_angle_dimension, Sys>, Y>& twotheta,
				Y n)
{
	return 4.*M_PI / Q * sin(twotheta/2.) / n;
}
// --------------------------------------------------------------------------------


// --------------------------------------------------------------------------------
template<class Sys, class Y>
units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>
kinematic_plane(bool bFixedKi,
				const units::quantity<units::unit<units::energy_dimension, Sys>, Y>& EiEf,
				const units::quantity<units::unit<units::energy_dimension, Sys>, Y>& DeltaE,
				const units::quantity<units::unit<units::plane_angle_dimension, Sys>, Y>& twotheta)
{
	const units::quantity<units::unit<units::energy_dimension, Sys>, Y> dE = DeltaE;
	if(bFixedKi)
		dE = -dE;

	units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y> Q =
			units::sqrt(2.*co::m_n / co::hbar) *
			(2*EiEf + dE - 2.*units::cos(twotheta)*units::sqrt(EiEf*(EiEf + dE)));

	return Q;
}

template<class Sys, class Y>
units::quantity<units::unit<units::energy_dimension, Sys>, Y>
kinematic_plane(bool bFixedKi, bool bBranch,
				const units::quantity<units::unit<units::energy_dimension, Sys>, Y>& EiEf,
				const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>& Q,
				const units::quantity<units::unit<units::plane_angle_dimension, Sys>, Y>& twotheta)
{
	auto c = 2.*co::m_n / (co::hbar*co::hbar);
	Y ctt = units::cos(twotheta);
	Y c2tt = units::cos(2.*twotheta);

	Y dSign = -1.;
	if(bBranch)
		dSign = 1.;

	Y dSignFixedKf = 1.;
	if(bFixedKi)
		dSignFixedKf = -1.;

	auto rt = c*c*c*c * (-EiEf*EiEf)*ctt*ctt
			+ c*c*c*c*EiEf*EiEf*ctt*ctt*c2tt + 2.*c*c*c*EiEf*Q*Q*ctt*ctt;

	units::quantity<units::unit<units::energy_dimension, Sys>, Y> E =
			1./(c*c)*(dSignFixedKf*2.*c*c*EiEf*ctt*ctt
			- dSignFixedKf*2.*c*c*EiEf
			+ dSign*std::sqrt(2.) * units::sqrt(rt)
			+ dSignFixedKf*c*Q*Q);

	return E;
}
// --------------------------------------------------------------------------------



// --------------------------------------------------------------------------------
// Debye-Waller factor

template<class Sys, class Y>
Y debye_waller_high_T(const units::quantity<units::unit<units::temperature_dimension, Sys>, Y>& T_D,
					const units::quantity<units::unit<units::temperature_dimension, Sys>, Y>& T,
					const units::quantity<units::unit<units::mass_dimension, Sys>, Y>& M,
					const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>& Q,
					units::quantity<units::unit<units::derived_dimension<units::length_base_dimension, 2>::type, Sys>, Y>* pZeta_sq=0)
{
	units::quantity<units::unit<units::derived_dimension<units::length_base_dimension, 2>::type, Sys>, Y> zeta_sq;
	zeta_sq = 9.*co::hbar*co::hbar / (co::k_B * T_D * M) * T/T_D;
	Y dwf = units::exp(-1./3. * Q*Q * zeta_sq);

	if(pZeta_sq) *pZeta_sq = zeta_sq;
	return dwf;
}


template<class Sys, class Y>
Y debye_waller_low_T(const units::quantity<units::unit<units::temperature_dimension, Sys>, Y>& T_D,
					const units::quantity<units::unit<units::temperature_dimension, Sys>, Y>& T,
					const units::quantity<units::unit<units::mass_dimension, Sys>, Y>& M,
					const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>& Q,
					units::quantity<units::unit<units::derived_dimension<units::length_base_dimension, 2>::type, Sys>, Y>* pZeta_sq=0)
{
	units::quantity<units::unit<units::derived_dimension<units::length_base_dimension, 2>::type, Sys>, Y> zeta_sq;
	zeta_sq = 9.*co::hbar*co::hbar / (4.*co::k_B*T_D*M) * (1. + 2./3. * M_PI*M_PI * (T/T_D)*(T/T_D));
	Y dwf = units::exp(-1./3. * Q*Q * zeta_sq);

	if(pZeta_sq) *pZeta_sq = zeta_sq;
	return dwf;
}

// --------------------------------------------------------------------------------


// --------------------------------------------------------------------------------
// scattering triangle / TAS stuff

// Q_vec = ki_vec - kf_vec
// kf_vec = ki_vec - Q_vec
// kf^2 = ki^2 + Q^2 - 2ki Q cos th
// cos th = (-kf^2 + ki^2 + Q^2) / (2kiQ)
template<class Sys, class Y>
units::quantity<units::unit<units::plane_angle_dimension, Sys>, Y>
get_angle_ki_Q(const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>& ki,
		const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>& kf,
		const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>& Q,
		bool bPosSense=1)
{
	units::quantity<units::unit<units::plane_angle_dimension, Sys>, Y> angle;

	if(Q*(1e-10 * units::si::meter) == 0.)
		angle = M_PI/2. * units::si::radians;
	else
	{
		auto c = (ki*ki - kf*kf + Q*Q)/(2.*ki*Q);
		if(units::abs(c) > 1.)
			throw Err("Scattering triangle not closed.");

		angle = units::acos(c);
	}

	if(!bPosSense)
		angle = -angle;

	return angle;
}

// Q_vec = ki_vec - kf_vec
// ki_vec = Q_vec + kf_vec
// ki^2 = Q^2 + kf^2 + 2Q kf cos th
// cos th = (ki^2 - Q^2 - kf^2) / (2Q kf)
template<class Sys, class Y>
units::quantity<units::unit<units::plane_angle_dimension, Sys>, Y>
get_angle_kf_Q(const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>& ki,
		const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>& kf,
		const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>& Q,
		bool bPosSense=1)
{
	units::quantity<units::unit<units::plane_angle_dimension, Sys>, Y> angle;

	if(Q*(1e-10 * units::si::meter) == 0.)
		angle = M_PI/2. * units::si::radians;
	else
	{
		auto c = (-kf*kf + ki*ki - Q*Q)/(2.*kf*Q);
		if(units::abs(c) > 1.)
			throw Err("Scattering triangle not closed.");

		angle = units::acos(c);
	}

	if(!bPosSense)
		angle = -angle;

	return angle;
}


template<class Sys, class Y>
units::quantity<units::unit<units::plane_angle_dimension, Sys>, Y>
get_mono_twotheta(const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>& k,
				const units::quantity<units::unit<units::length_dimension, Sys>, Y>& d,
				bool bPosSense=1)
{
	units::quantity<units::unit<units::plane_angle_dimension, Sys>, Y> tt
											= 2. * units::asin(M_PI/(d*k));
	if(!bPosSense)
		tt = -tt;
	return tt;
}

template<class Sys, class Y>
units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>
get_mono_k(const units::quantity<units::unit<units::plane_angle_dimension, Sys>, Y>& _theta,
				const units::quantity<units::unit<units::length_dimension, Sys>, Y>& d,
				bool bPosSense=1)
{
	units::quantity<units::unit<units::plane_angle_dimension, Sys>, Y> theta = _theta;
	if(!bPosSense)
		theta = -theta;

	return M_PI/(units::sin(theta) * d);
}


// Q_vec = ki_vec - kf_vec
// Q^2 = ki^2 + kf^2 - 2ki kf cos 2th
// cos 2th = (-Q^2 + ki^2 + kf^2) / (2ki kf)
template<class Sys, class Y>
units::quantity<units::unit<units::plane_angle_dimension, Sys>, Y>
get_sample_twotheta(const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>& ki,
					const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>& kf,
					const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>& Q,
					bool bPosSense=1)
{
	units::quantity<units::si::dimensionless> ttCos
							= (ki*ki + kf*kf - Q*Q)/(2.*ki*kf);
	if(units::abs(ttCos) > 1.)
		throw Err("Scattering triangle not closed.");

	units::quantity<units::unit<units::plane_angle_dimension, Sys>, Y> tt;
	tt = units::acos(ttCos);
	if(!bPosSense)
		tt = -tt;

	return tt;
}

template<class Sys, class Y>
const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>
get_sample_Q(const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>& ki,
					const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>& kf,
					const units::quantity<units::unit<units::plane_angle_dimension, Sys>, Y>& tt)
{
	units::quantity<units::si::dimensionless> ctt = units::cos(tt);
	units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>
		Q = units::sqrt(-ctt*(2.*ki*kf) + ki*ki + kf*kf);
	return Q;
}



template<class Sys, class Y>
units::quantity<units::unit<units::energy_dimension, Sys>, Y>
get_energy_transfer(const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>& ki,
					const units::quantity<units::unit<units::wavenumber_dimension, Sys>, Y>& kf)
{
	return co::hbar*co::hbar*ki*ki/(2.*co::m_n)
			- co::hbar*co::hbar*kf*kf/(2.*co::m_n);
}

// --------------------------------------------------------------------------------


// --------------------------------------------------------------------------------
// spurions

// inelastic spurions -> Shirane pp. 146-148
template<class Sys, class Y>
units::quantity<units::unit<units::energy_dimension, Sys>, Y>
get_inelastic_spurion(bool bConstEi,
		units::quantity<units::unit<units::energy_dimension, Sys>, Y> E,
		unsigned int iOrderMono, unsigned int iOrderAna)
{
	const double dOrderMonoSq = double(iOrderMono)*double(iOrderMono);
	const double dOrderAnaSq = double(iOrderAna)*double(iOrderAna);

	units::quantity<units::unit<units::energy_dimension, Sys>, Y> E_sp;

	// formulas from Shirane, p. 147
	if(bConstEi)
		E_sp = (1. - dOrderMonoSq/dOrderAnaSq) * E;
	else
		E_sp = (dOrderAnaSq/dOrderMonoSq - 1.) * E;

	return E_sp;
}

struct InelasticSpurion
{
	double dE_meV = 0.;
	unsigned int iOrderMono = 1;
	unsigned int iOrderAna = 1;
};

template<class Sys, class Y>
std::vector<InelasticSpurion> check_inelastic_spurions(bool bConstEi,
			units::quantity<units::unit<units::energy_dimension, Sys>, Y> Ei,
			units::quantity<units::unit<units::energy_dimension, Sys>, Y> Ef,
			units::quantity<units::unit<units::energy_dimension, Sys>, Y> E,
			unsigned int iMaxOrder=5)
{
	const double dESensitivity = 0.25;	// meV

	std::vector<InelasticSpurion> vecSpuris;

	for(unsigned int iOrder=1; iOrder<=iMaxOrder; ++iOrder)
	{
		InelasticSpurion spuri;
		units::quantity<units::unit<units::energy_dimension, Sys>, Y> EiEf;

		if(bConstEi)
		{
			spuri.iOrderAna = iOrder;
			EiEf = Ei;
		}
		else
		{
			spuri.iOrderMono = iOrder;
			EiEf = Ef;
		}

		spuri.dE_meV = get_inelastic_spurion(bConstEi, EiEf,
							spuri.iOrderMono, spuri.iOrderAna) / one_meV;

		//std::cout << spuri.dE_meV << " *** " << Y(E/one_meV) << std::endl;
		if(spuri.dE_meV!=0. && float_equal(spuri.dE_meV, Y(E/one_meV), dESensitivity))
			vecSpuris.push_back(spuri);
	}

	return vecSpuris;
}

struct ElasticSpurion
{
	bool bAType = 0;
	bool bMType = 0;

	bool bAKfSmallerKi = 0;
	bool bMKfSmallerKi = 0;
};

// accidental elastic spurions -> Shirane pp. 150-155 (esp. fig. 6.2)
template<typename T=double>
ElasticSpurion check_elastic_spurion(const ublas::vector<T>& ki,
							const ublas::vector<T>& kf,
							const ublas::vector<T>& q)
{
	const double dKi = ublas::norm_2(ki);
	const double dKf = ublas::norm_2(kf);
	const double dq = ublas::norm_2(q);

	const double dAngleSensitivity = 2.;
	const double dQSensitivity = std::max(dKi, dKf) / 50.;


	ElasticSpurion result;

	ublas::vector<T> ki_norm = ki;	ki_norm /= dKi;
	ublas::vector<T> kf_norm = kf;	kf_norm /= dKf;

	// Q, q and G point in the opposite direction in Shirane!
	// Shirane: Q = kf - ki, E = Ei - Ef
	// here: Q = ki - kf, E = Ei - Ef
	ublas::vector<T> q_norm = -q;	q_norm /= dq;

	double dAngleKfq = std::acos(ublas::inner_prod(kf_norm, q_norm));
	double dAngleKiq = std::acos(ublas::inner_prod(ki_norm, q_norm));

	//std::cout << "angle ki q: " << dAngleKiq/M_PI*180. << std::endl;
	//std::cout << "angle kf q: " << dAngleKfq/M_PI*180. << std::endl;

	bool bKiqParallel = 0, bkiqAntiParallel = 0;
	bool bKfqParallel = 0, bKfqAntiParallel = 0;

	if(float_equal(dAngleKiq, 0., dAngleSensitivity/180.*M_PI))
		bKiqParallel = 1;
	else if(float_equal(dAngleKiq, M_PI, dAngleSensitivity/180.*M_PI))
		bkiqAntiParallel = 1;
	if(float_equal(dAngleKfq, 0., dAngleSensitivity/180.*M_PI))
		bKfqParallel = 1;
	else if(float_equal(dAngleKfq, M_PI, dAngleSensitivity/180.*M_PI))
		bKfqAntiParallel = 1;

	// type A: q || kf, kf > ki
	if(bKfqParallel)
	{
		double dApparentKf = dKf - dq;

		if(::float_equal(dApparentKf, dKi, dQSensitivity))
		{
			result.bAType = 1;
			result.bAKfSmallerKi = 0;
		}
	}
	// type A: q || kf, kf < ki
	else if(bKfqAntiParallel)
	{
		double dApparentKf = dKf + dq;

		if(::float_equal(dApparentKf, dKi, dQSensitivity))
		{
			result.bAType = 1;
			result.bAKfSmallerKi = 1;
		}
	}

	// type M: q || ki, kf > ki
	if(bKiqParallel)
	{
		double dApparentKi = dKi + dq;

		if(::float_equal(dApparentKi, dKf, dQSensitivity))
		{
			result.bMType = 1;
			result.bMKfSmallerKi = 0;
		}
	}
	// type M: q || ki, kf < ki
	else if(bkiqAntiParallel)
	{
		double dApparentKi = dKi - dq;

		if(::float_equal(dApparentKi, dKf, dQSensitivity))
		{
			result.bMType = 1;
			result.bMKfSmallerKi = 1;
		}
	}

	return result;
}


// --------------------------------------------------------------------------------

#endif
