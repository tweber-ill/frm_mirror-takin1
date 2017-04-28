/**
 * 3d Brillouin zone drawing
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date apr-2017
 * @license GPLv2
 */

#ifndef __TAZ_BZ_3D__
#define __TAZ_BZ_3D__

#include <QDialog>
#include <QStatusBar>

#include <memory>

#include "libs/globals.h"
#include "libs/plotgl.h"
#include "tlibs/phys/bz.h"


class BZ3DDlg : public QDialog
{Q_OBJECT
protected:
	QSettings *m_pSettings = nullptr;
	QStatusBar *m_pStatus = nullptr;
	std::unique_ptr<PlotGl> m_pPlot;

public:
	BZ3DDlg(QWidget* pParent, QSettings* = 0);
	virtual ~BZ3DDlg() = default;

	void RenderBZ(const tl::Brillouin3D<t_real_glob>& bz);
	

protected:
	virtual void hideEvent(QHideEvent*) override;
	virtual void showEvent(QShowEvent*) override;
	virtual void closeEvent(QCloseEvent*) override;
};

#endif
