/*
 * math helpers
 *
 * @author: tweber
 * @date: 23-apr-2013
 */

#ifndef __MIEZE_MATH__
#define __MIEZE_MATH__

#include <cmath>
#include <vector>
#include <limits>

#include "linalg.h"


template<typename T=double>
void diff(unsigned int N, const T* pXIn, const T* pYIn, T* pYOut)
{
	for(unsigned int i=0; i<N-1; ++i)
		pYOut[i] = (pYIn[i+1]-pYIn[i]) / (pXIn[i+1]-pXIn[i]);

	// copy last value
	pYOut[N-1] = pYOut[N-2];
}

template<typename T=double>
std::vector<unsigned int> find_zeroes(unsigned int N, const T* pIn)
{
	/*
	double dMin = 0.;
	double dMax = 0.;
	std::pair<const double*, const double*> minmax = boost::minmax_element(pIn, pIn+N);
	if(minmax.first != pIn+N) dMin = *minmax.first;
	if(minmax.second != pIn+N) dMax = *minmax.second;
	*/

	//const double dThres = std::numeric_limits<double>::epsilon();

	std::vector<unsigned int> vecIndices;

	for(unsigned int i; i<N-1; ++i)
	{
		ublas::vector<T> zero(2);
		zero[0] = zero[1] = 0.;
		ublas::vector<T> xdir(2);
		xdir[0] = 1.; xdir[1] = 0.;
		Line<T> xaxis(zero, xdir);

		ublas::vector<T> pos0(2);
		pos0[0] = 0.; pos0[1] = pIn[i];
		ublas::vector<T> pos1(2);
		pos1[0] = 1.; pos1[1] = pIn[i+1];
		Line<T> line(pos0, pos1-pos0);

		T param;
		if(!line.intersect(xaxis, param))
			continue;

		ublas::vector<T> posInters = line(param);
		if(posInters[0]>=0. && posInters[0]<=1.)
			vecIndices.push_back(i);
	}

	return vecIndices;
}


template<typename T=double>
bool float_equal(T t1, T t2)
{
	return std::fabs(t1-t2) < std::numeric_limits<double>::epsilon();
}

#endif
