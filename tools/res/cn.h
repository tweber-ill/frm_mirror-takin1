/*
 * cooper-nathans calculation
 * @author tweber
 * @date 01-may-2013
 * @copyright GPLv2
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



struct CNParams
{
	// monochromator
	units::quantity<units::si::length> mono_d;
	units::quantity<units::si::plane_angle> mono_mosaic;
	double dmono_sense;

	// analyser
	units::quantity<units::si::length> ana_d;
	units::quantity<units::si::plane_angle> ana_mosaic;
	double dana_sense;

	// sample
	units::quantity<units::si::plane_angle> sample_mosaic;
	units::quantity<units::si::length> sample_lattice[3];
	units::quantity<units::si::plane_angle> sample_angles[3];
	double dsample_sense;

	// collimators
	units::quantity<units::si::plane_angle> coll_h_pre_mono;
	units::quantity<units::si::plane_angle> coll_h_pre_sample;
	units::quantity<units::si::plane_angle> coll_h_post_sample;
	units::quantity<units::si::plane_angle> coll_h_post_ana;
	units::quantity<units::si::plane_angle> coll_v_pre_mono;
	units::quantity<units::si::plane_angle> coll_v_pre_sample;
	units::quantity<units::si::plane_angle> coll_v_post_sample;
	units::quantity<units::si::plane_angle> coll_v_post_ana;

	units::quantity<units::si::wavenumber> ki, kf, Q;

	bool bCalcE = 1;
	units::quantity<units::si::energy> E;

	bool bCalcMonoAnaAngles = 1;
	bool bCalcSampleAngles = 1;
	units::quantity<units::si::plane_angle> thetaa, thetam, thetas;

	units::quantity<units::si::plane_angle> twotheta;
	units::quantity<units::si::plane_angle> angle_ki_Q;
	units::quantity<units::si::plane_angle> angle_kf_Q;

	ublas::vector<double> vec0, vec1, vecUp;

	// resolution volume stuff
	bool bConstMon;			// constant monitor or time?
	double dmono_refl;
	double dana_effic;
	
	bool bCalcR0 = 0;
};

struct CNResults
{
	bool bOk;
	std::string strErr;

	ublas::matrix<double> reso;
	ublas::vector<double> Q_avg;
	double dR0;				// resolution prefactors
	double dResVol;			// resolution volume in 1/A^3 * meV

	double dBraggFWHMs[4];
};


extern CNResults calc_cn(CNParams& cn);
extern bool calc_tas_angles(CNParams& cn, CNResults& res);
extern void calc_bragg_widths(CNParams& cn, CNResults& res);

#endif
