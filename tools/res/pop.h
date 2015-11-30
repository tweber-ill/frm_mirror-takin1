/*
 * popovici calculation
 * @author tweber
 * @date 08-may-2013
 * @license GPLv2
 *
 * @desc This is a reimplementation in C++ of the file rc_popma.m of the
 *		rescal5 package by Zinkin, McMorrow, Tennant, Farhi, and Wildes:
 *		http://www.ill.eu/en/instruments-support/computing-for-science/cs-software/all-software/matlab-ill/rescal-for-matlab/
 */

#ifndef __POP_H__
#define __POP_H__

#include "cn.h"

struct PopParams : public CNParams
{
	units::quantity<units::si::length> mono_w;
	units::quantity<units::si::length> mono_h;
	units::quantity<units::si::length> mono_thick;
	units::quantity<units::si::length> mono_curvh;
	units::quantity<units::si::length> mono_curvv;
	bool bMonoIsCurvedH=0, bMonoIsCurvedV=0;
	bool bMonoIsOptimallyCurvedH=0, bMonoIsOptimallyCurvedV=0;
	unsigned int mono_numtiles_v, mono_numtiles_h;

	units::quantity<units::si::length> ana_w;
	units::quantity<units::si::length> ana_h;
	units::quantity<units::si::length> ana_thick;
	units::quantity<units::si::length> ana_curvh;
	units::quantity<units::si::length> ana_curvv;
	bool bAnaIsCurvedH=0, bAnaIsCurvedV=0;
	bool bAnaIsOptimallyCurvedH=0, bAnaIsOptimallyCurvedV=0;
	unsigned int ana_numtiles_v, ana_numtiles_h;

	bool bSampleCub=1;
	units::quantity<units::si::length> sample_w_q;
	units::quantity<units::si::length> sample_w_perpq;
	units::quantity<units::si::length> sample_h;

	bool bSrcRect=1;
	units::quantity<units::si::length> src_w;
	units::quantity<units::si::length> src_h;

	bool bDetRect=1;
	units::quantity<units::si::length> det_w;
	units::quantity<units::si::length> det_h;

	bool bGuide=0;
	units::quantity<units::si::plane_angle> guide_div_h;
	units::quantity<units::si::plane_angle> guide_div_v;

	units::quantity<units::si::length> dist_mono_sample;
	units::quantity<units::si::length> dist_sample_ana;
	units::quantity<units::si::length> dist_ana_det;
	units::quantity<units::si::length> dist_src_mono;
};


extern CNResults calc_pop(const PopParams& pop);

#endif
