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

template<class> struct is_1d_type : std::false_type {};
template<class... PARAMS> struct is_1d_type<std::vector<PARAMS...>> : std::true_type {};
template<class... PARAMS> struct is_1d_type<std::array<PARAMS...>> : std::true_type {};
template<class... PARAMS> struct is_1d_type<std::list<PARAMS...>> : std::true_type {};
template<class... PARAMS> struct is_1d_type<boost::numeric::ublas::vector<PARAMS...>> : std::true_type {};
template<class... PARAMS> struct is_1d_type<std::initializer_list<PARAMS...>> : std::true_type {};

template<class> struct is_2d_type : std::false_type {};
template<class... PARAMS> struct is_2d_type<boost::numeric::ublas::matrix<PARAMS...>> : std::true_type {};
// -----------------------------------------------------------------------------



template<class T>
struct remove_constref
{
	typedef typename std::remove_const<
		typename std::remove_reference<T>::type
			>::type type;
};



#endif
