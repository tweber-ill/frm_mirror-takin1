/*
 * Monte-Carlo resolution calculation
 *
 * Author: Tobias Weber
 * Date: July 2012, Sep. 2014
 *
 * (based on the Mcstas' mcresplot perl program (www.mcstas.org)
 * 	and the rescal5 matlab program)
 */

#include "res.h"
#include "../res/cn.h"

#include "helper/neutrons.hpp"
#include "helper/log.h"

#include <algorithm>
#include <boost/algorithm/minmax_element.hpp>
#include <fstream>
#include <ios>
#include <limits>
#include <memory>

using namespace ublas;


/*
 * this function tries to be a 1:1 C++ reimplementation of the Perl function
 * 'read_mcstas_res' of the McStas 'mcresplot' program
 */
Resolution calc_res(unsigned int uiLen, const vector<double>* Q_vec,
					const vector<double>& Q_avg,
					const double *pp_vec, const double *pp_sum)
{
	std::unique_ptr<vector<double>[]> ptr_q_trafo_vec(new vector<double>[uiLen]);
	vector<double> *Q_trafo_vec = ptr_q_trafo_vec.get();

	const double p_sum = pp_sum ? *pp_sum : double(uiLen);

	vector<double> Q_dir(3), Q_perp(3);
	Q_dir[0] = Q_avg[0];
	Q_dir[1] = Q_avg[1];
	Q_dir[2] = Q_avg[2];
	Q_dir = Q_dir / norm_2(Q_dir);

	Q_perp[0] = -Q_dir[1];
	Q_perp[1] = Q_dir[0];
	Q_perp[2] = Q_dir[2];


	matrix<double> trafo(4, 4);

	// point to Q_dir direction
	trafo(0,0) = Q_dir[0];
	trafo(1,0) = Q_dir[1];
	trafo(2,0) = Q_dir[2];

	// perpendicular to Q_dir
	trafo(0,1) = Q_perp[0];
	trafo(1,1) = Q_perp[1];
	trafo(2,1) = Q_perp[2];

	vector<double> vecUp = cross_3(Q_dir, Q_perp);
	// z direction up
	trafo(0,2) = vecUp[0];
	trafo(1,2) = vecUp[1];
	trafo(2,2) = vecUp[2];

	// energy
	trafo(3,0) = trafo(0,3) = 0.;
	trafo(3,1) = trafo(1,3) = 0.;
	trafo(3,2) = trafo(2,3) = 0.;
	trafo(3,3) = 1.;

	//std::cout << "trafo = " << trafo << std::endl;


	matrix<double> ubermatrix(uiLen, 4);

	for(unsigned int uiRow=0; uiRow<uiLen; ++uiRow)
	{
		const vector<double>& Q = Q_vec[uiRow];
		const double p = pp_vec ? pp_vec[uiRow] : 1.;

		vector<double> Q_delta(4);
		Q_delta = Q - Q_avg;

		for(unsigned int ui=0; ui<4; ++ui)
			ubermatrix(uiRow,ui) = Q_delta[ui] * sqrt(p);
	}


	Resolution reso;
	reso.Q_avg = prod(trans(trafo), Q_avg);

	matrix<double>& res = reso.res;
	matrix<double>& cov = reso.cov;	
	res.resize(4,4,0);
	cov.resize(4,4,0);

	cov = prod(trans(ubermatrix), ubermatrix) / p_sum;
	cov = prod(cov, trafo);
	cov = prod(trans(trafo), cov);

	log_info("Covariance matrix: ", cov);
	reso.bHasRes = inverse(cov, res);

	if(reso.bHasRes)
	{
		reso.dQ.resize(4, 0);
		for(int iQ=0; iQ<4; ++iQ)
			reso.dQ[iQ] = SIGMA2HWHM/sqrt(res(iQ,iQ));
		
		log_info("Resolution matrix: ", res);

		const vector<double>& dQ = reso.dQ;
		const vector<double>& Q_avg = reso.Q_avg;

		std::ostringstream ostrVals;
		ostrVals << "Gaussian HWHM values: ";
		std::copy(dQ.begin(), dQ.end(), std::ostream_iterator<double>(ostrVals, ", "));

		std::ostringstream ostrElli;
		ostrElli << "Ellipsoid offsets: ";
		std::copy(Q_avg.begin(), Q_avg.end(), std::ostream_iterator<double>(ostrElli, ", "));

		log_info(ostrVals.str());
		log_info(ostrElli.str());
	}
	else
	{
		log_err("Covariance matrix could not be inverted!");
	}

	return reso;
}


