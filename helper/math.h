/*
 * math helpers
 *
 * @author: tweber
 * @date: 23-apr-2013
 */

#ifndef __MIEZE_MATH__
#define __MIEZE_MATH__

#include <cmath>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
//namespace ublas = boost::numeric::ublas;

template<typename T=double>
boost::numeric::ublas::matrix<T> rotation_matrix_2d(T angle)
{
	boost::numeric::ublas::matrix<T> mat(2,2);

	T s = std::sin(angle);
	T c = std::cos(angle);

	mat(0,0) = c; mat(0,1) = -s;
	mat(1,0) = s; mat(1,1) = c;

	return mat;
}

#endif
