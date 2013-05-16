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
namespace ublas = boost::numeric::ublas;

extern "C"
{
        #include <lapacke.h>
}

//#include "math.h"
template<typename T=double>
bool float_equal(T t1, T t2);


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

// code for inverse based on /boost/libs/numeric/ublas/test/test_lu.cpp
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

template<typename T=double>
bool eigenvec(const ublas::matrix<T>& mat, std::vector<ublas::vector<T> >& evecs, std::vector<T>& evals)
{
	if(mat.size1() != mat.size2())
		return false;
	if(mat.size1()==0 || mat.size1()==1)
		return false;

	const unsigned int iOrder = mat.size1();
	evecs.resize(iOrder);
	evals.resize(iOrder);
    for(unsigned int i=0; i<iOrder; ++i)
    		evecs[i].resize(iOrder);


	// is matrix already diagonal?
	if(is_diag_matrix(mat))
	{
		for(unsigned int i=0; i<iOrder; ++i)
		{
			evals[i] = mat(i,i);

			evecs[i] = ublas::zero_vector<T>(iOrder);
			evecs[i][i] = 1.;
		}

		return true;
	}


    bool bOk = true;

    double *pMem = new double[iOrder*iOrder + iOrder*iOrder + iOrder*iOrder + iOrder + iOrder];

    double *pMatrix = pMem;
    for(unsigned int i=0; i<iOrder; ++i)
            for(unsigned int j=0; j<iOrder; ++j)
                    pMatrix[i*iOrder + j] = mat(i,j);

    double *p_eigenvecs = pMatrix + iOrder*iOrder;
    double *p_eigenvecs_l = pMem + iOrder*iOrder;
    double *peigenvals_real = p_eigenvecs_l + iOrder;
    double *peigenvals_imag = peigenvals_real + iOrder;

    int iInfo = LAPACKE_dgeev(LAPACK_ROW_MAJOR, 'N', 'V', iOrder, pMatrix, iOrder,
                                                    peigenvals_real, peigenvals_imag,
                                                    p_eigenvecs_l, iOrder, p_eigenvecs, iOrder);
    if(iInfo!=0)
    {
            std::cerr << "Error: Could not solve eigenproblem (lapack error " << iInfo << ")."
            			<< std::endl;
            bOk = false;
    }

    for(unsigned int i=0; i<iOrder; ++i)
    {
            for(unsigned int j=0; j<iOrder; ++j)
                    evecs[i][j] = p_eigenvecs[j*iOrder + i];
            evals[i] = peigenvals_real[i];
    }

    delete[] pMem;
    return bOk;
}

template<typename T=double>
bool eigenvec_sym(const ublas::matrix<T>& mat, std::vector<ublas::vector<T> >& evecs, std::vector<T>& evals)
{
	if(mat.size1() != mat.size2())
		return false;
	if(mat.size1()==0 || mat.size1()==1)
		return false;

	const unsigned int iOrder = mat.size1();
	evecs.resize(iOrder);
	evals.resize(iOrder);
    for(unsigned int i=0; i<iOrder; ++i)
    		evecs[i].resize(iOrder);


    bool bOk = true;
    double *pMem = new double[iOrder*iOrder + iOrder];
    double *pMatrix = pMem;

    for(unsigned int i=0; i<iOrder; ++i)
            for(unsigned int j=0; j<iOrder; ++j)
                    pMatrix[i*iOrder + j] = mat(i,j);

    double *peigenvals_real = pMatrix + iOrder*iOrder;
    int iInfo = LAPACKE_dsyev(LAPACK_ROW_MAJOR, 'V', 'U',
    											iOrder, pMatrix, iOrder, peigenvals_real);
    if(iInfo!=0)
    {
            std::cerr << "Error: Could not solve eigenproblem (lapack error " << iInfo << ")."
            			<< std::endl;
            bOk = false;
    }

    for(unsigned int i=0; i<iOrder; ++i)
    {
            for(unsigned int j=0; j<iOrder; ++j)
                    evecs[i][j] = pMatrix[j*iOrder + i];
            evals[i] = peigenvals_real[i];
    }

    delete[] pMem;
    return bOk;
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
