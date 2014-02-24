/*
 * linalg helpers
 *
 * @author: tweber
 * @date: 30-apr-2013
 */

#ifndef __MIEZE_LINALG__
#define __MIEZE_LINALG__

#include <cmath>
#include <boost/algorithm/minmax_element.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/exception.hpp>
#include <boost/math/quaternion.hpp>
namespace ublas = boost::numeric::ublas;
namespace math = boost::math;


//#include "math.h"
template<typename T=double> bool float_equal(T t1, T t2);
template<typename T> T sign(T t);


template<class vec_type>
typename vec_type::value_type vec_len(const vec_type& vec)
{
	typename vec_type::value_type t = typename vec_type::value_type();

	for(unsigned int i=0; i<vec.size(); ++i)
		t += vec[i]*vec[i];

	t = std::sqrt(t);
	return t;
}

template<typename vec_type>
typename vec_type::value_type vec_angle_2(const vec_type& vec)
{
	return std::atan2(vec[1], vec[0]);
}

/*
 * remove an element from a vector
 */
template<class vector_type>
vector_type remove_elem(const vector_type& vec, unsigned int iIdx)
{
        vector_type vecret(vec.size()-1);

        for(unsigned int i=0, j=0; i<vec.size() && j<vecret.size();)
        {
                vecret[j] = vec[i];

                if(i!=iIdx) ++j;
                ++i;
        }

        return vecret;
}

template<class matrix_type>
matrix_type submatrix(const matrix_type& mat, unsigned int iRow, unsigned int iCol)
{
        matrix_type matret(mat.size1()-1, mat.size2()-1);

        for(unsigned int i=0, i0=0; i<mat.size1() && i0<matret.size1();)
        {
                for(unsigned int j=0, j0=0; j<mat.size2() && j0<matret.size2();)
                {
                        matret(i0,j0) = mat(i,j);

                        if(j!=iCol) ++j0;
                        ++j;
                }

                if(i!=iRow) ++i0;
                ++i;
        }

        return matret;
}

template<class matrix_type>
void submatrix_copy(matrix_type& mat, const matrix_type& sub, unsigned int iRowBegin, unsigned int iColBegin)
{
	for(unsigned int i=0; i<sub.size1(); ++i)
		for(unsigned int j=0; j<sub.size2(); ++j)
			mat(iRowBegin+i, iColBegin+j) = sub(i,j);
}

template<class matrix_type>
matrix_type remove_elems(const matrix_type& mat, unsigned int iIdx)
{
	return submatrix(mat, iIdx, iIdx);
}


template<class vector_type=ublas::vector<double>, class matrix_type=ublas::matrix<double>>
vector_type get_column(const matrix_type& mat, unsigned int iCol)
{
        vector_type vecret(mat.size1());

        for(unsigned int i=0; i<mat.size1(); ++i)
        	vecret[i] = mat(i, iCol);

        return vecret;
}


template<typename T=double>
ublas::matrix<T> rotation_matrix_2d(T angle)
{
	if(angle==0.)
		return ublas::identity_matrix<T>(2);

	ublas::matrix<T> mat(2,2);

	T s = std::sin(angle);
	T c = std::cos(angle);

	mat(0,0) = c; mat(0,1) = -s;
	mat(1,0) = s; mat(1,1) = c;

	return mat;
}

template<typename T=double>
ublas::matrix<T> rotation_matrix_3d_x(T angle)
{
	ublas::matrix<T> mat(3,3);

	T s = std::sin(angle);
	T c = std::cos(angle);

	mat(0,0)=1; mat(0,1)=0; mat(0,2)=0;
	mat(1,0)=0; mat(1,1)=c; mat(1,2)=-s;
	mat(2,0)=0; mat(2,1)=s; mat(2,2)=c;

	return mat;
}

template<typename T=double>
ublas::matrix<T> rotation_matrix_3d_y(T angle)
{
	ublas::matrix<T> mat(3,3);

	T s = std::sin(angle);
	T c = std::cos(angle);

	mat(0,0)=c; mat(0,1)=0; mat(0,2)=s;
	mat(1,0)=0; mat(1,1)=1; mat(1,2)=0;
	mat(2,0)=-s; mat(2,1)=0; mat(2,2)=c;

	return mat;
}

template<typename T=double>
ublas::matrix<T> rotation_matrix_3d_z(T angle)
{
	ublas::matrix<T> mat(3,3);

	T s = std::sin(angle);
	T c = std::cos(angle);

	mat(0,0)=c; mat(0,1)=-s; mat(0,2)=0;
	mat(1,0)=s; mat(1,1)=c; mat(1,2)=0;
	mat(2,0)=0; mat(2,1)=0; mat(2,2)=1;

	return mat;
}

