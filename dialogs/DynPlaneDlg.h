/*
 * Dynamic Plane Dialog
 * @author tweber
 * @date 2013, jan-2015
 * @copyright GPLv2
 */

#ifndef __DYN_PLANE_DLG_H__
#define __DYN_PLANE_DLG_H__

#include <QtGui/QDialog>
#include <QtCore/QSettings>
#include "../ui/ui_dyn_plane.h"
#include "RecipParamDlg.h"

#include <vector>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_picker.h>

class DynPlaneDlg : public QDialog, Ui::DynPlaneDlg
{ Q_OBJECT
protected:
	QSettings *m_pSettings = nullptr;
	std::vector<double> m_vecQ, m_vecE;

	QwtPlotCurve *m_pCurve = nullptr;
	QwtPlotGrid *m_pGrid = nullptr;
	QwtPlotPicker *m_pPicker = nullptr;

	double m_d2Theta = 0.;
	double m_dEi = 5., m_dEf = 5.;

protected:
	virtual void showEvent(QShowEvent *pEvt) override;
	virtual void accept() override;

protected slots:
	void cursorMoved(const QPointF& pt);
	void Calc();
	void FixedKiKfToggled();

public slots:
	void RecipParamsChanged(const RecipParams&);


public:
	DynPlaneDlg(QWidget* pParent = nullptr, QSettings *pSettings = nullptr);
	virtual ~DynPlaneDlg();
};

#endif
