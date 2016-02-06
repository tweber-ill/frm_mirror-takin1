/*
 * Scattering Triangle Tool
 * @author tweber
 * @date mar-2014
 * @license GPLv2
 */

#ifndef __TAZ_RECIP_3D__
#define __TAZ_RECIP_3D__

#include <QDialog>
#include "helper/plotgl.h"
#include "tlibs/math/linalg.h"
#include "tlibs/math/geo.h"
#include "tlibs/math/lattice.h"
#include "helper/spacegroup.h"

class Recip3DDlg : public QDialog
{Q_OBJECT
protected:
	PlotGl* m_pPlot;
	double m_dMaxPeaks = 5.;
	double m_dPlaneDistTolerance = 0.01;

public:
	Recip3DDlg(QWidget* pParent, QSettings* = 0);
	virtual ~Recip3DDlg();

	void CalcPeaks(const tl::Lattice<double>& lattice, const tl::Lattice<double>& recip,
					const tl::Plane<double>& planeRLU, const SpaceGroup* pSpaceGroup=0);

	void SetPlaneDistTolerance(double dTol) { m_dPlaneDistTolerance = dTol; }
	void SetMaxPeaks(double dMax) { m_dMaxPeaks = dMax; }

protected:
	void hideEvent(QHideEvent*);
	void showEvent(QShowEvent*);
};


#endif
