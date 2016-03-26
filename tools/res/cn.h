/*
 * cooper-nathans calculation
 * @author tweber
 * @date 01-may-2013
 * @license GPLv2
 *
 * @desc This is a reimplementation in C++ of the file rc_cnmat.m of the
 *		rescal5 package by Zinkin, McMorrow, Tennant, Farhi, and Wildes:
 *		http://www.ill.eu/en/instruments-support/computing-for-science/cs-software/all-software/matlab-ill/rescal-for-matlab/
 */

#ifndef __CN_H__
#define __CN_H__

#include "tlibs/math/neutrons.hpp"

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

namespace ublas = boost::numeric::ublas;
namespace units = boost::units;
namespace codata = boost::units::si::constants::codata;

using t_real_reso = double;


struct CNParams
{
	// monochromator
	tl::t_length_si<t_real_reso> mono_d;
	tl::t_angle_si<t_real_reso> mono_mosaic;
	t_real_reso dmono_sense = -1.;

	// analyser
	tl::t_length_si<t_real_reso> ana_d;
	tl::t_angle_si<t_real_reso> ana_mosaic;
	t_real_reso dana_sense = -1.;

	// sample
	tl::t_angle_si<t_real_reso> sample_mosaic;
	tl::t_length_si<t_real_reso> sample_lattice[3];
	tl::t_angle_si<t_real_reso> sample_angles[3];
	t_real_reso dsample_sense = 1.;

	// collimators
	tl::t_angle_si<t_real_reso> coll_h_pre_mono;
	tl::t_angle_si<t_real_reso> coll_h_pre_sample;
	tl::t_angle_si<t_real_reso> coll_h_post_sample;
	tl::t_angle_si<t_real_reso> coll_h_post_ana;
	tl::t_angle_si<t_real_reso> coll_v_pre_mono;
	tl::t_angle_si<t_real_reso> coll_v_pre_sample;
	tl::t_angle_si<t_real_reso> coll_v_post_sample;
	tl::t_angle_si<t_real_reso> coll_v_post_ana;

	tl::t_wavenumber_si<t_real_reso> ki, kf, Q;
	tl::t_energy_si<t_real_reso> E;

	tl::t_angle_si<t_real_reso> thetaa, thetam;
	tl::t_angle_si<t_real_reso> twotheta;

	tl::t_angle_si<t_real_reso> angle_ki_Q;
	tl::t_angle_si<t_real_reso> angle_kf_Q;

	// resolution volume stuff
	t_real_reso dmono_refl;
	t_real_reso dana_effic;

	bool bCalcR0 = 1;
};

struct CNResults
{
	bool bOk;
	std::string strErr;

	ublas::matrix<t_real_reso> reso;		// quadratic part of quadric
	ublas::vector<t_real_reso> reso_v;		// linear part of quadric
	t_real_reso reso_s;				// constant part of quadric

	ublas::vector<t_real_reso> Q_avg;
	t_real_reso dR0;				// resolution prefactor
	t_real_reso dResVol;				// resolution volume in 1/A^3 * meV

	t_real_reso dBraggFWHMs[4];
};


extern CNResults calc_cn(const CNParams& cn);

#endif
