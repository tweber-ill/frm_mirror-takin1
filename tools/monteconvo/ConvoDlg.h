/*
 * monte carlo convolution tool
 * @author tweber
 * @date aug-2015
 * @license GPLv2
 */

#ifndef __MCONVO_GUI_H__
#define __MCONVO_GUI_H__

#include <QDialog>
#include <QSettings>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_picker.h>

#include "SqwParamDlg.h"
#include "ui/ui_monteconvo.h"


class ConvoDlg : public QDialog, Ui::ConvoDlg
{ Q_OBJECT
private:

protected:
	QSettings *m_pSett = nullptr;
	SqwParamDlg *m_pSqwParamDlg = nullptr;

	QwtPlotCurve *m_pCurve, *m_pPoints = nullptr;
	QwtPlotGrid *m_pGrid = nullptr;
	QwtPlotPicker *m_pPicker = nullptr;

	std::vector<double> m_vecQ, m_vecS;

protected slots:
	void showSqwParamDlg();

	void browseCrysFiles();
	void browseResoFiles();
	void browseSqwFiles();

	void SaveResult();

	void Start();

public:
	ConvoDlg(QWidget* pParent=0, QSettings* pSett=0);
	virtual ~ConvoDlg();
};

#endif
