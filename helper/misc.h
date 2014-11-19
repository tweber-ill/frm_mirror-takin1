/*
 * misc helper
 * @author tweber
 * @date 07-mar-2013
 */

#ifndef __MIEZE_MISC_HELPER__
#define __MIEZE_MISC_HELPER__

#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <iterator>
#include <tuple>
#include "math.h"
//#include "../data/data.h"

// deletes an object when going out of scope
template<class T> class autodeleter
{
protected:
	T *m_t;
	bool m_bIsArray;

public:
	autodeleter(T* t, bool bIsArray=false) : m_t(t), m_bIsArray(bIsArray)
	{}

	~autodeleter()
	{
		if(m_t)
		{
			if(m_bIsArray)
				delete[] m_t;
			else
				delete m_t;
			m_t = 0;
		}
	}
};

template<typename T> T safe_log10(T t)
{
	const T t_invalid = T(-10);

	if(t > T(0))
		return log10(t);

	return t_invalid;
}

// pixel -> val
template<typename T=double>
T tic_trafo(unsigned int iDim, T dMin, T dMax, bool bLog, T dPix)
{
	if(bLog)
	{
		dMin = safe_log10<T>(dMin);
		dMax = safe_log10<T>(dMax);
	}

	T dval = dMin + dPix/T(iDim) * (dMax-dMin);
	if(bLog)
		dval = pow(10., dval);

	return dval;
}
// val -> pixel
template<typename T=double>
T tic_trafo_inv(unsigned int iDim, T dMin, T dMax, bool bLog, T dVal)
{
	if(bLog)
	{
		dMin = safe_log10<T>(dMin);
		dMax = safe_log10<T>(dMax);

		dVal = safe_log10<T>(dVal);
	}

	T dpix = (dVal-dMin)/(dMax-dMin) * double(iDim);
	return dpix;
}

template<typename T1, typename T2>
void convert(T1* pDst, const T2* pSrc, unsigned int iSize)
{
	for(unsigned int i=0; i<iSize; ++i)
		pDst[i] = T1(pSrc[i]);
}

template<class vec_type>
typename vec_type::value_type sum_vec(const vec_type& vec)
{
	typename vec_type::value_type val = 0;
	for(const typename vec_type::value_type& v : vec)
		val += v;
	return val;
}

template<typename T>
void apply_fkt(const T* pIn, T* pOut, T(*fkt)(T), unsigned int iSize)
{
	for(unsigned int i=0; i<iSize; ++i)
		pOut[i] = (*fkt)(pIn[i]);
}

inline unsigned int lerprgb(unsigned char r1, unsigned char g1, unsigned char b1,
							unsigned char r2, unsigned char g2, unsigned char b2,
							double dval)
{
	unsigned char r = lerp(r1, r2, dval);
	unsigned char g = lerp(g1, g2, dval);
	unsigned char b = lerp(b1, b2, dval);

	return (0xff<<24) | (r<<16) | (g<<8) | (b);
}

inline unsigned int lerprgb(unsigned int col1, unsigned int col2, double dval)
{
	unsigned char r1 = (unsigned char)((col1&0x00ff0000) >> 16);
	unsigned char r2 = (unsigned char)((col2&0x00ff0000) >> 16);

	unsigned char g1 = (unsigned char)((col1&0x0000ff00) >> 8);
	unsigned char g2 = (unsigned char)((col2&0x0000ff00) >> 8);

	unsigned char b1 = (unsigned char)(col1&0x000000ff);
	unsigned char b2 = (unsigned char)(col2&0x000000ff);

	unsigned char r = lerp(r1, r2, dval);
	unsigned char g = lerp(g1, g2, dval);
	unsigned char b = lerp(b1, b2, dval);

	return (0xff<<24) | (r<<16) | (g<<8) | (b);
}

/*template<typename T> bool has_nan_or_inf(T d)
{
        // NaN?
        if(d!=d)
                return true;

        // inf?
        if(d==std::numeric_limits<T>::infinity())
                return true;

        return false;
}*/

template<typename T> T* vec_to_array(const std::vector<T>& vec)
{
	T* t_arr = new T[vec.size()];

	unsigned int i=0;
	for(const T& t : vec)
		t_arr[i++] = t;

	return t_arr;
}



template<class T>
struct sort_obj
{
	std::vector<T> vec;
};

template<class T>
bool comp_fkt(sort_obj<T> t0, sort_obj<T> t1)
{ return t0.vec[0] < t1.vec[0]; }


// simultaneously sort two arrays
template<class Iter=double*>
void sort_2(Iter begin1, Iter end1, Iter begin2)
{
	typedef typename std::iterator_traits<Iter>::value_type T;

	const unsigned int N = end1-begin1;
	sort_obj<T> *pObj = new sort_obj<T>[N];
	for(unsigned int i=0; i<N; ++i)
	{
		pObj[i].vec.push_back(*(begin1+i));
		pObj[i].vec.push_back(*(begin2+i));
	}

	std::sort(pObj, pObj+N, comp_fkt<T>);
	for(unsigned int i=0; i<N; ++i)
	{
		*(begin1+i) = pObj[i].vec[0];
		*(begin2+i) = pObj[i].vec[1];
	}

	delete[] pObj;
}

// simultaneously sort three arrays
template<class Iter=double*>
void sort_3(Iter begin1, Iter end1, Iter begin2, Iter begin3)
{
	typedef typename std::iterator_traits<Iter>::value_type T;

	const unsigned int N = end1-begin1;
	sort_obj<T> *pObj = new sort_obj<T>[N];
	for(unsigned int i=0; i<N; ++i)
	{
		pObj[i].vec.push_back(*(begin1+i));
		pObj[i].vec.push_back(*(begin2+i));
		pObj[i].vec.push_back(*(begin3+i));
	}

	std::sort(pObj, pObj+N, comp_fkt<T>);
	for(unsigned int i=0; i<N; ++i)
	{
		*(begin1+i) = pObj[i].vec[0];
		*(begin2+i) = pObj[i].vec[1];
		*(begin3+i) = pObj[i].vec[2];
	}

	delete[] pObj;
}


// sort tuple-vector
template<const int isortidx, class... Ts>
void sorttuples(std::vector<std::tuple<Ts...> >& vec)
{
	std::sort(vec.begin(), vec.end(), 
		[](const std::tuple<Ts...>& tup1, const std::tuple<Ts...>& tup2) -> bool
		{ return std::get<isortidx>(tup1) < std::get<isortidx>(tup2);});
}



template<typename T>
std::list<T> vector_to_list(const std::vector<T>& vec)
{
	std::list<T> lst;
	for(const T& t : vec)
		lst.push_back(t);
	return lst;
}

template<typename T=double>
T max3(T t1, T t2, T t3)
{
	T tmax = t1;
	tmax = std::max(tmax, t2);
	tmax = std::max(tmax, t3);
	return tmax;
}

template<typename T1, typename T2>
void merge_map(std::map<T1, T2>& mapThis, const std::map<T1, T2>& mapOther)
{
	for(const std::pair<T1, T2>& thepair : mapOther)
		mapThis.insert(thepair);
}

#endif
