/*
 * basic linalg helpers
 *
 * @author: tweber
 * @date: 30-apr-2013
 */

#include "linalg.h"
#include "math.h"

template<>
double vec_angle_unsigned(const math::quaternion<double>& q1,
				const math::quaternion<double>& q2)
{
	double dot = q1.R_component_1()*q2.R_component_1() +
				q1.R_component_2()*q2.R_component_2() +
				q1.R_component_3()*q2.R_component_3() +
				q1.R_component_4()*q2.R_component_4();

	dot /= math::norm(q1);
	dot /= math::norm(q2);

	return std::acos(dot);
}