template<typename T=double>
ublas::matrix<T> skew(const ublas::vector<T>& vec)
{
	ublas::matrix<T> mat = ublas::zero_matrix<T>(vec.size(), vec.size());

	if(vec.size() == 3)
	{
		mat(0,1) = -vec[2];
		mat(0,2) = vec[1];
		mat(1,2) = -vec[0];

		mat(1,0) = -mat(0,1);
		mat(2,0) = -mat(0,2);
		mat(2,1) = -mat(1,2);
	}
	else
		throw "Skew only defined for three dimensions.";

	return mat;
}

template<typename T=double>
ublas::matrix<T> unit_matrix(unsigned int N)
{
	ublas::matrix<T> mat = ublas::zero_matrix<T>(N,N);
	for(unsigned int i=0; i<N; ++i)
		mat(i,i) = 1;
	return mat;
}


// Euler-Rodrigues formula
template<typename T=double>
ublas::matrix<T> rotation_matrix(const ublas::vector<T>& vec, T angle)
{
	return (T(1) - std::cos(angle)) * ublas::outer_prod(vec,vec) +
				std::cos(angle) * unit_matrix(vec.size()) +
				std::sin(angle) * skew(vec);
}


template<typename T=double>
T trace(const ublas::matrix<T>& mat)
{
	if(mat.size1() != mat.size2())
		return T(0);

	T tr = T(0.);
	for(unsigned int i=0; i<mat.size1(); ++i)
		tr += mat(i,i);
	return tr;
}




template<typename T=double>
bool isnan(const ublas::matrix<T>& mat)
{
	for(unsigned int i=0; i<mat.size1(); ++i)
		for(unsigned int j=0; j<mat.size2(); ++j)
			if(std::isnan(mat(i,j)))
				return true;
	return false;
}

template<typename T=double>
bool isinf(const ublas::matrix<T>& mat)
{
	for(unsigned int i=0; i<mat.size1(); ++i)
		for(unsigned int j=0; j<mat.size2(); ++j)
			if(std::isinf(mat(i,j)))
				return true;
	return false;
}

// code for inverse based on boost/libs/numeric/ublas/test/test_lu.cpp
template<typename T=double>
bool inverse(const ublas::matrix<T>& mat, ublas::matrix<T>& inv)
{
	if(mat.size1() != mat.size2())
		return false;
	//if(isnan(mat) || isinf(mat))
	//	return false;

	try
	{
		const unsigned int N = mat.size2();

		ublas::matrix<T> lu = mat;
		ublas::permutation_matrix<> perm(N);

		if(ublas::lu_factorize(lu, perm) != 0)
			return false;

		inv = ublas::identity_matrix<T>(N);
		ublas::lu_substitute(lu, perm, inv);
	}
	catch(ublas::internal_logic& ex)
	{
		std::cerr << "Error: Matrix inversion failed with exception: " << ex.what() << "." << "\n";
		std::cerr << "Matrix to be inverted was: " << mat << "." << std::endl;
		//std::cerr << "with determinant " << determinant(mat) << "." << std::endl;
		return false;
	}

	return true;
}

template<typename T=double>
bool is_diag_matrix(const ublas::matrix<T>& mat)
{
	for(unsigned int i=0; i<mat.size1(); ++i)
		for(unsigned int j=0; j<mat.size2(); ++j)
		{
			if(i==j) continue;

			if(!float_equal(mat(i,j), T(0.)))
				return false;
		}

	return true;
}


// vectors form columns of matrix
template<typename T=double>
ublas::matrix<T> column_matrix(const std::vector<ublas::vector<T> >& vecs)
{
	if(vecs.size() == 0)
		return ublas::zero_matrix<T>(0);

	ublas::matrix<T> mat(vecs.size(), vecs[0].size());
	for(unsigned int i=0; i<vecs[0].size(); ++i)
		for(unsigned int j=0; j<vecs.size(); ++j)
			mat(i,j) = vecs[j][i];

	return mat;
}

template<typename T=double>
bool eigenvec(const ublas::matrix<T>& mat, std::vector<ublas::vector<T> >& evecs, std::vector<T>& evals)
{
	std::cerr << "Error: No specialisation of \"eigenvec\" available for this type." << std::endl;
	return false;
}

template<typename T=double>
bool eigenvec_sym(const ublas::matrix<T>& mat, std::vector<ublas::vector<T> >& evecs, std::vector<T>& evals)
{
	std::cerr << "Error: No specialisation of \"eigenvec_sym\" available for this type." << std::endl;
    return false;
}

