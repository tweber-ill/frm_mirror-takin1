/*
 * Scattering Triangle Tool
 * @author tweber
 * @date mar-2014
 */

#ifndef __TAZ_RECIP_3D__
#define __TAZ_RECIP_3D__

#include <QtGui/QDialog>
#include "helper/plotgl.h"
#include "helper/linalg.h"
#include "helper/geo.h"
#include "helper/lattice.h"
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

	void CalcPeaks(const Lattice<double>& lattice,
					const Lattice<double>& recip, const Lattice<double>& recip_unrot,
					const Plane<double>& plane,
					const SpaceGroup* pSpaceGroup=0);

	void SetPlaneDistTolerance(double dTol) { m_dPlaneDistTolerance = dTol; }
	void SetMaxPeaks(double dMax) { m_dMaxPeaks = dMax; }

protected:
	void hideEvent(QHideEvent*);
	void showEvent(QShowEvent*);
};


#endif
