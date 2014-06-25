/*
 * linalg and geometry helpers
 *
 * @author: tweber
 * @date: 30-apr-2013
 */

#ifndef __TAZ_GEO_H__
#define __TAZ_GEO_H__

#include "flags.h"
#include "exception.h"
#include "linalg.h"

#include <iostream>
#include <cmath>
#include <boost/algorithm/minmax_element.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/exception.hpp>
namespace ublas = boost::numeric::ublas;
namespace math = boost::math;


template<typename T> class Line;

template<typename T> class Plane
{
protected:
	bool m_bValid = 0;
	ublas::vector<T> m_vecX0;
	ublas::vector<T> m_vecDir0, m_vecDir1;
	ublas::vector<T> m_vecNorm;
	T m_d;

public:
	Plane(const ublas::vector<T>& vec0,
		const ublas::vector<T>& dir0, const ublas::vector<T>& dir1)
		: m_vecX0(vec0), m_vecDir0(dir0), m_vecDir1(dir1)
	{
		m_vecNorm = cross_3(dir0, dir1);
		T tLenNorm = ublas::norm_2(m_vecNorm);
		if(float_equal<T>(tLenNorm, 0.) || tLenNorm!=tLenNorm)
		{
			m_bValid = 0;
			return;
		}
		m_vecNorm /= tLenNorm;

		// Hessian form: vecX0*vecNorm - d = 0
		m_d = ublas::inner_prod(m_vecX0, m_vecNorm);
		m_bValid = 1;
	}

	virtual ~Plane()
	{}

	const ublas::vector<T>& GetX0() const { return m_vecX0; }
	const ublas::vector<T>& GetDir0() const { return m_vecDir0; }
	const ublas::vector<T>& GetDir1() const { return m_vecDir1; }
	const ublas::vector<T>& GetNorm() const { return m_vecNorm; }
	const T& GetD() const { return m_d; }

	T GetDist(const ublas::vector<T>& vecPt) const
	{
		return ublas::inner_prod(vecPt, m_vecNorm) - m_d;
	}

	T GetAngle(const Plane<T>& plane) const
	{
		return std::acos(GetNorm(), plane.GetNorm());
	}

	// "Lotfusspunkt"
	ublas::vector<T> GetDroppedPerp(const ublas::vector<T>& vecP, T *pdDist=0) const
	{
		T dist = GetDist(vecP);
		ublas::vector<T> vecdropped = vecP - dist*m_vecNorm;

		if(pdDist)
		{
			ublas::vector<T> vecD = vecP - vecdropped;
			*pdDist = std::sqrt(ublas::inner_prod(vecD, vecD));
		}

		return vecdropped;
	}

	// http://mathworld.wolfram.com/Plane-PlaneIntersection.html
	bool intersect(const Plane<T>& plane2, Line<T>& lineRet) const
	{
		const Plane<T>& plane1 = *this;

		// direction vector
		ublas::vector<T> vecDir = cross_3(plane1.GetNorm(), plane2.GetNorm());

		// find common point in the two planes
		std::vector<ublas::vector<T> > vecNorms = {plane1.GetNorm(), plane2.GetNorm()};
		ublas::matrix<T> M = row_matrix(vecNorms);

		ublas::vector<T> vecD(2);
		vecD[0] = plane1.GetD();
		vecD[1] = plane2.GetD();

		ublas::vector<T> vec0(3);
		if(!::solve_linear(M, vecD, vec0))
			return 0;

		lineRet = Line<T>(vec0, vecDir);
		return 1;
	}

	bool IsValid() const { return m_bValid; }
};


