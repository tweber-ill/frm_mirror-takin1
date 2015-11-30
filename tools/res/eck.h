/*
 * implementation of the eckold-sobolev algo
 * 
 * @author tweber
 * @date feb-2015
 * @license GPLv2
 *
 * @desc algorithm: [eck14] G. Eckold and O. Sobolev, NIM A 752, pp. 54-64 (2014)
 */

#ifndef __RESO_ECK_H__
#define __RESO_ECK_H__

#include "pop.h"


struct EckParams : public PopParams
{
	units::quantity<units::si::plane_angle> mono_mosaic_v;
	units::quantity<units::si::plane_angle> ana_mosaic_v;

	units::quantity<units::si::length> pos_x, pos_y, pos_z;
};


extern CNResults calc_eck(const EckParams& eck);


#endif
