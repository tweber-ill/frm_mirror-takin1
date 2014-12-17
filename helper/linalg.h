/*
 * basic linalg helpers
 *
 * @author: tweber
 * @date: 30-apr-2013
 */

#ifndef __TAZ_LINALG_H__
#define __TAZ_LINALG_H__

#include "flags.h"
#include "exception.h"
#include "math.h"
#include "log.h"
#include "traits.h"

#include <initializer_list>
#include <cmath>

#include <boost/algorithm/minmax_element.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/exception.hpp>
namespace ublas = boost::numeric::ublas;

//#include <boost/math/quaternion.hpp>
//namespace math = boost::math;


template<class matrix_type=ublas::matrix<double> >
typename matrix_type::value_type determinant(const matrix_type& mat);


// create a vector
template<class V=ublas::vector<double> >
V make_vec(const std::initializer_list<typename V::value_type>& lst)
{
	typedef typename V::value_type T;
	typedef typename std::initializer_list<T>::const_iterator t_iter;

	V vec(lst.size());

	std::size_t i=0;
	for(t_iter iter = lst.begin(); iter!=lst.end(); ++i, ++iter)
		vec[i] = *iter;

	return vec;
}


// create a matrix
template<class M=ublas::matrix<double> >
M make_mat(const std::initializer_list<std::initializer_list<typename M::value_type> >& lst)
{
	typedef typename M::value_type T;

	std::size_t I = lst.size();
	std::size_t J = lst.begin()->size();

	M mat(I, J);
	typename std::initializer_list<std::initializer_list<T> >::const_iterator iter = lst.begin();

	for(std::size_t i=0; i<I; ++i, ++iter)
	{
		typename std::initializer_list<T>::const_iterator iterinner = iter->begin();
		for(std::size_t j=0; j<J; ++j, ++iterinner)
		{
			mat(i,j) = *iterinner;
		}
	}

	return mat;
}


template<class vec_type>
bool vec_equal(const vec_type& vec0, const vec_type& vec1,
		typename vec_type::value_type eps = std::numeric_limits<typename vec_type::value_type>::epsilon())
{
	typedef typename vec_type::value_type T;

	if(vec0.size() != vec1.size())
		return false;

	for(unsigned int i=0; i<vec0.size(); ++i)
		if(!float_equal<T>(vec0[i], vec1[i], eps))
			return false;

	return true;
}


