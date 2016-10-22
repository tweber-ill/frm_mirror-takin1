/*
 * 3d unit cell drawing
 * @author tweber
 * @date oct-2016
 * @license GPLv2
 */

#ifndef __TAZ_REAL_3D__
#define __TAZ_REAL_3D__

#include <QDialog>
#include "libs/plotgl.h"
#include "tlibs/math/linalg.h"
#include "libs/spacegroups/latticehelper.h"
#include "libs/globals.h"

class Real3DDlg : public QDialog
{Q_OBJECT
protected:
	PlotGl* m_pPlot;

public:
	Real3DDlg(QWidget* pParent, QSettings* = 0);
	virtual ~Real3DDlg();

	void CalcPeaks(const LatticeCommon<t_real_glob>& realcommon);

protected:
	void hideEvent(QHideEvent*);
	void showEvent(QShowEvent*);
};

#endif
