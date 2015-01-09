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
#include "linalg2.h"
#include "log.h"

#include <iostream>
#include <cmath>
#include <boost/algorithm/minmax_element.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/exception.hpp>
namespace ublas = boost::numeric::ublas;
namespace math = boost::math;


//------------------------------------------------------------------------------

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
		ublas::matrix<T> M = row_matrix({plane1.GetNorm(), plane2.GetNorm()});

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


//------------------------------------------------------------------------------


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
			log_err("\"Side of line\" only defined for 2d vectors.");
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
	bool intersect(const Plane<T>& plane, T& t) const
	{
		const unsigned int N = m_vecDir.size();
		if(N != 3)
		{
			log_err("Line-plane intersection only implemented for 3d vectors.");
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
			//log_warn("Could not invert matrix ", mat, ".");
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
			log_err("Perpendicular line only implemented for 2d vectors.");
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


//------------------------------------------------------------------------------

template<class T=double>
class Quadric
{
protected:
	// general: x^T Q x  +  r x  +  s  =  0
	// here: x^T Q x + s  =  0
	ublas::matrix<T> m_Q = ublas::zero_matrix<T>(3,3);
	//ublas::vector<T> m_r = ublas::zero_vector<T>(3);
	T m_s = 0;

	ublas::vector<T> m_vecOffs = ublas::zero_vector<T>(3);

public:
	Quadric(unsigned int iDim)
		: m_Q(ublas::zero_matrix<T>(iDim,iDim))/*, m_r(ublas::zero_vector<T>(iDim))*/
	{}
	Quadric(const ublas::matrix<T>& Q) : m_Q(Q) {}
	Quadric(const ublas::matrix<T>& Q, /*const ublas::vector<T>& r,*/ T s)
			: m_Q(Q), /*m_r(r),*/ m_s(s) {}
	virtual ~Quadric() {}

	const Quadric<T>& operator=(const Quadric<T>& quad)
	{
		this->m_Q = quad.m_Q;
		//this->m_r = quad.m_r;
		this->m_s = quad.m_s;
		this->m_vecOffs = quad.m_vecOffs;

		return *this;
	}

	Quadric<T>& operator=(Quadric<T>&& quad)
	{
		this->m_Q = std::move(quad.m_Q);
		//this->m_r = std::move(quad.m_r);
		this->m_s = std::move(quad.m_s);
		this->m_vecOffs = std::move(quad.m_vecOffs);

		return *this;
	}

	Quadric(const Quadric<T>& quad) { *this = quad; }
	Quadric(Quadric<T>&& quad) { *this = quad; }

	void SetOffset(const ublas::vector<T>& vec) { m_vecOffs = vec; }
	const ublas::vector<T>& GetOffset() const { return m_vecOffs; }

	const ublas::matrix<T>& GetQ() const { return m_Q; }
	//const ublas::vector<T>& GetR() const { return m_r; }
	T GetS() const { return m_s; }

	void SetQ(const ublas::matrix<T>& Q) { m_Q = Q; }
	//void SetR(const ublas::vector<T>& r) { m_r = r; }
	void SetS(T s) { m_s = s; }

	T operator()(const ublas::vector<T>& _x) const
	{
		ublas::vector<T> x = x-m_vecOffs;

		ublas::vector<T> vecQ = ublas::prod(m_Q, x);
		T dQ = ublas::inner_prod(x, vecQ);
		//T dR = ublas::inner_prod(m_r, x);

		return dQ /*+ dR*/ + m_s;
	}

	// remove column and row iIdx
	void RemoveElems(unsigned int iIdx)
	{
		m_Q = remove_elems(m_Q, iIdx);
		//m_r = remove_elem(m_r, iIdx);
		m_vecOffs = remove_elem(m_vecOffs, iIdx);
	}

	void transform(const ublas::matrix<T>& S)
	{
		ublas::matrix<T> TS = ublas::trans(S);
		ublas::matrix<T> QS = ublas::prod(m_Q, S);
		m_Q = ublas::prod(TS, QS);
	}

	// Q = O D O^T
	// O: eigenvecs, D: eigenvals
	bool GetPrincipalAxes(ublas::matrix<T>& matEvecs, std::vector<T>& vecEvals) const
	{
		std::vector<ublas::vector<T> > evecs;
		if(!eigenvec_sym(m_Q, evecs, vecEvals))
		{
			log_err("Cannot determine eigenvectors.");
			return false;
		}

		sort_eigenvecs<double>(evecs, vecEvals, 1,
			[](double d) -> double { return 1./std::sqrt(d); });

		matEvecs = column_matrix(evecs);
		return true;
	}

	// quad: x^T Q x + s = 0; line: x = x0 + t d
	// (x0 + t d)^T Q (x0 + t d) + s = 0
	// (x0 + t d)^T Q x0 + (x0 + t d)^T Q t d + s = 0
	// (x0^T + t d^T) Q x0 + (x0^T + t d^T) Q t d + s = 0
	// x0^T Q x0 + s  +  (d^T Q x0 + x0^T Q d) t  +  d^T Q d t^2 = 0
	std::vector<T> intersect(const Line<T>& line) const
	{
		const ublas::matrix<T>& Q = GetQ();
		const T& s = m_s;
		const ublas::vector<T>& d = line.GetDir();
		const ublas::vector<T> x0 = line.GetX0() - m_vecOffs;;

		// solving at^2 + bt + c = 0 for t
		ublas::vector<T> vecQd = ublas::prod(Q, d);
		T a = ublas::inner_prod(d, vecQd);

		ublas::vector<T> vecQx0 = ublas::prod(Q, x0);
		T c = ublas::inner_prod(x0, vecQx0) + s;

		T b = ublas::inner_prod(x0, vecQd);
		b += ublas::inner_prod(d, vecQx0);

		//std::cout << "a=" << a << ", b=" << b << ", c=" << c << std::endl;
		return quadratic_solve(a,b,c);
	}
};

template<class T=double>
std::ostream& operator<<(std::ostream& ostr, const Quadric<T>& quad)
{
	ostr << "Q = " << quad.GetQ() << ", ";
	//ostr << "r = " << quad.GetR() << ", ";
	ostr << "s = " << quad.GetS();
	return ostr;
}


template<class T=double>
class QuadSphere : public Quadric<T>
{
protected:

public:
	QuadSphere(unsigned int iDim) : Quadric<T>(iDim) {}

	QuadSphere(T r) : Quadric<T>(3)
	{
		this->m_Q(0,0) =
		this->m_Q(1,1) =
		this->m_Q(2,2) = 1./(r*r);

		this->m_s = -1.;
	}

	QuadSphere(unsigned int iDim, T r) : Quadric<T>(iDim)
	{
		for(unsigned int i=0; i<iDim; ++i)
			this->m_Q(i,i) = 1./(r*r);

		this->m_s = -1.;
	}

	// only valid in principal axis system
	T GetRadius() const { return 1./std::sqrt(this->m_Q(0,0)); }
	T GetVolume() const { return get_ellipsoid_volume(this->m_Q); }

	virtual ~QuadSphere() {}
};


/*
 * this is a 1:1 C++ reimplementation of 'rc_int' from 'mcresplot' and 'rescal5'
 * integrate over row/column iIdx
 */
template<class T = double>
ublas::matrix<T> ellipsoid_gauss_int(const ublas::matrix<T>& mat, unsigned int iIdx)
{
	ublas::vector<T> b(mat.size1());
	for(unsigned int i=0; i<mat.size1(); ++i)
		b[i] = 2.*mat(i,iIdx);
	b = remove_elem(b, iIdx);
	ublas::matrix<T> bb = outer_prod(b,b)/4.;

	ublas::matrix<T> m = remove_elems(mat, iIdx);
	m -= bb/mat(iIdx, iIdx);
	return m;
}

template<class T=double>
class QuadEllipsoid : public Quadric<T>
{
protected:

public:
	QuadEllipsoid(unsigned int iDim) : Quadric<T>(iDim) {}

	QuadEllipsoid(T a, T b) : Quadric<T>(2)
	{
		this->m_Q(0,0) = 1./(a*a);
		this->m_Q(1,1) = 1./(b*b);

		this->m_s = -1.;
	}

	QuadEllipsoid(T a, T b, T c) : Quadric<T>(3)
	{
		this->m_Q(0,0) = 1./(a*a);
		this->m_Q(1,1) = 1./(b*b);
		this->m_Q(2,2) = 1./(c*c);

		this->m_s = -1.;
	}

	QuadEllipsoid(T a, T b, T c, T d) : Quadric<T>(4)
	{
		this->m_Q(0,0) = 1./(a*a);
		this->m_Q(1,1) = 1./(b*b);
		this->m_Q(2,2) = 1./(c*c);
		this->m_Q(3,3) = 1./(d*d);

		this->m_s = -1.;
	}

	virtual ~QuadEllipsoid() {}

	// only valid in principal axis system
	T GetRadius(unsigned int i) const { return 1./std::sqrt(this->m_Q(i,i)); }
	T GetVolume() const { return get_ellipsoid_volume(this->m_Q); }

	void GaussInt(unsigned int iIdx)
	{
		ublas::matrix<T> m_Qint = ellipsoid_gauss_int(this->m_Q, iIdx);
		this->RemoveElems(iIdx);
		this->m_Q = m_Qint;
	}
};


//------------------------------------------------------------------------------


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

	for(unsigned int i=0; i<N-1; ++i)
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
        {
            //std::cerr << "No intersection." << std::endl;
			continue;
        }
        //std::cout << "Intersection param: " << param << std::endl;

		ublas::vector<T> posInters = line(param);
		if(posInters[0]>=0. && posInters[0]<=1.)
			vecIndices.push_back(i);
	}

	return vecIndices;
}

#endif
