/*
 * neutron scattering formulas
 * @author tweber
 * @date 01-may-2013
 */

#ifndef __NEUTRON_FORMULAS_H__
#define __NEUTRON_FORMULAS_H__

#include <boost/units/unit.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/dimensionless_quantity.hpp>
#include <boost/units/physical_dimensions.hpp>
#include <boost/units/systems/si.hpp>
#include <boost/units/systems/angle/degrees.hpp>

#include <boost/units/systems/si/codata/universal_constants.hpp>
#include <boost/units/systems/si/codata/neutron_constants.hpp>
#include <boost/units/systems/si/codata/electromagnetic_constants.hpp>
#include <boost/units/systems/si/codata/physico-chemical_constants.hpp>
#include <boost/units/base_units/metric/angstrom.hpp>
#include <boost/units/cmath.hpp>


namespace ublas = boost::numeric::ublas;
namespace units = boost::units;
namespace codata = boost::units::si::constants::codata;


inline units::quantity<units::si::energy> k2E(const units::quantity<units::si::wavenumber>& k)
{
	units::quantity<units::si::momentum> p = codata::hbar*k;
	units::quantity<units::si::energy> E = p*p / (2.*codata::m_n);
	return E;
}

inline units::quantity<units::si::wavenumber> E2k(const units::quantity<units::si::energy>& E, bool &bImag)
{
	if(E < 0.*one_meV)
		bImag = 1;
	else
		bImag = 0;

	units::quantity<units::si::momentum> p = units::sqrt(2.*codata::m_n*units::abs(E));
	units::quantity<units::si::wavenumber> k = p / codata::hbar;
	return k;
}

inline bool get_twotheta(const units::quantity<units::si::wavenumber>& ki,
							const units::quantity<units::si::wavenumber>& kf,
							const units::quantity<units::si::wavenumber>& Q,
							units::quantity<units::si::plane_angle>& twotheta)
{
	units::quantity<units::si::dimensionless> dCos = (ki*ki + kf*kf - Q*Q) / (2.*ki*kf);
	if(units::abs(dCos) > 1)
		return false;

	twotheta = units::acos(dCos);
	return true;
}

inline units::quantity<units::si::plane_angle>
get_angle_ki_Q(const units::quantity<units::si::wavenumber>& ki,
		const units::quantity<units::si::wavenumber>& kf,
		const units::quantity<units::si::wavenumber>& Q)
{
	if(Q*(1e-10 * units::si::meter) == 0.)
		return M_PI/2. * units::si::radians;

	return units::acos((ki*ki - kf*kf + Q*Q)/(2.*ki*Q));
}

inline units::quantity<units::si::plane_angle>
get_angle_kf_Q(const units::quantity<units::si::wavenumber>& ki,
		const units::quantity<units::si::wavenumber>& kf,
		const units::quantity<units::si::wavenumber>& Q)
{
	if(Q*(1e-10 * units::si::meter) == 0.)
		return M_PI/2. * units::si::radians;

	return M_PI*units::si::radians
			- units::acos((kf*kf - ki*ki + Q*Q)/(2.*kf*Q));
}

#endif
