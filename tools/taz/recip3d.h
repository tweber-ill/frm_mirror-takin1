/*
 * Scattering Triangle Tool
 * @author tweber
 * @date mar-2014
 */

#ifndef __TAZ_RECIP_3D__
#define __TAZ_RECIP_3D__

#include <QtGui/QDialog>
#include "plot/plotgl.h"
#include "helper/linalg.h"
#include "helper/lattice.h"

class Recip3DDlg : public QDialog
{Q_OBJECT
protected:
	PlotGl* m_pPlot;
	double m_dMaxPeaks = 5.;
	double m_dPlaneDistTolerance = 0.01;

public:
	Recip3DDlg(QWidget* pParent);
	virtual ~Recip3DDlg();

	void CalcPeaks(const Lattice& lattice,
					const Lattice& recip, const Lattice& recip_unrot,
					const Plane<double>& plane);

	void SetPlaneDistTolerance(double dTol) { m_dPlaneDistTolerance = dTol; }
	void SetMaxPeaks(double dMax) { m_dMaxPeaks = dMax; }

protected slots:
};


#endif
