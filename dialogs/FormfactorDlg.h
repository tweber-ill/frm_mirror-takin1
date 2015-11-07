/*
 * Form Factor & Scattering Length Dialog
 * @author tweber
 * @date nov-2015
 * @license GPLv2
 */

#ifndef __TAKIN_FF_DLG_H__
#define __TAKIN_FF_DLG_H__

#include <QDialog>
#include <QSettings>
#include "ui/ui_formfactors.h"

#include <vector>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_picker.h>
#include <qwt_legend.h>


class FormfactorDlg : public QDialog, Ui::FormFactorDlg
{ Q_OBJECT
protected:
	QSettings *m_pSettings = nullptr;

	// form factors
	std::vector<double> m_vecQ, m_vecFF;

	QwtPlotCurve *m_pCurve = nullptr;
	QwtPlotGrid *m_pGrid = nullptr;
	QwtPlotPicker *m_pPicker = nullptr;

	// scattering lengths
	std::vector<double> m_vecElem, m_vecSc;

	QwtPlotCurve *m_pCurveSc = nullptr;
	QwtPlotGrid *m_pGridSc = nullptr;
	QwtPlotPicker *m_pPickerSc = nullptr;


protected:
	virtual void closeEvent(QCloseEvent* pEvt) override;
	void SetupAtoms();

protected slots:
	void SearchAtom(const QString& qstr);
	void AtomSelected(QListWidgetItem *pItem, QListWidgetItem *pItemPrev);

	void PlotScatteringLengths();

	void cursorMoved(const QPointF& pt);

public:
	FormfactorDlg(QWidget* pParent = nullptr, QSettings *pSettings = nullptr);
	virtual ~FormfactorDlg();
};

#endif
