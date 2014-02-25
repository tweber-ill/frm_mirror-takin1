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
	bool bChangedTwoTheta=0;
	bool bChangedAnaTwoTheta=0;
	bool bChangedMonoTwoTheta=0;

	double dTwoTheta;
	double dAnaTwoTheta;
	double dMonoTwoTheta;
};

#endif
