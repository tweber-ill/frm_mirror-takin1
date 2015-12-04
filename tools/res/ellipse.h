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


struct Ellipse
{
	tl::QuadEllipsoid<double> quad;

	double phi, slope;
	double x_hwhm, y_hwhm;
	double x_offs, y_offs;
	double area;

	std::string x_lab, y_lab;

	ublas::vector<double> operator()(double t) const;
	void GetCurvePoints(std::vector<double>& x, std::vector<double>& y,
						unsigned int iPoints=512,
						double *pLRTB=0);
};

struct Ellipsoid
{
	tl::QuadEllipsoid<double> quad;

	//double alpha, beta, gamma;
	ublas::matrix<double> rot;

	double x_hwhm, y_hwhm, z_hwhm;
	double x_offs, y_offs, z_offs;
	double vol;

	std::string x_lab, y_lab, z_lab;
};

struct Ellipsoid4d
{
	tl::QuadEllipsoid<double> quad;

	ublas::matrix<double> rot;

	double x_hwhm, y_hwhm, z_hwhm, w_hwhm;
	double x_offs, y_offs, z_offs, w_offs;
	double vol;

	std::string x_lab, y_lab, z_lab, w_lab;
};

enum class EllipseCoordSys : int
{
	AUTO = -1,

	Q_AVG = 0,	// Q|| Qperp system (1/A)
	RLU,		// absolute hkl system (rlu)
	RLU_ORIENT	// system using scattering plane (rlu)
};

extern std::ostream& operator<<(std::ostream& ostr, const Ellipse& ell);
extern const std::string& ellipse_labels(int iCoord, EllipseCoordSys = EllipseCoordSys::Q_AVG);
extern Ellipse calc_res_ellipse(const ublas::matrix<double>& reso, const ublas::vector<double>& Q_avg, int iX, int iY, int iInt, int iRem1=-1, int iRem2=-1);
extern Ellipsoid calc_res_ellipsoid(const ublas::matrix<double>& reso, const ublas::vector<double>& Q_avg, int iX, int iY, int iZ, int iInt, int iRem=-1);
extern Ellipsoid4d calc_res_ellipsoid4d(const ublas::matrix<double>& reso, const ublas::vector<double>& Q_avg);


enum class McNeutronCoords
{
	DIRECT = 0,
	ANGS = 1,
	RLU = 2
};

struct McNeutronOpts
{
	McNeutronCoords coords = McNeutronCoords::RLU;
	ublas::matrix<double> matU, matB, matUB;
	ublas::matrix<double> matUinv, matBinv, matUBinv;
	double dAngleQVec0;

	bool bCenter;
};

extern void mc_neutrons(const Ellipsoid4d& ell4d, unsigned int iNum,
						const McNeutronOpts& opts,
						std::vector<ublas::vector<double>>& vecResult);


/*
 * this is a 1:1 C++ reimplementation of 'rc_int' from 'mcresplot' and 'rescal5'
 * integrate over row/column iIdx
 */
template<class T = double>
ublas::matrix<T> ellipsoid_gauss_int(const ublas::matrix<T>& mat, unsigned int iIdx)
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

template<class T = double>
ublas::vector<T> ellipsoid_gauss_int(const ublas::vector<T>& vec,
									const ublas::matrix<T>& mat,
									unsigned int iIdx)
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
