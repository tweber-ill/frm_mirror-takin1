/*
 * linalg helpers
 *
 * @author: tweber
 * @date: 30-apr-2013
 */

#ifndef __MIEZE_LINALG__
#define __MIEZE_LINALG__

#include <boost/algorithm/minmax_element.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
namespace ublas = boost::numeric::ublas;

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


template<class vector_type, class matrix_type>
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
	ublas::matrix<T> mat(2,2);

	T s = std::sin(angle);
	T c = std::cos(angle);

	mat(0,0) = c; mat(0,1) = -s;
	mat(1,0) = s; mat(1,1) = c;

	return mat;
}


// code for inverse based on /boost/libs/numeric/ublas/test/test_lu.cpp
template<typename T=double>
bool inverse(const ublas::matrix<T>& mat, ublas::matrix<T>& inv)
{
	//if(mat.size1() != mat.size2())
	//	return false;
	const unsigned int N = mat.size2();

	ublas::matrix<T> lu = mat;
	ublas::permutation_matrix<> perm(N);

	if(ublas::lu_factorize(lu, perm) != 0)
		return false;

	inv = ublas::identity_matrix<T>(N);
	ublas::lu_substitute(lu, perm, inv);

	return true;
}

template<typename T=double>
double determinant(const ublas::matrix<T>& mat)
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
		double a[] = {mat(0,0), mat(1,0), mat(2,0)};
		double b[] = {mat(0,1), mat(1,1), mat(2,1)};
		double c[] = {mat(0,2), mat(1,2), mat(2,2)};

		return a[0]*b[1]*c[2] + a[1]*b[2]*c[0] + a[2]*b[0]*c[1]
		           -c[0]*b[1]*a[2] - c[1]*b[2]*a[0] - c[2]*b[0]*a[1];
	}

	const unsigned int i = 0;
	T val = T(0);
	for(unsigned int j=0; j<mat.size2(); ++j)
		val += pow(T(-1), i+j) * mat(i,j) * determinant(submatrix(mat, i, j));

	return val;
}


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

	bool intersect(const Line& line, T& t)
	{
		const ublas::vector<T>& pos0 =  this->GetX0();
		const ublas::vector<T>& pos1 =  line.GetX0();

		const ublas::vector<T>& dir0 =  this->GetDir();
		const ublas::vector<T>& dir1 =  line.GetDir();

		const unsigned int N = pos0.size();

		// pos0 + t0*dir0 = pos1 + t1*dir1
		// pos0 - pos1 = t1*dir1 - t0*dir0

		const ublas::vector<T>& pos = pos0-pos1;
		ublas::matrix<T> mat(N,2);

		for(unsigned int i=0; i<N; ++i)
		{
			mat(i, 0) = dir0[i];
			mat(i, 1) = dir1[i];
		}

		ublas::matrix<T> inv;
		if(!::inverse(mat, inv))
			return false;

		ublas::vector<T> params = ublas::prod(inv, pos);
		t = -params[0];

		return true;
	}
};

#endif
