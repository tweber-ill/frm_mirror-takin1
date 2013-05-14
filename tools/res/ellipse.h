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

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
namespace ublas = boost::numeric::ublas;

struct Ellipse
{
        double phi;
        double x_hwhm, y_hwhm;
        double x_offs, y_offs;

        std::string x_lab, y_lab;
};

extern std::ostream& operator<<(std::ostream& ostr, const Ellipse& ell);
extern Ellipse calc_res_ellipse(const ublas::matrix<double>& reso, int iX, int iY, int iInt, int iRem1=-1, int iRem2=-1);

#endif
