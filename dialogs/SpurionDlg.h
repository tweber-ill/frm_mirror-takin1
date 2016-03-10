/*
 * Spurion Dialog
 * @author Tobias Weber
 * @date 26-may-2014
 * @license GPLv2
 */

#ifndef __SPURION_DLG_H__
#define __SPURION_DLG_H__

#include <QDialog>
#include <QSettings>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <vector>

#include "ui/ui_spurions.h"
#include "RecipParamDlg.h"


class SpurionDlg : public QDialog, Ui::SpurionDlg
{ Q_OBJECT
	protected:
		QSettings *m_pSettings = 0;
		double m_dEi=0., m_dEf=0.;

		std::vector<double> m_vecQ, m_vecE;

		QwtPlotCurve *m_pBraggCurve = nullptr;
		QwtPlotGrid *m_pBraggGrid = nullptr;
		QwtPlotPicker *m_pBraggPicker = nullptr;
		QwtPlotZoomer* m_pZoomerBragg = nullptr;
		QwtPlotPanner* m_pPannerBragg = nullptr;

	public:
		SpurionDlg(QWidget* pParent=0, QSettings *pSett=0);
		virtual ~SpurionDlg();

	protected slots:
		void ChangedKiKfMode();
		void Calc();

		void CalcInel();
		void CalcBragg();

		void cursorMoved(const QPointF& pt);
		void paramsChanged(const RecipParams& parms);

	protected:
		virtual void showEvent(QShowEvent *pEvt);
		virtual void accept();
};


#endif