template<class vec_type>
typename vec_type::value_type vec_len(const vec_type& vec)
{
	typename vec_type::value_type t = typename vec_type::value_type();

	for(unsigned int i=0; i<vec.size(); ++i)
		t += vec[i]*vec[i];

	t = std::sqrt(t);
	return t;
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
matrix_type remove_column(const matrix_type& mat, unsigned int iCol)
{
	matrix_type matret(mat.size1(), mat.size2()-1);
	for(unsigned int i=0; i<mat.size1(); ++i)
	{
		for(unsigned int j=0, j0=0; j<mat.size2() && j0<matret.size2(); ++j)
		{
			matret(i,j0) = mat(i,j);
                        if(j!=iCol) ++j0;
		}
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


template<class vector_type=ublas::vector<double>, class matrix_type=ublas::matrix<double> >
vector_type get_column(const matrix_type& mat, unsigned int iCol)
{
        vector_type vecret(mat.size1());

        for(unsigned int i=0; i<mat.size1(); ++i)
        	vecret[i] = mat(i, iCol);

        return vecret;
}

template<class vector_type=ublas::vector<double>, class matrix_type=ublas::matrix<double> >
vector_type get_row(const matrix_type& mat, unsigned int iRow)
{
        vector_type vecret(mat.size2());

        for(unsigned int i=0; i<mat.size2(); ++i)
        	vecret[i] = mat(iRow, i);

        return vecret;
}


template<class matrix_type=ublas::matrix<double> >
matrix_type rotation_matrix_2d(typename matrix_type::value_type angle)
{
	typedef typename matrix_type::value_type T;

	T s, c;

	if(angle==0.)
	{
		s = T(0);
		c = T(1);
	}
	else
	{
		s = std::sin(angle);
		c = std::cos(angle);
	}

	return make_mat<matrix_type>
				({	{c, -s},
					{s,  c}});
}

template<class matrix_type=ublas::matrix<double> >
matrix_type rotation_matrix_3d_x(typename matrix_type::value_type angle)
{
	typedef typename matrix_type::value_type T;

	T s, c;
	if(angle==0.)
	{
		s = T(0);
		c = T(1);
	}
	else
	{
		s = std::sin(angle);
		c = std::cos(angle);
	}

	return make_mat<matrix_type>
				({	{1, 0,  0},
					{0, c, -s},
					{0, s,  c}});
}

template<class matrix_type=ublas::matrix<double> >
matrix_type rotation_matrix_3d_y(typename matrix_type::value_type angle)
{
	typedef typename matrix_type::value_type T;

	T s, c;
	if(angle==0.)
	{
		s = T(0);
		c = T(1);
	}
	else
	{
		s = std::sin(angle);
		c = std::cos(angle);
	}

	return make_mat<matrix_type>
				({	{c,  0, s},
					{0,  1, 0},
					{-s, 0, c}});
}

template<class matrix_type=ublas::matrix<double> >
matrix_type rotation_matrix_3d_z(typename matrix_type::value_type angle)
{
	typedef typename matrix_type::value_type T;

	T s, c;
	if(angle==0.)
	{
		s = T(0);
		c = T(1);
	}
	else
	{
		s = std::sin(angle);
		c = std::cos(angle);
	}

	return make_mat<matrix_type>
				({	{c, -s, 0},
					{s,  c, 0},
					{0,  0, 1}});
}

template<class matrix_type = ublas::matrix<double>,
	class vector_type = ublas::vector<double>>
matrix_type skew(const vector_type& vec)
{
	if(vec.size() == 3)
	{
		return make_mat<matrix_type>
				({	{       0, -vec[2],  vec[1]},
					{  vec[2],       0, -vec[0]},
					{ -vec[1],  vec[0],       0}});
	}
	else
		throw Err("Skew only defined for three dimensions.");
}

template<class matrix_type = ublas::matrix<double>>
matrix_type unit_matrix(unsigned int N)
{
	matrix_type mat(N,N);

	for(std::size_t i=0; i<N; ++i)
		for(std::size_t j=0; j<N; ++j)
			mat(i,j) = (i==j ? 1 : 0);
	return mat;
}


// Euler-Rodrigues formula
template<class mat_type=ublas::matrix<double>,
	class vec_type=ublas::vector<double>,
	typename T = typename mat_type::value_type>
mat_type rotation_matrix(const vec_type& vec, T angle)
{
	T s, c;
	if(angle==0.)
	{
		s = T(0);
		c = T(1);
	}
	else
	{
		s = std::sin(angle);
		c = std::cos(angle);
	}

	return (T(1) - c) * ublas::outer_prod(vec,vec) +
				c * unit_matrix(vec.size()) +
				s * skew(vec);
}


template<class matrix_type=ublas::matrix<double> >
typename matrix_type::value_type trace(const matrix_type& mat)
{
	typedef typename matrix_type::value_type T;

	if(mat.size1() != mat.size2())
		return T(0);

	T tr = T(0.);
	for(unsigned int i=0; i<mat.size1(); ++i)
		tr += mat(i,i);
	return tr;
}

// see: https://www.opengl.org/sdk/docs/man2/xhtml/gluPerspective.xml
template<class matrix_type=ublas::matrix<double, ublas::row_major, ublas::bounded_array<double,4*4>>, 
	class T=typename matrix_type::value_type>
matrix_type perspective_matrix(T yfov, T asp, T n, T f)
{
	const T y = cot(0.5*yfov);
	const T x = y/asp;
	const T dsgn = -1.;

	return make_mat<matrix_type>
	({
		{     x,        0.,                0.,              0. },
		{    0.,         y,                0.,              0. },
		{    0.,        0.,  dsgn*(f+n)/(f-n), (-2.*f*n)/(f-n) },
		{    0.,        0.,           dsgn*1.,              0. }
	});
}

// -----------------------------------------------------------------------------
template<typename T, class FKT, const int iDim=get_type_dim<T>::value>
struct is_nan_or_inf_impl
{
	is_nan_or_inf_impl(const FKT&) {}
	bool operator()(T) const { throw Err("No implementation of is_nan_or_inf!"); }
};

template<typename real_type, class FKT>
struct is_nan_or_inf_impl<real_type, FKT, 0>	// scalar impl.
{
	const FKT& m_fkt;
	is_nan_or_inf_impl(const FKT& fkt) : m_fkt(fkt) {}
	bool operator()(real_type d) const { return m_fkt(d); }
};

template<typename vec_type, class FKT>
struct is_nan_or_inf_impl<vec_type, FKT, 1>		// vector impl.
{
	const FKT& m_fkt;
	is_nan_or_inf_impl(const FKT& fkt) : m_fkt(fkt) {}

	bool operator()(const vec_type& vec) const
	{
		for(unsigned int i=0; i<vec.size(); ++i)
			if(m_fkt(vec[i]))
				return true;
		return false;
	}
};

template<typename mat_type, class FKT>
struct is_nan_or_inf_impl<mat_type, FKT, 2>		// matrix impl.
{
	const FKT& m_fkt;
	is_nan_or_inf_impl(const FKT& fkt) : m_fkt(fkt) {}

	bool operator()(const mat_type& mat) const
	{
		for(unsigned int i=0; i<mat.size1(); ++i)
			for(unsigned int j=0; j<mat.size2(); ++j)
				if(m_fkt(mat(i,j)))
					return true;
		return false;
	}
};

template<class T=ublas::matrix<double> >
bool isnan(const T& mat)
{
	typedef typename underlying_value_type<T>::value_type real_type;

	using fkt = std::function<bool(real_type)>;
	fkt stdisnan = (bool(*)(real_type))std::isnan;
	is_nan_or_inf_impl<T, fkt> _isnan(stdisnan);
	return _isnan(mat);
}

template<class T=ublas::matrix<double> >
bool isinf(const T& mat)
{
	typedef typename underlying_value_type<T>::value_type real_type;

	using fkt = std::function<bool(real_type)>;
	fkt stdisinf = (bool(*)(real_type))std::isinf;
	is_nan_or_inf_impl<T, fkt> _isinf(stdisinf);
	return _isinf(mat);
}

template<class T=ublas::matrix<double> >
bool is_nan_or_inf(const T& mat)
{
	typedef typename underlying_value_type<T>::value_type real_type;

	using fkt = std::function<bool(real_type)>;
	fkt stdisnaninf = [](real_type d)->bool { return std::isnan(d) || std::isinf(d); };
	is_nan_or_inf_impl<T, fkt> _isnaninf(stdisnaninf);
	return _isnaninf(mat);
}
// -----------------------------------------------------------------------------


// code for inverse based on boost/libs/numeric/ublas/test/test_lu.cpp
template<class mat_type=ublas::matrix<double>>
bool inverse(const mat_type& mat, mat_type& inv)
{
	using T = typename mat_type::value_type;
	const typename mat_type::size_type N = mat.size1();
	if(N != mat.size2())
		return false;
	//if(isnan(mat) || isinf(mat))
	//	return false;

	try
	{
		mat_type lu = mat;
		ublas::permutation_matrix<> perm(N);

		if(ublas::lu_factorize(lu, perm) != 0)
			return false;

		inv = ublas::identity_matrix<T>(N);
		ublas::lu_substitute(lu, perm, inv);
	}
	catch(const std::exception& ex)
	{
		log_err("Matrix inversion failed with exception: ", ex.what(), ".", "\n",
				"Matrix to be inverted was: ", mat, ".");
		return false;
	}

	return true;
}


template<typename T>
bool solve_linear_approx(const ublas::matrix<T>& M, const ublas::vector<T>& v,
				ublas::vector<T>& x);

// solve Mx = v for x
template<typename T=double>
bool solve_linear(const ublas::matrix<T>& M, const ublas::vector<T>& v,
						ublas::vector<T>& x)
{
	if(M.size1() == M.size2())		// determined, TODO: check rank
	{
		try
		{
			const unsigned int N = M.size1();

			ublas::matrix<T> lu = M;
			ublas::permutation_matrix<> perm(N);

			typename ublas::matrix<T>::size_type sing = ublas::lu_factorize(lu, perm);
			if(sing != 0)
				return false;

			x = v;
			ublas::lu_substitute(lu, perm, x);
		}
		catch(const std::exception& ex)
		{
			log_err("Linear equation solver failed with exception: ", ex.what(), ".");
			return false;
		}
	}
	else if(M.size1() < M.size2())	// underdetermined
	{
		ublas::matrix<T> Q, R;
		if(!qr(M, Q, R))
			return false;
		typedef typename ublas::vector<T>::size_type t_int;

		// M x = v
		// QR x = v
		// R x = Q^T v

		ublas::vector<T> vnew = ublas::prod(ublas::trans(Q), v);

		/*std::cout << "M = " << M << std::endl;
		std::cout << "Q = " << Q << std::endl;
		std::cout << "R = " << R << std::endl;
		std::cout << "v' = " << vnew << std::endl;*/

		x = ublas::zero_vector<T>(M.size2());
		ublas::vector<T> xnew(R.size1());
		bool bOk = 0;

		/*
		// pick one of the solutions
		// TODO: resort columns so that Rupper doesn't get singular
		ublas::matrix<T> Rupper = ublas::subrange(R, 0, R.size1(),
						R.size2()-R.size1(), R.size2());
		//std::cout << "Rupper = " << Rupper << std::endl;

		bOk = solve_linear(Rupper, vnew, xnew);

		for(t_int i=0; i<xnew.size(); ++i)
			x[x.size()-xnew.size()+i] = xnew[i];
		*/

		// find non-singular right-upper submatrix
		std::vector<t_int> vecDelCols;
		unsigned int iNumToDel = R.size2()-R.size1();
		if(iNumToDel != 1)
		{
			log_err(__func__, " not yet implemented.");
			return false;
		}

		bool bFoundNonSingular = 0;
		ublas::matrix<T> Rsub;
		for(int iCol=int(R.size2()-1); iCol>=0; --iCol)
		{
			Rsub = remove_column(R, (unsigned int)iCol);
			//std::cout << "Rsub" << Rsub << std::endl;
			//std::cout << "det: " << determinant(Rsub) << std::endl;

			T det = determinant<ublas::matrix<T>>(Rsub);
			if(!float_equal(det, 0.))
			{
				bFoundNonSingular = 1;
				vecDelCols.push_back(iCol);
				break;
			}
		}

		if(!bFoundNonSingular)
		{
			log_err("No non-singluar submatrix found in linear equation solver.");
			return false;
		}

		bOk = solve_linear(Rsub, vnew, xnew);
		//std::cout << "Rsub = " << Rsub << std::endl;
		//std::cout << "v' = " << vnew << std::endl;
		//std::cout << "x' = " << xnew << std::endl;

		for(t_int i=0, i0=0; i<xnew.size() && i0<x.size(); ++i, ++i0)
		{
			while(std::find(vecDelCols.begin(), vecDelCols.end(), i0) != vecDelCols.end())
				++i0;
			x[i0] = xnew[i];
		}

		return bOk;
	}
	else if(M.size1() > M.size2())	// overdetermined
		return solve_linear_approx<T>(M,v,x);
	else
		return false;

	return true;
}

// solve M^T M x = M^T v for x
template<typename T=double>
bool solve_linear_approx(const ublas::matrix<T>& M, const ublas::vector<T>& v,
						ublas::vector<T>& x)
{
	if(M.size1() <= M.size2())
	{
		//std::cerr << "Error: Matrix has to be overdetermined." << std::endl;
		return false;
	}

	ublas::matrix<T> Q, R;
	if(!qr(M, Q, R))
		return false;

	// M^T M x = M^T v
	// R^T Q^T Q R x = R^T Q^T v
	// R^T R x = R^T Q^T v

	const ublas::matrix<T> RT = ublas::trans(R);
	const ublas::matrix<T> QT = ublas::trans(Q);
	const ublas::matrix<T> RTR = ublas::prod(RT, R);
	const ublas::matrix<T> RTQT = ublas::prod(RT, QT);

	const ublas::vector<T> vnew = ublas::prod(RTQT, v);
	return solve_linear<T>(RTR, vnew, x);
}


template<class matrix_type=ublas::matrix<double> >
bool is_diag_matrix(const matrix_type& mat)
{
	typedef typename matrix_type::value_type T;

	for(unsigned int i=0; i<mat.size1(); ++i)
		for(unsigned int j=0; j<mat.size2(); ++j)
		{
			if(i==j) continue;

			if(!float_equal(mat(i,j), T(0.)))
				return false;
		}

	return true;
}


template<class matrix_type=ublas::matrix<double>, class vec_type=ublas::vector<double>,
	class container_type=std::initializer_list<vec_type>, const bool bRowMat>
inline matrix_type row_col_matrix(const container_type& vecs)
{
	if(vecs.size() == 0)
		return matrix_type(0,0);

	const unsigned int N = vecs.size();
	const unsigned int M = vecs.begin()->size();

	matrix_type mat(bRowMat?N:M, bRowMat?M:N);
	unsigned int j=0;
	for(typename container_type::const_iterator iter=vecs.begin(); iter!=vecs.end(); ++iter)
	{
		const vec_type& vec = *iter;

		for(unsigned int i=0; i<vec.size(); ++i)
		{
			if(bRowMat)
				mat(j,i) = vec[i];
			else
				mat(i,j) = vec[i];
		}

		++j;
	}

	return mat;
}

// vectors form rows of matrix
template<class matrix_type=ublas::matrix<double>, class vec_type=ublas::vector<double>,
	class container_type=std::initializer_list<vec_type> >
matrix_type row_matrix(const container_type& vecs)
{
	return row_col_matrix<matrix_type, vec_type, container_type, true>(vecs);
}

// vectors form columns of matrix
template<class matrix_type=ublas::matrix<double>, class vec_type=ublas::vector<double>,
	class container_type=std::initializer_list<vec_type> >
matrix_type column_matrix(const container_type& vecs)
{
	return row_col_matrix<matrix_type, vec_type, container_type, false>(vecs);
}

template<typename vector_type = ublas::vector<double> >
vector_type cross_3(const vector_type& vec0, const vector_type& vec1)
{
	return make_vec<vector_type>
		({
			vec0[1]*vec1[2] - vec1[1]*vec0[2],
			vec0[2]*vec1[0] - vec1[2]*vec0[0],
			vec0[0]*vec1[1] - vec1[0]*vec0[1]
		});
}


template<class matrix_type/*=ublas::matrix<double>*/>
typename matrix_type::value_type determinant(const matrix_type& mat)
{
	typedef typename matrix_type::value_type T;

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
	/*
	else if(mat.size1()==3)
	{
		ublas::vector<T> vec0 = get_column(mat, 0);
		ublas::vector<T> vec1 = get_column(mat, 1);
		ublas::vector<T> vec2 = get_column(mat, 2);

		ublas::vector<T> vecCross = cross_3<ublas::vector<T> >(vec1, vec2);
		return ublas::inner_prod(vec0, vecCross);
	}*/

	const unsigned int i = 0;
	T val = T(0);

	for(unsigned int j=0; j<mat.size2(); ++j)
	{
		T dSign = 1.;
		if(is_odd<unsigned int>(i+j))
			dSign = -1.;
		val += dSign * mat(i,j) * determinant<matrix_type>(submatrix(mat, i, j));
	}

	return val;
}

template<class matrix_type=ublas::matrix<double> >
typename matrix_type::value_type get_volume(const matrix_type& mat)
{
	//typedef typename matrix_type::value_type T;
	return determinant<matrix_type>(mat);
}


template<class matrix_type=ublas::matrix<double> >
typename matrix_type::value_type get_ellipsoid_volume(const matrix_type& mat)
{
	typedef typename matrix_type::value_type T;
	T tDet = determinant<matrix_type>(mat);

	return T(4./3. * M_PI * std::sqrt(1./tDet));
}



// calculate fractional coordinate basis vectors from angles
// see: http://www.bmsc.washington.edu/CrystaLinks/man/pdb/part_75.html
// for the reciprocal lattice this is equal to the B matrix from Acta Cryst. (1967), 22, 457
template<class t_vec>
bool fractional_basis_from_angles(typename t_vec::value_type a,
					typename t_vec::value_type b,
					typename t_vec::value_type c,
					typename t_vec::value_type alpha,
					typename t_vec::value_type beta,
					typename t_vec::value_type gamma,
				t_vec& veca, t_vec& vecb, t_vec& vecc)
{
	typedef typename t_vec::value_type T;

	const T dSG = std::sin(gamma);
	const T dCG = std::cos(gamma);
	const T dCA = std::cos(alpha);
	const T dCB = std::cos(beta);

	const T dCA2 = dCA*dCA;
	const T dCB2 = dCB*dCB;
	const T dCG2 = dCG*dCG;

	const T dVol =a*b*c*std::sqrt(1.- dCA2 - dCB2 - dCG2 + 2.*dCA*dCB*dCG);
	//std::cout << "vol = " <<  dVol << std::endl;

	if(std::isinf(dVol) || std::isnan(dVol))
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


// signed angle wrt basis
template<typename vec_type>
typename vec_type::value_type vec_angle(const vec_type& vec)
{
	if(vec.size() == 2)
		return std::atan2(vec[1], vec[0]);

	throw Err("vec_angle not yet implemented for size != 2.");
}


// -----------------------------------------------------------------------------
template<typename T> void set_eps_0(T& d);

template<typename T, bool bScalar=std::is_scalar<T>::value>
struct set_eps_0_impl
{
	void operator()(T&) const { throw Err("No implementation of set_eps_0!"); }
};

template<typename real_type>
struct set_eps_0_impl<real_type, 1>
{
	void operator()(real_type& d) const
	{
		if(std::fabs(d) < std::numeric_limits<real_type>::epsilon())
			d = real_type(0);
	}
};

template<typename vec_type>
struct set_eps_0_impl<vec_type, 0>
{
	void operator()(vec_type& vec) const
	{
		typedef typename vec_type::value_type real_type;

		for(real_type& d : vec)
			set_eps_0<real_type>(d);
	}
};

template<typename T>
void set_eps_0(T& d)
{
	set_eps_0_impl<T, std::is_scalar<T>::value> op;
	op(d);
}
// -----------------------------------------------------------------------------


// signed angle between two vectors
template<typename vec_type>
typename vec_type::value_type vec_angle(const vec_type& vec0,
					const vec_type& vec1,
					const vec_type* pvec_norm=0)
{
	typedef typename vec_type::value_type real_type;

	if(vec0.size() != vec1.size())
		throw Err("In vec_angle: Vector sizes do not match.");

	if(vec0.size() == 2)
	{
		return vec_angle<vec_type>(vec0) - vec_angle<vec_type>(vec1);
	}
	if(vec0.size() == 3)
	{
		//real_type dNorm0 = ublas::norm_2(vec0);
		//real_type dNorm1 = ublas::norm_2(vec1);

		real_type dC = ublas::inner_prod(vec0, vec1);
		vec_type veccross = cross_3<vec_type>(vec0, vec1);
		real_type dS = ublas::norm_2(veccross);

		real_type dAngle = std::atan2(dS, dC);

		// get signed angle
		if(pvec_norm)
		{
			if(ublas::inner_prod(veccross, *pvec_norm) < real_type(0))
				dAngle = -dAngle;
		}

		return dAngle;
	}

	throw Err("vec_angle only implemented for size == 2 and size == 3.");
}


template<class T, LinalgType ty=get_linalg_type<T>::value>
struct vec_angle_unsigned_impl
{
	void operator()(const T&, const T&) const { throw Err("No implementation of vec_angle_unsigned!"); }
};

// unsigned angle between two vectors
template<class T>
struct vec_angle_unsigned_impl<T, LinalgType::VECTOR>
{
	typename T::value_type operator()(const T& q1, const T& q2) const
	{
		typedef typename T::value_type REAL;

		if(q1.size() != q2.size())
			return REAL();

		REAL dot = REAL();
		REAL len1 = REAL();
		REAL len2 = REAL();
		for(unsigned int i=0; i<q1.size(); ++i)
		{
			dot += q1[i]*q2[i];

			len1 += q1[i]*q1[i];
			len2 += q2[i]*q2[i];
		}

		len1 = std::sqrt(len1);
		len2 = std::sqrt(len2);

		dot /= len1;
		dot /= len2;

		return std::acos(dot);
	}
};

template<class T>
typename T::value_type vec_angle_unsigned(const T& q1, const T& q2)
{
	return vec_angle_unsigned_impl<T>()(q1, q2);
}

// -----------------------------------------------------------------------------


// see: http://run.usc.edu/cs520-s12/assign2/p245-shoemake.pdf
template<class T>
T slerp(const T& q1, const T& q2, typename T::value_type t)
{
	typedef typename T::value_type REAL;

	REAL angle = vec_angle_unsigned<T>(q1, q2);

	T q = std::sin((1.-t)*angle)/std::sin(angle) * q1 +
			std::sin(t*angle)/std::sin(angle) * q2;

	return q;
}



// --------------------------------------------------------------------------------


// see e.g.: http://www.itl.nist.gov/div898/handbook/pmc/section5/pmc541.htm
template<typename T=double>
ublas::matrix<T> covariance(const std::vector<ublas::vector<T>>& vecVals,
			const std::vector<T>* pProb = 0)
{
	if(vecVals.size() == 0) return ublas::matrix<T>();

	using t_vecvec = typename std::remove_reference<decltype(vecVals)>::type;
	using t_innervec_org = decltype(vecVals[0]);
	using t_innervec = typename std::remove_const<
						typename std::remove_reference<t_innervec_org>::type>
								::type;

	t_innervec vecMean = mean_value<t_vecvec>(vecVals);
	//std::cout << "Mean: " << vecMean << std::endl;

	ublas::matrix<T> matCov(vecVals[0].size(), vecVals[0].size());

	T tSum = T(0);
	const std::size_t N = vecVals.size();
	for(std::size_t i=0; i<N; ++i)
	{
		T tprob = 1.;

		t_innervec vec = vecVals[i] - vecMean;
		ublas::matrix<T> matOuter = ublas::outer_prod(vec, vec);

		if(pProb)
		{
			tprob = (*pProb)[i];
			matOuter *= tprob;
		}

		matCov += matOuter;
		tSum += tprob;
	}
	matCov /= tSum;

	return matCov;
}



// --------------------------------------------------------------------------------


template<typename T=double, typename... Args>
void to_gl_array(const ublas::matrix<T, Args...>& mat, T* glmat)
{
	glmat[0]=mat(0,0);  glmat[1]=mat(1,0);  glmat[2]=mat(2,0);
	glmat[4]=mat(0,1);  glmat[5]=mat(1,1);  glmat[6]=mat(2,1);
	glmat[8]=mat(0,2);  glmat[9]=mat(1,2);  glmat[10]=mat(2,2);

	if(mat.size1()>=4 && mat.size2()>=4)
	{
		glmat[3]=mat(3,0); glmat[7]=mat(3,1); glmat[11]=mat(3,2);
		glmat[12]=mat(0,3); glmat[13]=mat(1,3); glmat[14]=mat(2,3); glmat[15]=mat(3,3);
	}
	else
	{
		glmat[3]=0; glmat[7]=0; glmat[11]=0;
		glmat[12]=0; glmat[13]=0; glmat[14]=0; glmat[15]=1;
	}
}



// --------------------------------------------------------------------------------


#include <boost/math/common_factor_rt.hpp>

template<class t_vec=ublas::vector<int>>
t_vec get_gcd_vec(const t_vec& vec)
{
	if(vec.size() <= 1)
		return vec;

	typedef typename t_vec::value_type t_int;

	t_int igcd_total = 1;
	for(std::size_t i=0; i<vec.size()-1; ++i)
	{
		t_int i0 = vec[i];
		t_int i1 = vec[i+1];

		t_int igcd = boost::math::gcd<t_int>(i0, i1);

		if(i==0)
			igcd_total = igcd;
		else
			igcd_total = boost::math::gcd<t_int>(igcd, igcd_total);
	}

	if(igcd_total == 0)
		return vec;

	return vec/igcd_total;
}

#endif
