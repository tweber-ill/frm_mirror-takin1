/*
 * Scattering Triangle tool
 * @author tweber
 * @date feb-2014
 */

#ifndef __TASOPTIONS_H__
#define __TASOPTIONS_H__

#include "helper/linalg.h"
#include <QtGui/QWheelEvent>

static inline ublas::vector<double> qpoint_to_vec(const QPointF& pt)
{
	ublas::vector<double> vec(2);
	vec[0] = double(pt.x());
	vec[1] = double(pt.y());

	return vec;
}

static inline QPointF vec_to_qpoint(const ublas::vector<double>& vec)
{
	if(vec.size() < 2)
		return QPointF(0., 0.);

	return QPointF(vec[0], vec[1]);
}

struct TriangleOptions
{
	bool bChangedTheta = 0;
	bool bChangedTwoTheta = 0;
	bool bChangedAnaTwoTheta = 0;
	bool bChangedMonoTwoTheta = 0;

	bool bChangedMonoD = 0;
	bool bChangedAnaD = 0;

	bool bChangedAngleKiVec0 = 0;


	double dTheta;
	double dTwoTheta;
	double dAnaTwoTheta;
	double dMonoTwoTheta;

	double dMonoD;
	double dAnaD;

	double dAngleKiVec0;
};

struct CrystalOptions
{
	bool bChangedLattice = 0;
	bool bChangedLatticeAngles = 0;
	bool bChangedSpacegroup = 0;
	bool bChangedPlane1 = 0;
	bool bChangedPlane2 = 0;

	double dLattice[3];
	double dLatticeAngles[3];
	std::string strSpacegroup;
	double dPlane1[3];
	double dPlane2[3];
};


#endif
