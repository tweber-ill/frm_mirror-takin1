/*
 * cooper-nathans calculation
 * @author tweber
 * @date 01-may-2013
 *
 * @desc This is a reimplementation in C++ of the file rc_cnmat.m of the
 *    			rescal5 package by Zinkin, McMorrow, Tennant, Farhi, and Wildes:
 *    			http://www.ill.eu/en/instruments-support/computing-for-science/cs-software/all-software/matlab-ill/rescal-for-matlab/
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
#include <boost/units/base_units/angle/arcminute.hpp>
#include <boost/units/cmath.hpp>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

namespace ublas = boost::numeric::ublas;
namespace units = boost::units;
namespace codata = boost::units::si::constants::codata;


#define one_meV (1e-3 * codata::e * units::si::volts)

static const double SIGMA2FWHM = 2.*sqrt(2.*log(2.));
static const double SIGMA2HWHM = SIGMA2FWHM/2.;

static const units::quantity<units::si::length> angstrom = 1e-10 * units::si::meter;

static const double KSQ2E = (codata::hbar*codata::hbar / (2.*codata::m_n)) / one_meV / (angstrom*angstrom);
static const double E2KSQ = 1./KSQ2E;


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

	// resolution volume stuff
	bool bConstMon;		// constant monitor or time?
	double dmono_refl;
	double dana_effic;
};

struct CNResults
{
	bool bOk;
	std::string strErr;

	units::quantity<units::si::plane_angle> thetaa, thetam, thetas;

	units::quantity<units::si::plane_angle> twotheta;
	units::quantity<units::si::plane_angle> angle_ki_Q;
	units::quantity<units::si::plane_angle> angle_kf_Q;

	ublas::matrix<double> reso;
	ublas::vector<double> Q_avg;
	double dR0;				// resolution volume in 1/A^3 * meV
	double dR0_vi, dR0_vf;
};


extern ublas::matrix<double> gauss_int(const ublas::matrix<double>& mat, unsigned int iIdx);
extern CNResults calc_cn(CNParams& cn);
extern bool calc_cn_angles(CNParams& cn, CNResults& res);
extern void calc_cn_vol(CNParams& cn, CNResults& res);

#endif
