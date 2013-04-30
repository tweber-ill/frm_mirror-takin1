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
