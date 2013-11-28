/*
 * neutron scattering formulas
 * @author tweber
 * @date 01-may-2013
 */

#ifndef __TAS_FORMULAS_H__
#define __TAS_FORMULAS_H__

#include "../../helper/neutrons.hpp"

// Q_vec = ki_vec - kf_vec
// Q^2 = ki^2 + kf^2 - 2ki kf cos 2th
// cos 2th = (-Q^2 + ki^2 + kf^2) / (2ki kf)
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
