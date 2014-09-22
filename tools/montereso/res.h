/*
 * Monte-Carlo resolution calculation
 *
 * Author: Tobias Weber
 * Date: July 2012, Sep. 2014
 *
 * (based on the Mcstas' mcresplot perl program (www.mcstas.org)
 * 	and the rescal5 matlab program)
 */

#ifndef __MONTERES_H__
#define __MONTERES_H__

#include "helper/linalg.h"

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


Resolution calc_res(unsigned int uiLen, const ublas::vector<double>* Q_vec,
					const ublas::vector<double>& Q_avg,
					const double *pp_vec = 0,
					const double *pp_sum = 0);

Resolution calc_res(unsigned int uiLen,
			  const double *_Q_x, const double *_Q_y, const double *_Q_z,
			  const double *_E);

Resolution calc_res(unsigned int uiLen,
			  const double *ki_x, const double *ki_y, const double *ki_z,
			  const double *kf_x, const double *kf_y, const double *kf_z,
			  const double *p_i=0, const double *p_f=0);

#endif
