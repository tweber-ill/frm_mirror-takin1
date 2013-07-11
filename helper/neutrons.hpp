/*
 * neutron formulas
 * Author: Tobias Weber
 * Date: May 2012, 11-jul-2013
 */

#ifndef __NEUTRON_FORMULAS__
#define __NEUTRON_FORMULAS__

#include <cmath>

#include <boost/units/unit.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/dimensionless_quantity.hpp>
#include <boost/units/cmath.hpp>
#include <boost/units/physical_dimensions.hpp>
#include <boost/units/systems/si.hpp>
#include <boost/units/systems/angle/degrees.hpp>
#include <boost/units/systems/si/codata/universal_constants.hpp>
#include <boost/units/systems/si/codata/neutron_constants.hpp>
#include <boost/units/systems/si/codata/electromagnetic_constants.hpp>
#include <boost/units/systems/si/codata/physico-chemical_constants.hpp>
namespace units = boost::units;
namespace co = boost::units::si::constants::codata;

#include <boost/units/base_units/metric/angstrom.hpp>
#include <boost/units/cmath.hpp>


#ifndef one_meV
	#define one_meV (1e-3 * co::e * units::si::volts)
#endif

static const double SIGMA2FWHM = 2.*sqrt(2.*log(2.));
static const double SIGMA2HWHM = SIGMA2FWHM/2.;

static const units::quantity<units::si::length> angstrom = 1e-10 * units::si::meter;

static const double KSQ2E = (co::hbar*co::hbar / (2.*co::m_n)) / one_meV / (angstrom*angstrom);
static const double E2KSQ = 1./KSQ2E;

//static const units::quantity<units::si::length> angstrom = 1e-10 * units::si::meter;


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

#endif
