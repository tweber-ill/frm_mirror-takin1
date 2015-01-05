/*
 * Ellipse Dialog
 * @author Tobias Weber
 * @date may-2013, 29-apr-2014
 */

#ifndef __RESO_ELLI_DLG__
#define __RESO_ELLI_DLG__

#include <QtGui/QDialog>
#include <QtCore/QSettings>

#include <vector>

#include "../helper/plotgl.h"
#include "../helper/linalg.h"
#include "../tools/res/ellipse.h"
#include "../tools/res/pop.h"

#include "../ui/ui_ellipses.h"

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>

class EllipseDlg : public QDialog, Ui::EllipseDlg
{ Q_OBJECT
	protected:
		std::vector<QwtPlot*> m_vecPlots;
		std::vector<QwtPlotCurve*> m_vecPlotCurves;
		std::vector<QwtPlotGrid*> m_vecGrid;
		std::vector<QwtPlotPicker*> m_vecPickers;
		std::vector<QwtPlotZoomer*> m_vecZoomers;

		std::vector<Ellipse> m_elliProj;
		std::vector<Ellipse> m_elliSlice;

		std::vector<std::vector<double> > m_vecXCurvePoints;
		std::vector<std::vector<double> > m_vecYCurvePoints;

		QSettings *m_pSettings = 0;

	public:
		EllipseDlg(QWidget* pParent=0, QSettings* pSett=0);
		virtual ~EllipseDlg();

	protected:
		virtual void showEvent(QShowEvent *pEvt);
		virtual void accept();

	protected slots:
		void cursorMoved(const QPointF& pt);

	public slots:
		void SetParams(const ublas::matrix<double>& reso, const ublas::vector<double>& Q_avg);
};

#endif
