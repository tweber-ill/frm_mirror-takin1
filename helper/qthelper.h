/*
 * qt&qwt helpers
 * @author Tobias Weber
 * @date feb-2016
 * @license GPLv2
 */

#ifndef __QT_HELPER_H__
#define __QT_HELPER_H__

#include <vector>
#include <QTableWidget>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>


class QwtPlotWrapper
{
protected:
	QwtPlot *m_pPlot = nullptr;
	std::vector<QwtPlotCurve*> m_vecCurves;
	QwtPlotGrid *m_pGrid = nullptr;
	QwtPlotPicker *m_pPicker = nullptr;
	QwtPlotZoomer *m_pZoomer = nullptr;
	QwtPlotPanner *m_pPanner = nullptr;

public:
	QwtPlotWrapper(QwtPlot *pPlot, unsigned int iNumCurves=1);
	virtual ~QwtPlotWrapper();
	
	QwtPlot* GetPlot() { return m_pPlot; }
	QwtPlotCurve* GetCurve(unsigned int iCurve=0) { return m_vecCurves[iCurve]; }
	QwtPlotZoomer* GetZoomer() { return m_pZoomer; }
	QwtPlotPicker* GetPicker() { return m_pPicker; }
	bool HasTrackerSignal() const;
	
	void SetData(const std::vector<double>& vecX, const std::vector<double>& vecY,
		unsigned int iCurve=0, bool bReplot=1);
};

// ----------------------------------------------------------------------------

extern bool save_table(const char* pcFile, const QTableWidget* pTable);

extern void set_zoomer_base(QwtPlotZoomer *pZoomer,
	const std::vector<double>& vecX, const std::vector<double>& vecY,
	bool bMetaCall=false);


#endif