template<>
bool eigenvec<double>(const ublas::matrix<double>& mat,
									std::vector<ublas::vector<double> >& evecs,
									std::vector<double>& evals);
template<>
bool eigenvec_sym<double>(const ublas::matrix<double>& mat,
											std::vector<ublas::vector<double> >& evecs,
											std::vector<double>& evals);


// algo from:
// http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q55
template<typename T=double>
math::quaternion<T> rot3_to_quat(const ublas::matrix<T>& rot)
{
	T tr = trace(rot) + 1.;
	T x,y,z,w;

	if(tr > std::numeric_limits<T>::epsilon())
	{
		T s = std::sqrt(tr) * 2.;
		x = (rot(2,1) - rot(1,2)) / s;
		y = (rot(0,2) - rot(2,0)) / s;
		z = (rot(1,0) - rot(0,1)) / s;
		w = s/4.;
	}
	else
	{
		if (rot(0,0) > rot(1,1) && rot(0,0) > rot(2,2))
		{
			T s = std::sqrt(1. + rot(0,0) - rot(1,1) - rot(2,2)) * 2.;
			x = s/4.;
			y = (rot(1,0) + rot(0,1)) / s;
			z = (rot(0,2) + rot(2,0)) / s;
			w = (rot(2,1) - rot(1,2)) / s;
		}
		else if(rot(1,1) > rot(2,2))
		{
			T s = std::sqrt(1. + rot(1,1) - rot(0,0) - rot(2,2)) * 2.;
			x = (rot(1,0) + rot(0,1)) / s;
			y = s/4.;
			z = (rot(2,1) + rot(1,2)) / s;
			w = (rot(0,2) - rot(2,0)) / s;
		}
		else
		{
			T s = std::sqrt(1. + rot(2,2) - rot(0,0) - rot(1,1)) * 2.;
			x = (rot(0,2) + rot(2,0)) / s;
			y = (rot(2,1) + rot(1,2)) / s;
			z = s/4.;
			w = (rot(1,0) - rot(0,1)) / s;
		}
	}

	T n = std::sqrt(w*w + x*x + y*y + z*z);
	return math::quaternion<T>(w,x,y,z)/n;
}

// algo from:
// http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q54
// http://www.cg.info.hiroshima-cu.ac.jp/~miyazaki/knowledge/teche52.html
template<typename T=double>
ublas::matrix<T> quat_to_rot3(const math::quaternion<T>& quat)
{
	ublas::matrix<T> mat(3,3);
	T w = quat.R_component_1();
	T x = quat.R_component_2();
	T y = quat.R_component_3();
	T z = quat.R_component_4();

	mat(0,0) = 1. - 2.*(y*y + z*z);
	mat(1,1) = 1. - 2.*(x*x + z*z);
	mat(2,2) = 1. - 2.*(x*x + y*y);

	mat(0,1) = 2.*(x*y - z*w);
	mat(1,0) = 2.*(x*y + z*w);
	mat(0,2) = 2.*(x*z + y*w);
	mat(2,0) = 2.*(x*z - y*w);
	mat(1,2) = 2.*(y*z - x*w);
	mat(2,1) = 2.*(y*z + x*w);
	//mat = ublas::trans(mat);

	return mat;
}


template<typename T=double>
std::vector<T> quat_to_euler(const math::quaternion<T>& quat)
{
	T q[] = {quat.R_component_1(), quat.R_component_2(), quat.R_component_3(), quat.R_component_4()};

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
		T angle = atan2(rot(1,0), rot(0,0));
		vecResult.push_back(angle);
	}
	else if(rot.size2()==3)
	{
		math::quaternion<T> quat = rot3_to_quat(rot);
		vecResult = quat_to_euler(quat);
	}

	return vecResult;
}


template<typename vector_type = ublas::vector<double> >
vector_type cross_3(const vector_type& vec0, const vector_type& vec1)
{
	vector_type vec;
	vec.resize(3);

	vec[0] = vec0[1]*vec1[2] - vec1[1]*vec0[2];
	vec[1] = vec0[2]*vec1[0] - vec1[2]*vec0[0];
	vec[2] = vec0[0]*vec1[1] - vec1[0]*vec0[1];

	return vec;
}

