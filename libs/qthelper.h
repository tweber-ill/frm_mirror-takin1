/*
 * qt&qwt helpers
 * @author Tobias Weber
 * @date feb-2016
 * @license GPLv2
 */

#ifndef __QT_HELPER_H__
#define __QT_HELPER_H__

#include <vector>
#include <type_traits>
#include <QTableWidget>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>

using t_real_qwt = double;		// qwt's intrinsic value type


class QwtPlotWrapper
{
protected:
	QwtPlot *m_pPlot = nullptr;
	std::vector<QwtPlotCurve*> m_vecCurves;
	QwtPlotGrid *m_pGrid = nullptr;
	QwtPlotPicker *m_pPicker = nullptr;
	QwtPlotZoomer *m_pZoomer = nullptr;
	QwtPlotPanner *m_pPanner = nullptr;

	bool m_bHasDataPtrs = 1;
	std::vector<std::pair<const std::vector<t_real_qwt>*, const std::vector<t_real_qwt>*>> m_vecDataPtrs;

public:
	QwtPlotWrapper(QwtPlot *pPlot, unsigned int iNumCurves=1, bool bNoTrackerSignal=0);
	virtual ~QwtPlotWrapper();

	QwtPlot* GetPlot() { return m_pPlot; }
	QwtPlotCurve* GetCurve(unsigned int iCurve=0) { return m_vecCurves[iCurve]; }
	QwtPlotZoomer* GetZoomer() { return m_pZoomer; }
	QwtPlotPicker* GetPicker() { return m_pPicker; }
	bool HasTrackerSignal() const;

	void SetData(const std::vector<t_real_qwt>& vecX, const std::vector<t_real_qwt>& vecY,
		unsigned int iCurve=0, bool bReplot=1, bool bCopy=0);

	void SavePlot() const;
};


// ----------------------------------------------------------------------------

template<typename t_real, bool bSetDirectly=std::is_same<t_real, t_real_qwt>::value>
struct set_qwt_data
{
	void operator()(QwtPlotWrapper& plot, const std::vector<t_real>& vecX, const std::vector<t_real>& vecY,
		unsigned int iCurve=0, bool bReplot=1) {}
};

// same types -> set data directly
template<typename t_real>
struct set_qwt_data<t_real, 1>
{
	void operator()(QwtPlotWrapper& plot, const std::vector<t_real>& vecX, const std::vector<t_real>& vecY,
		unsigned int iCurve=0, bool bReplot=1)
	{
		plot.SetData(vecX, vecY, iCurve, bReplot, 0);
	}
};

// different types -> copy & convert data first
template<typename t_real>
struct set_qwt_data<t_real, 0>
{
	void operator()(QwtPlotWrapper& plot, const std::vector<t_real>& vecX, const std::vector<t_real>& vecY,
		unsigned int iCurve=0, bool bReplot=1)
	{
		std::vector<t_real_qwt> vecNewX, vecNewY;
		vecNewX.reserve(vecX.size());
		vecNewY.reserve(vecY.size());

		for(t_real d : vecX) vecNewX.push_back(d);
		for(t_real d : vecY) vecNewY.push_back(d);

		plot.SetData(vecNewX, vecNewY, iCurve, bReplot, 1);
	}
};

// ----------------------------------------------------------------------------


extern bool save_table(const char* pcFile, const QTableWidget* pTable);

extern void set_zoomer_base(QwtPlotZoomer *pZoomer,
	const std::vector<t_real_qwt>& vecX, const std::vector<t_real_qwt>& vecY,
	bool bMetaCall=false);

#endif
