/*
 * Custom type traits
 * @author Tobias Weber
 * @date 19-nov-2014
 */
 
#ifndef __MY_TRAITS_H__
#define __MY_TRAITS_H__

#include <type_traits>


// -----------------------------------------------------------------------------
template<class T, bool bScalar=std::is_scalar<T>::value>
struct underlying_value_type
{};

template<class T>
struct underlying_value_type<T, 1>
{
	typedef T value_type;
};

template<class T>
struct underlying_value_type<T, 0>
{
	typedef typename T::value_type value_type;
};
// -----------------------------------------------------------------------------



// -----------------------------------------------------------------------------
#include <vector>
#include <array>
#include <list>
#include <initializer_list>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>

typedef std::integral_constant<int, 0>	dim_0d_type;
typedef std::integral_constant<int, 1>	dim_1d_type;
typedef std::integral_constant<int, 2>	dim_2d_type;

template<class> struct get_type_dim : dim_0d_type {};

template<class... PARAMS> struct get_type_dim<std::vector<PARAMS...>> : dim_1d_type {};
template<class... PARAMS> struct get_type_dim<std::array<PARAMS...>> : dim_1d_type {};
template<class... PARAMS> struct get_type_dim<std::list<PARAMS...>> : dim_1d_type {};
template<class... PARAMS> struct get_type_dim<boost::numeric::ublas::vector<PARAMS...>> : dim_1d_type {};
template<class... PARAMS> struct get_type_dim<std::initializer_list<PARAMS...>> : dim_1d_type {};

template<class... PARAMS> struct get_type_dim<boost::numeric::ublas::matrix<PARAMS...>> : dim_2d_type {};
// -----------------------------------------------------------------------------



template<class T>
struct remove_constref
{
	typedef typename std::remove_const<
		typename std::remove_reference<T>::type
			>::type type;
};



#endif
