/*
 * cooper-nathans calculation
 * @author tweber
 * @date 01-may-2013
 *
 * @desc This is a reimplementation in C++ of the file rc_cnmat.m of the
 *    			rescal5 package by Zinkin, McMorrow, Tennant, Farhi, and Wildes.
 */

#ifndef __CN_H__
#define __CN_H__

#include <boost/units/unit.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/dimensionless_quantity.hpp>
#include <boost/units/physical_dimensions.hpp>
#include <boost/units/systems/si.hpp>
#include <boost/units/systems/angle/degrees.hpp>

#include <boost/units/systems/si/codata/universal_constants.hpp>
#include <boost/units/systems/si/codata/neutron_constants.hpp>
#include <boost/units/systems/si/codata/electromagnetic_constants.hpp>
#include <boost/units/systems/si/codata/physico-chemical_constants.hpp>
#include <boost/units/base_units/metric/angstrom.hpp>
#include <boost/units/cmath.hpp>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>

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

	// scattering triangle
	bool bki_fix;		// ki or kf fixed?
	units::quantity<units::si::wavenumber> ki, kf, Q;
	units::quantity<units::si::energy> E;		// Ei - Ef
};

struct CNResults
{
	bool bOk;
	std::string strErr;

	ublas::matrix<double> reso;
};


extern CNResults calc_cn(CNParams& cn);

#endif