template<typename T=double>
T determinant(const ublas::matrix<T>& mat)
{
	if(mat.size1() != mat.size2())
		return T(0);

	if(mat.size1()==1)
	{
		return mat(0,0);
	}
	else if(mat.size1()==2)
	{
		return mat(0,0)*mat(1,1) - mat(1,0)*mat(0,1);
	}
	else if(mat.size1()==3)
	{
		ublas::vector<T> vec0 = get_column(mat, 0);
		ublas::vector<T> vec1 = get_column(mat, 1);
		ublas::vector<T> vec2 = get_column(mat, 2);

		ublas::vector<T> vecCross = cross_3<ublas::vector<T> >(vec1, vec2);
		return ublas::inner_prod(vec0, vecCross);
	}

	const unsigned int i = 0;
	T val = T(0);
	for(unsigned int j=0; j<mat.size2(); ++j)
		val += pow(T(-1), i+j) * mat(i,j) * determinant(submatrix(mat, i, j));

	return val;
}

template<typename T=double>
T get_volume(const ublas::matrix<T>& mat)
{
	return determinant<T>(mat);
}



// calculate skew coordinate basis vectors from angles
// see: http://www.ccl.net/cca/documents/molecular-modeling/node4.html
template<class t_vec, class T>
bool skew_basis_from_angles(T a, T b, T c,
							T alpha, T beta, T gamma,
							t_vec& veca, t_vec& vecb, t_vec& vecc)
{
	const T dSG = std::sin(gamma);
	const T dCG = std::cos(gamma);
	const T dCA = std::cos(alpha);
	const T dCB = std::cos(beta);

	const T dCA2 = dCA*dCA;
	const T dCB2 = dCB*dCB;
	const T dCG2 = dCG*dCG;

	const T dVol =a*b*c*std::sqrt(1.- dCA2 - dCB2 - dCG2 + 2.*dCA*dCB*dCG);
	//std::cout << "vol = " <<  dVol << std::endl;

	if(::isinf(dVol) || ::isnan(dVol))
		return false;

	veca[0] = a;
	veca[1] = 0.;
	veca[2] = 0.;

	vecb[0] = b*dCG;
	vecb[1] = b*dSG;
	vecb[2] = 0.;

	vecc[0] = c*dCB;
	vecc[1] = c*(dCA - dCB*dCG) / dSG;
	vecc[2] = dVol / (a*b*dSG);

	return true;
}


template<typename T>
class Plane
{
protected:
	ublas::vector<T> m_vecX0;
	ublas::vector<T> m_vecDir0, m_vecDir1;
	ublas::vector<T> m_vecNorm;

public:
	Plane(const ublas::vector<T>& vec0,
		const ublas::vector<T>& dir0, const ublas::vector<T>& dir1)
		: m_vecX0(vec0), m_vecDir0(dir0), m_vecDir1(dir1)
	{
		m_vecNorm = cross_3(dir0, dir1);
		m_vecNorm /= ublas::norm_2(m_vecNorm);
	}

	virtual ~Plane()
	{}

	const ublas::vector<T>& GetX0() const { return m_vecX0; }
	const ublas::vector<T>& GetDir0() const { return m_vecDir0; }
	const ublas::vector<T>& GetDir1() const { return m_vecDir1; }
	const ublas::vector<T>& GetNorm() const { return m_vecNorm; }

	ublas::vector<T> GetDroppedPerp(const ublas::vector<T>& vecP, double *pdDist=0) const
	{
		T d = ublas::inner_prod(m_vecNorm, m_vecX0);
		T t = d - ublas::inner_prod(m_vecNorm, vecP);
		ublas::vector<T> vecdropped = vecP + t*m_vecNorm;

		if(pdDist)
		{
			ublas::vector<T> vecD = vecP - vecdropped;
			*pdDist = std::sqrt(ublas::inner_prod(vecD, vecD));
		}

		return vecdropped;
	}
};


template<typename T>
class Line
{
protected:
	ublas::vector<T> m_vecX0;
	ublas::vector<T> m_vecDir;

public:
	Line(const ublas::vector<T>& vec0, const ublas::vector<T>& dir)
		: m_vecX0(vec0), m_vecDir(dir)
	{}

	virtual ~Line() {}

	ublas::vector<T> operator()(T t)
	{
		return m_vecX0 + t*m_vecDir;
	}

	const ublas::vector<T>& GetX0() const { return m_vecX0; }
	const ublas::vector<T>& GetDir() const { return m_vecDir; }

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

	bool intersect(const Line<T>& line, T& t)
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
            std::cerr << "Could not invert matrix " << mat << std::endl;
			return false;
        }

		ublas::vector<T> params = ublas::prod(inv, pos);
		t = params[0];

        //std::cout << "t=" << t << ", ";
		return true;
	}
};

#endif
