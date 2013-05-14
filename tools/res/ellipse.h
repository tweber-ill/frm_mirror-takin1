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

#include "../../fitter/fitter.h"

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
namespace ublas = boost::numeric::ublas;

struct Ellipse : public FunctionModel_param
{
        double phi;
        double x_hwhm, y_hwhm;
        double x_offs, y_offs;

        std::string x_lab, y_lab;

        virtual ublas::vector<double> operator()(double t) const
        {
        	ublas::vector<double> vec(2);

            vec[0] = x_hwhm*std::cos(2.*M_PI*t)*std::cos(phi) - y_hwhm*std::sin(2.*M_PI*t)*std::sin(phi) + x_offs;
            vec[1] = x_hwhm*std::cos(2.*M_PI*t)*std::sin(phi) + y_hwhm*std::sin(2.*M_PI*t)*std::cos(phi) + y_offs;

            return vec;
        }
};

extern std::ostream& operator<<(std::ostream& ostr, const Ellipse& ell);
extern Ellipse calc_res_ellipse(const ublas::matrix<double>& reso, int iX, int iY, int iInt, int iRem1=-1, int iRem2=-1);

#endif
