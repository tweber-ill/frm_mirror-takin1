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



struct CNParams
{
	// monochromator
	units::quantity<units::si::length> mono_d;
	units::quantity<units::si::plane_angle> mono_mosaic;
	double dmono_sense=-1.;

	// analyser
	units::quantity<units::si::length> ana_d;
	units::quantity<units::si::plane_angle> ana_mosaic;
	double dana_sense=-1.;

	// sample
	units::quantity<units::si::plane_angle> sample_mosaic;
	units::quantity<units::si::length> sample_lattice[3];
	units::quantity<units::si::plane_angle> sample_angles[3];
	double dsample_sense=1.;

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
	units::quantity<units::si::energy> E;

	units::quantity<units::si::plane_angle> thetaa, thetam;
	units::quantity<units::si::plane_angle> twotheta;

	units::quantity<units::si::plane_angle> angle_ki_Q;
	units::quantity<units::si::plane_angle> angle_kf_Q;

	// resolution volume stuff
	double dmono_refl;
	double dana_effic;

	bool bCalcR0 = 1;
};

struct CNResults
{
	bool bOk;
	std::string strErr;

	ublas::matrix<double> reso;		// quadratic part of quadric
	ublas::vector<double> reso_v;	// linear part of quadric
	double reso_s;					// constant part of quadric

	ublas::vector<double> Q_avg;
	double dR0;						// resolution prefactor
	double dResVol;					// resolution volume in 1/A^3 * meV

	double dBraggFWHMs[4];
};


extern CNResults calc_cn(const CNParams& cn);

#endif