template<typename T> class Line
{
protected:
	ublas::vector<T> m_vecX0;
	ublas::vector<T> m_vecDir;

public:
	Line()
	{}
	Line(const ublas::vector<T>& vec0, const ublas::vector<T>& dir)
		: m_vecX0(vec0), m_vecDir(dir)
	{}

	virtual ~Line() {}

	ublas::vector<T> operator()(T t) const
	{
		return m_vecX0 + t*m_vecDir;
	}

	const ublas::vector<T>& GetX0() const { return m_vecX0; }
	const ublas::vector<T>& GetDir() const { return m_vecDir; }

	T GetDist(const Line<T>& l1) const
	{
		const Line<T>& l0 = *this;

		ublas::vector<T> vecNorm = cross_3(l0.GetDir(), l1.GetDir());

		T tnum = std::fabs(ublas::inner_prod(l1.GetX0()-l0.GetX0(), vecNorm));
		T tdenom = ublas::norm_2(vecNorm);

		return tnum/tdenom;
	}

	T GetDist(const ublas::vector<T>& vecPt) const
	{
		T tnum = ublas::norm_2(cross_3(m_vecDir, vecPt-m_vecX0));
		T tdenom = ublas::norm_2(m_vecDir);

		return tnum / tdenom;
	}


	// "Lotfusspunkt"
	ublas::vector<T> GetDroppedPerp(const ublas::vector<T>& vecP, T *pdDist=0) const
	{
		T t = ublas::inner_prod(vecP-GetX0(), GetDir()) / ublas::inner_prod(GetDir(), GetDir());
		ublas::vector<T> vecdropped = operator()(t);

		if(pdDist)
		{
			ublas::vector<T> vecD = vecP - vecdropped;
			*pdDist = std::sqrt(ublas::inner_prod(vecD, vecD));
		}

		return vecdropped;
	}


	bool GetSide(const ublas::vector<T>& vecP, T *pdDist=0) const
	{
		const unsigned int N = m_vecDir.size();
		if(N != 2)
		{
			std::cerr << "Error: \"Side of line\" only defined for 2d vectors."
					<< std::endl;
			return false;
		}

		ublas::vector<T> vecDropped = GetDroppedPerp(vecP, pdDist);


		ublas::vector<T> vecNorm(2);
		vecNorm[0] = m_vecDir[1];
		vecNorm[1] = -m_vecDir[0];

		T tDot = ublas::inner_prod(vecP-vecDropped, vecNorm);

		//std::cout << "dropped: " << vecDropped << ", dot: " << tDot << std::endl;
		return tDot < T(0);
	}


	// http://mathworld.wolfram.com/Line-PlaneIntersection.html
	bool intersect(const Plane<T>& plane, T& t)
	{
		const unsigned int N = m_vecDir.size();
		if(N != 3)
		{
			std::cerr << "Error: Line-plane intersection only implemented for 3d vectors."
					<< std::endl;
			return false;
		}

		const ublas::vector<T>& posl = this->GetX0();
		const ublas::vector<T>& dirl = this->GetDir();

		const ublas::vector<T>& xp0 = plane.GetX0();
		const ublas::vector<T> xp1 = plane.GetX0() + plane.GetDir0();
		const ublas::vector<T> xp2 = plane.GetX0() + plane.GetDir1();

		ublas::matrix<T> matDenom(N+1,N+1);
		matDenom(0,0) = 1;		matDenom(0,1) = 1;		matDenom(0,2) = 1;		matDenom(0,3) = 0;
		matDenom(1,0) = xp0[0];	matDenom(1,1) = xp1[0];	matDenom(1,2) = xp2[0];	matDenom(1,3) = dirl[0];
		matDenom(2,0) = xp0[1];	matDenom(2,1) = xp1[1];	matDenom(2,2) = xp2[1];	matDenom(2,3) = dirl[1];
		matDenom(3,0) = xp0[2];	matDenom(3,1) = xp1[2];	matDenom(3,2) = xp2[2];	matDenom(3,3) = dirl[2];

		T denom = determinant(matDenom);
		if(::float_equal(denom, 0.))
			return false;

		ublas::matrix<T> matNum(N+1,N+1);
		matNum(0,0) = 1;		matNum(0,1) = 1;		matNum(0,2) = 1;		matNum(0,3) = 1;
		matNum(1,0) = xp0[0];	matNum(1,1) = xp1[0];	matNum(1,2) = xp2[0];	matNum(1,3) = posl[0];
		matNum(2,0) = xp0[1];	matNum(2,1) = xp1[1];	matNum(2,2) = xp2[1];	matNum(2,3) = posl[1];
		matNum(3,0) = xp0[2];	matNum(3,1) = xp1[2];	matNum(3,2) = xp2[2];	matNum(3,3) = posl[2];

		T num = determinant(matNum);

		t = -num / denom;
		return true;
	}

	bool intersect(const Line<T>& line, T& t) const
	{
		const ublas::vector<T>& pos0 =  this->GetX0();
		const ublas::vector<T>& pos1 =  line.GetX0();

		const ublas::vector<T>& dir0 =  this->GetDir();
		const ublas::vector<T>& dir1 =  line.GetDir();

		const unsigned int N = pos0.size();

		// pos0 + t0*dir0 = pos1 + t1*dir1
		// pos0 - pos1 = t1*dir1 - t0*dir0

		const ublas::vector<T> pos = pos0-pos1;
		ublas::matrix<T> mat = ublas::identity_matrix<T>(N);

		for(unsigned int i=0; i<N; ++i)
		{
			mat(i, 0) = -dir0[i];
			mat(i, 1) = dir1[i];
		}

		ublas::matrix<T> inv;
		if(!::inverse(mat, inv))
		{
			//std::cerr << "Could not invert matrix " << mat << std::endl;
			return false;
		}

		ublas::vector<T> params = ublas::prod(inv, pos);
		t = params[0];

		//std::cout << "t=" << t << ", ";
		return true;
	}

	bool GetMiddlePerp(Line<T>& linePerp) const
	{
		const unsigned int N = m_vecDir.size();
		if(N != 2)
		{
			std::cerr << "Error: Perpendicular line only implemented for 2d vectors."
					<< std::endl;
			return false;
		}

		ublas::vector<T> vecDir(2);
		vecDir[0] = -m_vecDir[1];
		vecDir[1] = m_vecDir[0];

		ublas::vector<T> vecPos = this->operator()(0.5);

		linePerp = Line<T>(vecPos, vecDir);
		return true;
	}
};

template<typename T>
std::ostream& operator<<(std::ostream& ostr, const Plane<T>& plane)
{
	ostr << plane.GetX0() << " + s*" << plane.GetDir0()
				<< " + t*" << plane.GetDir1();
	return ostr;
}

template<typename T>
std::ostream& operator<<(std::ostream& ostr, const Line<T>& line)
{
	ostr << line.GetX0() << " + t*" << line.GetDir();
	return ostr;
}

#endif
