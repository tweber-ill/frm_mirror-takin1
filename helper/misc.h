/*
 * misc helper
 * @author tweber
 * @date 07-mar-2013
 */

#ifndef __MIEZE_MISC_HELPER__
#define __MIEZE_MISC_HELPER__

#include <vector>
#include <list>
#include <algorithm>
#include "../data/data.h"

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
inline double tic_trafo(unsigned int iDim, double dMin, double dMax, bool bLog, double dPix)
{
        if(bLog)
        {
                dMin = safe_log10(dMin);
                dMax = safe_log10(dMax);
        }

        double dval = dMin + dPix/double(iDim) * (dMax-dMin);
        if(bLog)
                dval = pow(10., dval);

        return dval;
}

// val -> pixel
inline double tic_trafo_inv(unsigned int iDim, double dMin, double dMax, bool bLog, double dVal)
{
        if(bLog)
        {
                dMin = safe_log10(dMin);
                dMax = safe_log10(dMax);

                dVal = safe_log10(dVal);
        }

        double dpix = (dVal-dMin)/(dMax-dMin) * double(iDim);
        return dpix;
}

template<typename T>
T lerp(T a, T b, double val)
{
	return T(a) + T(double(b-a)*val);
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

inline uint lerprgb(uchar r1, uchar g1, uchar b1,
							uchar r2, uchar g2, uchar b2,
							double dval)
{
	uchar r = lerp(r1, r2, dval);
	uchar g = lerp(g1, g2, dval);
	uchar b = lerp(b1, b2, dval);

	return (0xff<<24) | (r<<16) | (g<<8) | (b);
}

inline uint lerprgb(uint col1, uint col2, double dval)
{
	uchar r1 = uchar((col1&0x00ff0000) >> 16);
	uchar r2 = uchar((col2&0x00ff0000) >> 16);

	uchar g1 = uchar((col1&0x0000ff00) >> 8);
	uchar g2 = uchar((col2&0x0000ff00) >> 8);

	uchar b1 = uchar(col1&0x000000ff);
	uchar b2 = uchar(col2&0x000000ff);

	uchar r = lerp(r1, r2, dval);
	uchar g = lerp(g1, g2, dval);
	uchar b = lerp(b1, b2, dval);

	return (0xff<<24) | (r<<16) | (g<<8) | (b);
}

template<typename T> bool has_nan_or_inf(T d)
{
        // NaN?
        if(d!=d)
                return true;

        // inf?
        if(d==std::numeric_limits<T>::infinity())
                return true;

        return false;
}

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
template<class Iter=double*, typename T=double> void sort_2(Iter begin1, Iter end1, Iter begin2)
{
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
template<class Iter=double*, typename T=double> void sort_3(Iter begin1, Iter end1, Iter begin2, Iter begin3)
{
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
