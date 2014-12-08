/*
 * basic quaternion helpers
 *
 * @author: tweber
 * @date: 2013, 3-dec-2014
 */

 #ifndef __TAZ_QUAT_H__
 #define __TAZ_QUAT_H__

#include <boost/math/quaternion.hpp>
namespace math = boost::math;

#include "linalg.h"

// algo from:
// http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q55
template<typename T=double>
math::quaternion<T> rot3_to_quat(const ublas::matrix<T>& rot)
{
	T tr = trace(rot) + 1.;
	T x,y,z,w;

	if(tr > std::numeric_limits<T>::epsilon())				// w largest
	{
		T s = std::sqrt(tr) * 2.;
		x = (rot(2,1) - rot(1,2)) / s;
		y = (rot(0,2) - rot(2,0)) / s;
		z = (rot(1,0) - rot(0,1)) / s;
		w = s/4.;
	}
	else
	{
		if(rot(0,0)>=rot(1,1) && rot(0,0)>=rot(2,2))		// x largest
		{
			T s = std::sqrt(1. + rot(0,0) - rot(1,1) - rot(2,2)) * 2.;
			x = s/4.;
			y = (rot(1,0) + rot(0,1)) / s;
			z = (rot(0,2) + rot(2,0)) / s;
			w = (rot(2,1) - rot(1,2)) / s;
		}
		else if(rot(1,1)>=rot(0,0) && rot(1,1)>=rot(2,2))	// y largest
		{
			T s = std::sqrt(1. + rot(1,1) - rot(0,0) - rot(2,2)) * 2.;
			x = (rot(1,0) + rot(0,1)) / s;
			y = s/4.;
			z = (rot(2,1) + rot(1,2)) / s;
			w = (rot(0,2) - rot(2,0)) / s;
		}
		else if(rot(2,2)>=rot(0,0) && rot(2,2)>=rot(1,1))	// z largest
		{
			T s = std::sqrt(1. + rot(2,2) - rot(0,0) - rot(1,1)) * 2.;
			x = (rot(0,2) + rot(2,0)) / s;
			y = (rot(2,1) + rot(1,2)) / s;
			z = s/4.;
			w = (rot(1,0) - rot(0,1)) / s;
		}
		else
		{
			throw Err("rot3_to_quat: Invalid condition.");
		}
	}

	T norm_eucl = std::sqrt(w*w + x*x + y*y + z*z);
	math::quaternion<T> quatRet(w,x,y,z);
	return quatRet /*/ math::norm(quatRet)*/ / norm_eucl;
}


template<typename T=double>
ublas::matrix<T> quat_to_rot3(const math::quaternion<T>& quat)
{
	const math::quaternion<T> cquat = math::conj(quat);
	const math::quaternion<T> i(0,1,0,0), j(0,0,1,0), k(0,0,0,1);

	const math::quaternion<T> cols[] = {quat*i*cquat,
										quat*j*cquat,
										quat*k*cquat};

	ublas::matrix<T> mat(3,3);
	for(unsigned int icol=0; icol<3; ++icol)
	{
		mat(0, icol) = cols[icol].R_component_2();
		mat(1, icol) = cols[icol].R_component_3();
		mat(2, icol) = cols[icol].R_component_4();
	}

	return mat;
}


template<typename T=double>
std::vector<T> quat_to_euler(const math::quaternion<T>& quat)
{
	T q[] = {quat.R_component_1(),
			quat.R_component_2(),
			quat.R_component_3(),
			quat.R_component_4()};

	// formulas from:
	// http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
	T phi = std::atan2(2.*(q[0]*q[1] + q[2]*q[3]), 1.-2.*(q[1]*q[1] + q[2]*q[2]));
	T theta = std::asin(2.*(q[0]*q[2] - q[3]*q[1]));
	T psi = std::atan2(2.*(q[0]*q[3] + q[1]*q[2]), 1.-2.*(q[2]*q[2] + q[3]*q[3]));

	std::vector<T> vec = { phi, theta, psi };
	return vec;
}

template<typename T=double>
std::vector<T> rotation_angle(const ublas::matrix<T>& rot)
{
	std::vector<T> vecResult;

	if(rot.size1()!=rot.size2())
		return vecResult;
	if(rot.size1()<2)
		return vecResult;

	if(rot.size2()==2)
	{
		// rot = ( c -s )
		//       ( s  c )

		T angle = std::atan2(rot(1,0), rot(0,0));
		vecResult.push_back(angle);
	}
	else if(rot.size2()==3)
	{
		math::quaternion<T> quat = rot3_to_quat(rot);
		vecResult = quat_to_euler(quat);
	}

	return vecResult;
}


template<typename T=double>
math::quaternion<T> stereo_proj(const math::quaternion<T>& quat)
{ return (1.+quat)/(1.-quat); }

template<typename T=double>
math::quaternion<T> stereo_proj_inv(const math::quaternion<T>& quat)
{ return (1.-quat)/(1.+quat); }


template<class QUAT>
struct vec_angle_unsigned_impl<QUAT, LinalgType::QUATERNION>
{
	typename QUAT::value_type operator()(const QUAT& q1, const QUAT& q2) const
	{
		typedef typename QUAT::value_type REAL;
		REAL dot = q1.R_component_1()*q2.R_component_1() +
					q1.R_component_2()*q2.R_component_2() +
					q1.R_component_3()*q2.R_component_3() +
					q1.R_component_4()*q2.R_component_4();

		dot /= math::norm(q1);
		dot /= math::norm(q2);

		return std::acos(dot);
	}
};

#endif
