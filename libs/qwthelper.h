/**
 * qwt helpers
 * @author Tobias Weber
 * @date feb-2016
 * @license GPLv2
 */

#ifndef __QWT_HELPER_H__
#define __QWT_HELPER_H__

#include <vector>
#include <string>
#include <type_traits>

#include <qwt_plot.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_raster_data.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_color_map.h>

#include "tlibs/string/string.h"


using t_real_qwt = double;		// qwt's intrinsic value type


class MyQwtRasterData : public QwtRasterData
{
protected:
	std::unique_ptr<t_real_qwt[]> m_pData;
	std::size_t m_iW=0, m_iH=0;
	t_real_qwt m_dXRange[2], m_dYRange[2], m_dZRange[2];

public:
	void Init(std::size_t iW, std::size_t iH)
	{
		if(m_iW==iW && m_iH==iH) return;

		m_iW = iW;
		m_iH = iH;

		m_pData.reset(new t_real_qwt[iW*iH]);
		std::fill(m_pData.get(), m_pData.get()+m_iW*m_iH, t_real_qwt(0));
	}

	MyQwtRasterData(std::size_t iW=0, std::size_t iH=0)
	{
		Init(iW, iH);
	}

	void SetXRange(t_real_qwt dMin, t_real_qwt dMax);
	void SetYRange(t_real_qwt dMin, t_real_qwt dMax);
	void SetZRange(t_real_qwt dMin, t_real_qwt dMax);
	void SetZRange();	// automatically determined range

	t_real_qwt GetZMin() const { return m_dZRange[0]; }
	t_real_qwt GetZMax() const { return m_dZRange[1]; }

	std::size_t GetWidth() const { return m_iW; }
	std::size_t GetHeight() const { return m_iH; }

	void SetPixel(std::size_t iX, std::size_t iY, t_real_qwt dVal)
	{
		if(iX<m_iW && iY<m_iH)
			m_pData[iY*m_iW + iX] = dVal;
	}

	t_real_qwt GetPixel(std::size_t iX, std::size_t iY) const
	{
		if(!m_pData) return t_real_qwt(0);
		return m_pData[iY*m_iW + iX];
	}

	virtual t_real_qwt value(t_real_qwt dx, t_real_qwt dy) const override;
};


// ----------------------------------------------------------------------------


class QwtPlotWrapper : public QObject
{ Q_OBJECT
protected:
	QwtPlot *m_pPlot = nullptr;
	QwtPlotGrid *m_pGrid = nullptr;
	QwtPlotPicker *m_pPicker = nullptr;
	QwtPlotZoomer *m_pZoomer = nullptr;
	QwtPlotPanner *m_pPanner = nullptr;

	std::vector<QwtPlotCurve*> m_vecCurves;	// 1d plots
	QwtPlotSpectrogram *m_pSpec = nullptr;	// 2d plot

	bool m_bHasDataPtrs = 1;
	std::vector<std::pair<const std::vector<t_real_qwt>*, const std::vector<t_real_qwt>*>> m_vecDataPtrs;
	MyQwtRasterData *m_pRaster = nullptr;

public:
	QwtPlotWrapper(QwtPlot *pPlot, unsigned int iNumCurves=1,
		bool bNoTrackerSignal=0, bool bUseSpline=0, bool bUseSpectrogram=0);
	virtual ~QwtPlotWrapper();

	QwtPlot* GetPlot() { return m_pPlot; }
	QwtPlotCurve* GetCurve(unsigned int iCurve=0) { return m_vecCurves[iCurve]; }
	QwtPlotZoomer* GetZoomer() { return m_pZoomer; }
	QwtPlotPicker* GetPicker() { return m_pPicker; }
	MyQwtRasterData* GetRaster() { return m_pRaster; }
	bool HasTrackerSignal() const;

	void SetData(const std::vector<t_real_qwt>& vecX, const std::vector<t_real_qwt>& vecY,
		unsigned int iCurve=0, bool bReplot=1, bool bCopy=0);

	void SavePlot() const;

public slots:
	void setAxisTitle(int iAxis, const QString& str);
	void scaleColorBar();
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


extern void set_zoomer_base(QwtPlotZoomer *pZoomer,
	t_real_qwt dL, t_real_qwt dR, t_real_qwt dT, t_real_qwt dB,
	bool bMetaCall = false);

extern void set_zoomer_base(QwtPlotZoomer *pZoomer,
	const std::vector<t_real_qwt>& vecX, const std::vector<t_real_qwt>& vecY,
	bool bMetaCall = false);


#endif
