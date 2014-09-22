/*
 * resolution ellipse calculation
 * @author tweber
 * @date 14-may-2013
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

struct Ellipse
{
	double phi, slope;
	double x_hwhm, y_hwhm;
	double x_offs, y_offs;
	double area;

	std::string x_lab, y_lab;

	ublas::vector<double> operator()(double t) const;
	const char* GetModelName() const { return "ellipse"; };

	void GetCurvePoints(std::vector<double>& x, std::vector<double>& y, unsigned int iPoints=512);
};

struct Ellipsoid
{
	//double alpha, beta, gamma;
	ublas::matrix<double> rot;

	double x_hwhm, y_hwhm, z_hwhm;
	double x_offs, y_offs, z_offs;
	double vol;

	std::string x_lab, y_lab, z_lab;
};

struct Ellipsoid4d
{
	ublas::matrix<double> rot;

	double x_hwhm, y_hwhm, z_hwhm, w_hwhm;
	double x_offs, y_offs, z_offs, w_offs;
	double vol;

	std::string x_lab, y_lab, z_lab, w_lab;
};

extern std::ostream& operator<<(std::ostream& ostr, const Ellipse& ell);
extern Ellipse calc_res_ellipse(const ublas::matrix<double>& reso, const ublas::vector<double>& Q_avg, int iX, int iY, int iInt, int iRem1=-1, int iRem2=-1);
extern Ellipsoid calc_res_ellipsoid(const ublas::matrix<double>& reso, const ublas::vector<double>& Q_avg, int iX, int iY, int iZ, int iInt, int iRem=-1);
extern Ellipsoid4d calc_res_ellipsoid4d(const ublas::matrix<double>& reso, const ublas::vector<double>& Q_avg);

extern void mc_neutrons(const Ellipsoid4d& ell4d, unsigned int iNum, bool bCenter,
						std::vector<ublas::vector<double>>& vecResult);


extern ublas::matrix<double> gauss_int(const ublas::matrix<double>& mat, unsigned int iIdx);

#endif