Resolution calc_res(unsigned int uiLen,
			  const double *_Q_x, const double *_Q_y, const double *_Q_z,
			  const double *_E)
{
	vector<double> Q_avg(4);
	Q_avg[0] = Q_avg[1] = Q_avg[2] = Q_avg[3] = 0.;

	std::unique_ptr<vector<double>[]> ptr_Q_vec(new vector<double>[uiLen]);
	vector<double> *Q_vec = ptr_Q_vec.get();

	for(unsigned int uiRow=0; uiRow<uiLen; ++uiRow)
	{
		vector<double>& Q = Q_vec[uiRow];

		Q.resize(4, 0);
		Q[0] = _Q_x[uiRow];
		Q[1] = _Q_y[uiRow];
		Q[2] = _Q_z[uiRow];
		Q[3] = _E[uiRow];

		Q_avg += Q;
	}
	Q_avg /= double(uiLen);
	log_info("Average Q vector: ", Q_avg);

	return calc_res(uiLen, Q_vec, Q_avg);
}


/*
 * this function tries to be a 1:1 C++ reimplementation of the Perl function
 * 'read_mcstas_res' of the McStas 'mcresplot' program
 */
Resolution calc_res(unsigned int uiLen,
			  const double *_ki_x, const double *_ki_y, const double *_ki_z,
			  const double *_kf_x, const double *_kf_y, const double *_kf_z,
			  const double *_p_i, const double *_p_f)
{
	log_info("Calculating resolution...");

	std::unique_ptr<vector<double>[]> ptr_Q_vec(new vector<double>[uiLen]);
	vector<double> *Q_vec = ptr_Q_vec.get();


	std::unique_ptr<double[]> ptr_dE_vec(new double[uiLen]);
	std::unique_ptr<double[]> ptr_p_vec(new double[uiLen]);

	double *dE_vec = ptr_dE_vec.get();
	double *p_vec = ptr_p_vec.get();


	const double pi_max = _p_i ? *std::max_element(_p_i, _p_i+uiLen) : 1.;
	const double pf_max = _p_f ? *std::max_element(_p_f, _p_f+uiLen) : 1.;
	const double p_max = fabs(pi_max*pf_max);


	vector<double> Q_avg(4);
	Q_avg[0] = Q_avg[1] = Q_avg[2] = Q_avg[3] = 0.;

	double p_sum = 0.;

	for(unsigned int uiRow=0; uiRow<uiLen; ++uiRow)
	{
		vector<double>& Q = Q_vec[uiRow];
		Q.resize(3, 0);
		double& p = p_vec[uiRow];

		p = (_p_i && _p_f) ? fabs(_p_i[uiRow]*_p_f[uiRow]) : 1.;
		p /= p_max;		// normalize p to 0..1
		p_sum += p;

		double &dE = dE_vec[uiRow];

		vector<double> ki(3), kf(3);
		ki[0]=_ki_x[uiRow]; ki[1]=_ki_y[uiRow]; ki[2]=_ki_z[uiRow];
		kf[0]=_kf_x[uiRow]; kf[1]=_kf_y[uiRow]; kf[2]=_kf_z[uiRow];

		Q = ki - kf;
		double Ei = KSQ2E * inner_prod(ki, ki);
		double Ef = KSQ2E * inner_prod(kf, kf);
		dE = Ei - Ef;

		// insert the energy into the Q vector
		Q.resize(4, true);
		Q[3] = dE;

		Q_avg += Q*p;
	}
	Q_avg /= p_sum;
	log_info("Average Q vector: ", Q_avg);


	return calc_res(uiLen, Q_vec, Q_avg, p_vec, &p_sum);
}
