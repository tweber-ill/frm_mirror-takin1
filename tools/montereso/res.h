/*
 * Monte-Carlo resolution calculation
 *
 * @author Tobias Weber
 * @date July 2012, Sep. 2014
 * @copyright GPLv2
 *
 * (based on the Mcstas' mcresplot perl program (www.mcstas.org)
 * 	and the rescal5 matlab program)
 */

#ifndef __MONTERES_H__
#define __MONTERES_H__

#include "tlibs/math/linalg.h"
namespace ublas = boost::numeric::ublas;

struct Resolution
{
	// covariance matrix
	ublas::matrix<double> cov;

	// resolution matrix
	bool bHasRes;
	ublas::matrix<double> res;

	// half-widths
	ublas::vector<double> dQ;	// in 1/A and meV

	// ellipse origin
	ublas::vector<double> Q_avg;
};


Resolution calc_res(const std::vector<ublas::vector<double>>& Q_vec,
					const ublas::vector<double>& Q_avg,
					const std::vector<double>* pp_vec = 0);

Resolution calc_res(unsigned int uiLen,
			  const double *_Q_x, const double *_Q_y, const double *_Q_z,
			  const double *_E);

Resolution calc_res(unsigned int uiLen,
			  const double *ki_x, const double *ki_y, const double *ki_z,
			  const double *kf_x, const double *kf_y, const double *kf_z,
			  const double *p_i=0, const double *p_f=0);

#endif
