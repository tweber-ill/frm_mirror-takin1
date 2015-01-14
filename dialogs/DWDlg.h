/*
 * Debye-Waller Dialog
 * @author tweber
 * @date 2013, jan-2015
 * @copyright GPLv2
 */

#ifndef __DWDLG_H__
#define __DWDLG_H__

#include <QtGui/QDialog>
#include <QtCore/QSettings>
#include "../ui/ui_dw.h"

#include <vector>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_picker.h>

class DWDlg : public QDialog, Ui::DWDlg
{ Q_OBJECT
protected:
	QSettings *m_pSettings = nullptr;
	std::vector<double> m_vecQ, m_vecDeb;

	QwtPlotCurve *m_pCurve = nullptr;
	QwtPlotGrid *m_pGrid = nullptr;
	QwtPlotPicker *m_pPicker = nullptr;

protected:
	virtual void showEvent(QShowEvent *pEvt) override;
	virtual void accept() override;

protected slots:
	void cursorMoved(const QPointF& pt);
	void Calc();

public:
	DWDlg(QWidget* pParent = nullptr, QSettings *pSettings = nullptr);
	virtual ~DWDlg();
};

#endif
