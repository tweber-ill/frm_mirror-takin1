/*
 * resolution ellipse calculation
 * @author tweber
 * @date 14-may-2013
 * @license GPLv2
 *
 * @desc This is a reimplementation in C++ of the file rc_projs.m of the
 *    			rescal5 package by Zinkin, McMorrow, Tennant, Farhi, and Wildes:
 *    			http://www.ill.eu/en/instruments-support/computing-for-science/cs-software/all-software/matlab-ill/rescal-for-matlab/
 */

#ifndef __RES_ELLIPSE_
#define __RES_ELLIPSE_

#include <string>
#include <ostream>
#include <cmath>
#include <vector>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
namespace ublas = boost::numeric::ublas;

#include "tlibs/math/linalg.h"
#include "tlibs/math/geo.h"
#include "defs.h"

using t_real_elli = t_real_reso;


struct Ellipse
{
	tl::QuadEllipsoid<t_real_elli> quad;

	t_real_elli phi, slope;
	t_real_elli x_hwhm, y_hwhm;
	t_real_elli x_offs, y_offs;
	t_real_elli area;

	std::string x_lab, y_lab;

	ublas::vector<t_real_elli> operator()(t_real_elli t) const;
	void GetCurvePoints(std::vector<t_real_elli>& x, std::vector<t_real_elli>& y,
		unsigned int iPoints=512, t_real_elli *pLRTB=0);
};

struct Ellipsoid
{
	tl::QuadEllipsoid<t_real_elli> quad;

	//t_real_elli alpha, beta, gamma;
	ublas::matrix<t_real_elli> rot;

	t_real_elli x_hwhm, y_hwhm, z_hwhm;
	t_real_elli x_offs, y_offs, z_offs;
	t_real_elli vol;

	std::string x_lab, y_lab, z_lab;
};

struct Ellipsoid4d
{
	tl::QuadEllipsoid<t_real_elli> quad;

	ublas::matrix<t_real_elli> rot;

	t_real_elli x_hwhm, y_hwhm, z_hwhm, w_hwhm;
	t_real_elli x_offs, y_offs, z_offs, w_offs;
	t_real_elli vol;

	std::string x_lab, y_lab, z_lab, w_lab;
};

enum class EllipseCoordSys : int
{
	AUTO = -1,

	Q_AVG = 0,	// Q|| Qperp system (1/A)
	RLU,		// absolute hkl system (rlu)
	RLU_ORIENT	// system using scattering plane (rlu)
};

extern std::ostream& operator<<(std::ostream& ostr, const struct Ellipse& ell);
extern const std::string& ellipse_labels(int iCoord, EllipseCoordSys = EllipseCoordSys::Q_AVG);
extern struct Ellipse calc_res_ellipse(const ublas::matrix<t_real_elli>& reso, const ublas::vector<t_real_elli>& Q_avg, int iX, int iY, int iInt, int iRem1=-1, int iRem2=-1);
extern struct Ellipsoid calc_res_ellipsoid(const ublas::matrix<t_real_elli>& reso, const ublas::vector<t_real_elli>& Q_avg, int iX, int iY, int iZ, int iInt, int iRem=-1);
extern struct Ellipsoid4d calc_res_ellipsoid4d(const ublas::matrix<t_real_elli>& reso, const ublas::vector<t_real_elli>& Q_avg);



/*
 * this is a 1:1 C++ reimplementation of 'rc_int' from 'mcresplot' and 'rescal5'
 * (see also [eck14], equ. 57)
 * integrate over row/column iIdx
 */
template<class T = t_real_elli>
ublas::matrix<T> ellipsoid_gauss_int(const ublas::matrix<T>& mat, std::size_t iIdx)
{
	ublas::vector<T> b(mat.size1());
	for(std::size_t i=0; i<mat.size1(); ++i)
		b[i] = 2.*mat(i,iIdx);
	b = tl::remove_elem(b, iIdx);
	ublas::matrix<T> bb = ublas::outer_prod(b,b)/4.;

	ublas::matrix<T> m = tl::remove_elems(mat, iIdx);
	m -= bb/mat(iIdx, iIdx);
	return m;
}

/*
 * (see also [eck14], equ. 57)
 */
template<class T = t_real_elli>
ublas::vector<T> ellipsoid_gauss_int(const ublas::vector<T>& vec,
	const ublas::matrix<T>& mat, std::size_t iIdx)
{
	ublas::vector<T> vecInt(vec.size()-1);

	for(std::size_t i=0, j=0; i<vec.size(); ++i)
	{
		if(i==iIdx) continue;

		vecInt[j] = vec[i] - vec[iIdx]*mat(i,iIdx)/mat(iIdx, iIdx);
		++j;
	}

	return vecInt;
}

#endif
