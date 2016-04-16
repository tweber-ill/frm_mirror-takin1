/**
 * implementation of Violini's TOF reso algorithm
 * @author Tobias Weber
 * @date apr-2016
 * @license GPLv2
 *
 * @desc for algo, see: [viol14] N. Violini et al., NIM A 736 (2014) pp. 31-39
 */

#include "viol.h"
#include "ellipse.h"
#include "tlibs/math/linalg.h"
#include "tlibs/math/geo.h"
#include "tlibs/math/math.h"
#include "tlibs/log/log.h"

#include <string>
#include <iostream>

typedef t_real_reso t_real;
typedef ublas::matrix<t_real> t_mat;
typedef ublas::vector<t_real> t_vec;

using angle = tl::t_angle_si<t_real>;
using wavenumber = tl::t_wavenumber_si<t_real>;
using velocity = tl::t_velocity_si<t_real>;
using t_time = tl::t_time_si<t_real>;
using energy = tl::t_energy_si<t_real>;
using length = tl::t_length_si<t_real>;
using mass = tl::t_mass_si<t_real>;

static const auto rads = tl::get_one_radian<t_real>();
static const length angs = tl::get_one_angstrom<t_real>();
static const energy meV = tl::get_one_meV<t_real>();
static const t_time sec = tl::get_one_second<t_real>();
static const length meter = tl::get_one_meter<t_real>();


ResoResults calc_viol(const ViolParams& params)
{
	ResoResults res;
	res.Q_avg.resize(4);

	//const energy E = params.E;
	//const wavenumber Q = params.Q;
	const wavenumber &ki = params.ki, &kf = params.kf;
	const energy E = tl::get_energy_transfer(ki, kf);
	const wavenumber Q = ki - kf;

	res.Q_avg[0] = Q * angs;
	res.Q_avg[1] = 0.;
	res.Q_avg[2] = 0.;
	res.Q_avg[3] = E / meV;

	const velocity vi = tl::k2v(ki);
	const velocity vf = tl::k2v(kf);

	const length& lp = params.len_pulse_mono;
	const length& lm = params.len_mono_sample;
	const length& ls = params.len_sample_det;
	const length& slp = params.sig_len_pulse_mono;
	const length& slm = params.sig_len_mono_sample;
	const length& sls = params.sig_len_sample_det;
	const t_time &sp = params.sig_pulse;
	const t_time &sm = params.sig_mono;
	const t_time &sd = params.sig_det;

	const t_time ti = lp / vi;
	const t_time tf = ls / vf;

	tl::log_debug("ki = ", ki, ", kf = ", kf);
	tl::log_debug("vi = ", vi, ", vf = ", vf);
	tl::log_debug("ti = ", ti, ", tf = ", tf);
	tl::log_debug("Q = ", Q, ", E = ", E);

	const mass mn = tl::get_m_n<t_real>();

	// formulas 20 & 21 in [viol14]
	const t_time st = tl::my_units_sqrt<t_time>(sp*sp + sm*sm);
	const t_time stm = tl::my_units_sqrt<t_time>(sd*sd + sm*sm);

	// E formulas 14-18 in [viol14]
	std::vector<std::function<t_real()>> vecEderivs =
	{
		[&]()->t_real { return (-mn*lp*lp/(ti*ti*ti) -mn*ls*ls/(tf*tf*tf)*lm/lp) /meV*sec; },
		[&]()->t_real { return mn*ls*ls/(tf*tf*tf) /meV*sec; },
		[&]()->t_real { return (mn*lp/(ti*ti) + mn*(ls*ls*ls)/(tf*tf*tf)*ti/(lp*lp)*lm/ls) /meV*meter; },
		[&]()->t_real { return -mn*(ls*ls)/(tf*tf*tf) * ti/lp /meV*meter; },
		[&]()->t_real { return -mn*ls/(tf*tf) /meV*meter; },
	};

	std::vector<t_real> vecEsigs = { st/sec, stm/sec, slp/meter, slm/meter, sls/meter };

	// formula 19 in [viol14]
	t_real sigE = std::sqrt(
		std::inner_product(vecEderivs.begin(), vecEderivs.end(), vecEsigs.begin(), t_real(0),
		[](t_real r1, t_real r2)->t_real { return r1 + r2; },
		[](const std::function<t_real()>& f1, t_real r2)->t_real { return f1()*f1()*r2*r2; } ));

	tl::log_debug("sigma E = ", sigE);


	// TODO: implement Q part

	// formulas 10 & 11 in [viol14]
	t_mat matSigSq = tl::diag_matrix({
		st*st /sec/sec,
		stm*stm /sec/sec,
		slp*slp /meter/meter,
		slm*slm /meter/meter,
		sls*sls /meter/meter });
	std::size_t N = matSigSq.size1();
	t_mat matJacobiInstr(N, N, t_real(0));
	for(std::size_t iDeriv=0; iDeriv<vecEderivs.size(); ++iDeriv)
		matJacobiInstr(N-1, iDeriv) = vecEderivs[iDeriv]()/E*meV;
	t_mat matJacobiQE = tl::transform_inv(matSigSq, matJacobiInstr, true);

	return res;
}
