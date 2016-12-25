/**
 * qwt helpers
 * @author Tobias Weber
 * @date feb-2016
 * @license GPLv2
 */

#include "qwthelper.h"
#include "globals.h"
#include "globals_qt.h"
#include "tlibs/math/math.h"
#include "tlibs/string/string.h"
#include "tlibs/helper/misc.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <memory>

#include <qwt_picker_machine.h>
#include <qwt_plot_canvas.h>
#include <qwt_curve_fitter.h>
#include <qwt_scale_widget.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>


class MyQwtPlotZoomer : public QwtPlotZoomer
{ /*Q_OBJECT*/
protected:
	virtual void widgetMouseReleaseEvent(QMouseEvent *pEvt) override
	{
		// filter out middle button event which is already used for panning
		if(pEvt->button() & Qt::MiddleButton)
			return;

		QwtPlotZoomer::widgetMouseReleaseEvent(pEvt);
	}

public:
	template<class t_widget_or_canvas>
	explicit MyQwtPlotZoomer(t_widget_or_canvas* ptr) : QwtPlotZoomer(ptr)
	{
		QwtPlotZoomer::setMaxStackDepth(-1);
		QwtPlotZoomer::setEnabled(1);
	}

	virtual ~MyQwtPlotZoomer() {}

/*public slots:
	virtual void setZoomBase(const QRectF& rect) override
	{
		QwtPlotZoomer::setZoomBase(rect);
	}*/
};


// ----------------------------------------------------------------------------


class MyQwtPlotPicker : public QwtPlotPicker
{
protected:
	QwtPlotWrapper *m_pPlotWrap = nullptr;

	virtual void widgetKeyPressEvent(QKeyEvent *pEvt) override
	{
		const int iKey = pEvt->key();
		//std::cout << "Plot key: " <<  iKey << std::endl;
		if(iKey == Qt::Key_S)
			m_pPlotWrap->SavePlot();
		else
			QwtPlotPicker::widgetKeyPressEvent(pEvt);
	}

public:
	MyQwtPlotPicker(QwtPlotWrapper *pPlotWrap, bool bNoTrackerSignal=0) :
		QwtPlotPicker(pPlotWrap->GetPlot()->xBottom, pPlotWrap->GetPlot()->yLeft,
#if QWT_VER<6
		QwtPlotPicker::PointSelection,
#endif
		QwtPlotPicker::NoRubberBand,
		bNoTrackerSignal ? QwtPicker::AlwaysOn : QwtPicker::AlwaysOff,
		pPlotWrap->GetPlot()->canvas()),
		m_pPlotWrap(pPlotWrap)
	{
#if QWT_VER>=6
		QwtPlotPicker::setStateMachine(new QwtPickerTrackerMachine());
		//connect(this, SIGNAL(moved(const QPointF&)), this, SLOT(cursorMoved(const QPointF&)));
#endif
		QwtPlotPicker::setEnabled(1);
	}

	virtual ~MyQwtPlotPicker() {}
};


// ----------------------------------------------------------------------------


QwtPlotWrapper::QwtPlotWrapper(QwtPlot *pPlot,
	unsigned int iNumCurves, bool bNoTrackerSignal, bool bUseSpline, bool bUseSpectrogram)
	: m_pPlot(pPlot)
{
	QPen penGrid;
	penGrid.setColor(QColor(0x99,0x99,0x99));
	penGrid.setStyle(Qt::DashLine);

	QPen penCurve;
	penCurve.setColor(QColor(0,0,0x99));
	penCurve.setWidth(2);

	QColor colorBck(240, 240, 240, 255);
	m_pPlot->setCanvasBackground(colorBck);

	if(bUseSpectrogram)
	{
		m_pSpec = new QwtPlotSpectrogram();
		m_pSpec->setRenderHint(QwtPlotItem::RenderAntialiased, true);
		m_pSpec->setDisplayMode(QwtPlotSpectrogram::ImageMode, 1);

		auto create_colmap = []() -> QwtLinearColorMap*
		{
			QwtLinearColorMap *pCol = new QwtLinearColorMap();
			pCol->setColorInterval(QColor(0x00, 0x00, 0xff), QColor(0xff, 0x00, 0x00));
			return pCol;
		};
		m_pSpec->setColorMap(create_colmap());

		t_real_qwt dCBMin=0., dCBMax=1.;
		m_pPlot->enableAxis(QwtPlot::yRight);
		QwtScaleWidget *pRightAxis = m_pPlot->axisWidget(QwtPlot::yRight);
		pRightAxis->setColorBarEnabled(1);
		pRightAxis->setColorMap(QwtInterval(dCBMin, dCBMax), create_colmap());
		m_pPlot->setAxisScale(QwtPlot::yRight, dCBMin, dCBMax);

		m_pRaster = new MyQwtRasterData();
		m_pSpec->setData(m_pRaster);

		m_pSpec->attach(m_pPlot);
	}
	else
	{
		m_vecCurves.reserve(iNumCurves);
		for(unsigned int iCurve=0; iCurve<iNumCurves; ++iCurve)
		{
			QwtPlotCurve* pCurve = new QwtPlotCurve();
			pCurve->setPen(penCurve);
			pCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

			if(bUseSpline)
			{
				pCurve->setCurveFitter(new QwtSplineCurveFitter());
				((QwtSplineCurveFitter*)pCurve->curveFitter())->setFitMode(
					QwtSplineCurveFitter::/*Parametric*/Spline);
				pCurve->setCurveAttribute(QwtPlotCurve::Fitted);
			}

			pCurve->attach(m_pPlot);
			m_vecCurves.push_back(pCurve);
		}

		m_pGrid = new QwtPlotGrid();
		m_pGrid->setPen(penGrid);
		m_pGrid->attach(m_pPlot);
	}

	m_pPanner = new QwtPlotPanner(m_pPlot->canvas());
	m_pPanner->setMouseButton(Qt::MiddleButton);

#if QWT_VER>=6
	m_pZoomer = new MyQwtPlotZoomer(m_pPlot->canvas());
#else
	// display tracker overlay anyway
	if(bNoTrackerSignal==0) bNoTrackerSignal = 1;
#endif

	m_pPicker = new MyQwtPlotPicker(this, bNoTrackerSignal);
	m_pPlot->canvas()->setMouseTracking(1);

	// fonts
	QwtText txt = m_pPlot->title();
	txt.setFont(g_fontGfx);
	m_pPlot->setTitle(txt);
	for(QwtPlot::Axis ax : {QwtPlot::yLeft, QwtPlot::yRight, QwtPlot::xBottom, QwtPlot::xTop})
	{
		txt = m_pPlot->axisTitle(ax);
		txt.setFont(g_fontGfx);
		m_pPlot->setAxisTitle(ax, txt);
	}
}


QwtPlotWrapper::~QwtPlotWrapper()
{
	if(m_pPicker)
	{
		m_pPicker->setEnabled(0);
		delete m_pPicker;
		m_pPicker = nullptr;
	}
	if(m_pGrid) { delete m_pGrid; m_pGrid = nullptr; }
	if(m_pZoomer) { delete m_pZoomer; m_pZoomer = nullptr; }
	if(m_pPanner) { delete m_pPanner; m_pPanner = nullptr; }

	for(QwtPlotCurve* pCurve : m_vecCurves)
		if(pCurve) delete pCurve;
	m_vecCurves.clear();

	if(m_pSpec) { delete m_pSpec; m_pSpec = nullptr; }
}


#if QWT_VER>=6
	bool QwtPlotWrapper::HasTrackerSignal() const { return 1; }
#else
	bool QwtPlotWrapper::HasTrackerSignal() const { return 0; }
#endif


void QwtPlotWrapper::SetData(const std::vector<t_real_qwt>& vecX, const std::vector<t_real_qwt>& vecY,
	unsigned int iCurve, bool bReplot, bool bCopy)
{
	if(!bCopy)	// copy pointers
	{
#if QWT_VER>=6
		m_vecCurves[iCurve]->setRawSamples(vecX.data(), vecY.data(), std::min<t_real_qwt>(vecX.size(), vecY.size()));
#else
		m_vecCurves[iCurve]->setRawData(vecX.data(), vecY.data(), std::min<t_real_qwt>(vecX.size(), vecY.size()));
#endif

		m_bHasDataPtrs = 1;
		if(iCurve >= m_vecDataPtrs.size())
			m_vecDataPtrs.resize(iCurve+1);

		m_vecDataPtrs[iCurve].first = &vecX;
		m_vecDataPtrs[iCurve].second = &vecY;
	}
	else		// copy data
	{
#if QWT_VER>=6
		m_vecCurves[iCurve]->setSamples(vecX.data(), vecY.data(), std::min<t_real_qwt>(vecX.size(), vecY.size()));
#else
		m_vecCurves[iCurve]->setData(vecX.data(), vecY.data(), std::min<t_real_qwt>(vecX.size(), vecY.size()));
#endif

		m_bHasDataPtrs = 0;
		m_vecDataPtrs.clear();
	}

	if(bReplot)
	{
		set_zoomer_base(m_pZoomer, vecX, vecY);
		m_pPlot->replot();
	}
}


void QwtPlotWrapper::SavePlot() const
{
	if(!m_bHasDataPtrs)
	{
		// if data was deep-copied, it is now lost somewhere in the plot objects...
		QMessageBox::critical(m_pPlot, "Error", "Cannot get plot data.");
		return;
	}

	QFileDialog::Option fileopt = QFileDialog::Option(0);
	//if(!m_settings.value("main/native_dialogs", 1).toBool())
	//	fileopt = QFileDialog::DontUseNativeDialog;

	std::string strFile = QFileDialog::getSaveFileName(m_pPlot,
		"Save Plot Data", nullptr, "Data files (*.dat *.DAT)",
		nullptr, fileopt).toStdString();

	tl::trim(strFile);
	if(strFile == "")
		return;

	std::ofstream ofstrDat(strFile);
	if(!ofstrDat)
	{
		std::string strErr = "Cannot open file \"" + strFile + "\" for saving.";
		QMessageBox::critical(m_pPlot, "Error", strErr.c_str());
		return;
	}

	ofstrDat.precision(g_iPrec);
	ofstrDat << "# title: " << m_pPlot->title().text().toStdString() << "\n";
	ofstrDat << "# x_label: " << m_pPlot->axisTitle(QwtPlot::xBottom).text().toStdString() << "\n";
	ofstrDat << "# y_label: " << m_pPlot->axisTitle(QwtPlot::yLeft).text().toStdString() << "\n";
	ofstrDat << "# x_label_2: " << m_pPlot->axisTitle(QwtPlot::xTop).text().toStdString() << "\n";
	ofstrDat << "# y_label_2: " << m_pPlot->axisTitle(QwtPlot::yRight).text().toStdString() << "\n";
	ofstrDat << "\n";

	std::size_t iDataSet=0;
	for(const auto& pairVecs : m_vecDataPtrs)
	{
		if(m_vecDataPtrs.size() > 1)
			ofstrDat << "## -------------------------------- begin of dataset " << (iDataSet+1)
				<< " --------------------------------\n";

		const std::vector<t_real_qwt>* pVecX = pairVecs.first;
		const std::vector<t_real_qwt>* pVecY = pairVecs.second;

		const std::size_t iSize = std::min(pVecX->size(), pVecY->size());

		for(std::size_t iCur=0; iCur<iSize; ++iCur)
		{
			ofstrDat << std::left << std::setw(g_iPrec*2) << pVecX->operator[](iCur) << " ";
			ofstrDat << std::left << std::setw(g_iPrec*2) << pVecY->operator[](iCur) << "\n";
		}

		if(m_vecDataPtrs.size() > 1)
			ofstrDat << "## -------------------------------- end of dataset " << (iDataSet+1)
				<< " ----------------------------------\n\n";
		++iDataSet;
	}
}

void QwtPlotWrapper::setAxisTitle(int iAxis, const QString& str)
{
	if(!m_pPlot) return;
	m_pPlot->setAxisTitle(iAxis, str);
}

void QwtPlotWrapper::setZoomBase(const QRectF& rect)
{
	if(!m_pZoomer) return;
	m_pZoomer->setZoomBase(rect);
}

void QwtPlotWrapper::scaleColorBar()
{
	if(!m_pPlot || !m_pRaster) return;
	t_real_qwt dMin = m_pRaster->GetZMin();
	t_real_qwt dMax = m_pRaster->GetZMax();

	QwtScaleWidget *pRightAxis = m_pPlot->axisWidget(QwtPlot::yRight);
	pRightAxis->setColorMap(QwtInterval(dMin, dMax), (QwtColorMap*)pRightAxis->colorMap());
	m_pPlot->setAxisScale(QwtPlot::yRight, dMin, dMax);
}

// ----------------------------------------------------------------------------


void MyQwtRasterData::SetXRange(t_real_qwt dMin, t_real_qwt dMax)
{
	m_dXRange[0] = dMin; m_dXRange[1] = dMax;
	setInterval(Qt::XAxis, QwtInterval(dMin, dMax));
	//set_zoomer_base(m_pZoomer, m_dXRange[0],m_dXRange[1],m_dYRange[0],m_dYRange[1], 1);
}

void MyQwtRasterData::SetYRange(t_real_qwt dMin, t_real_qwt dMax)
{
	m_dYRange[0] = dMin; m_dYRange[1] = dMax;
	setInterval(Qt::YAxis, QwtInterval(dMin, dMax));
	//set_zoomer_base(m_pZoomer, m_dXRange[0],m_dXRange[1],m_dYRange[0],m_dYRange[1], 1);
}

void MyQwtRasterData::SetZRange(t_real_qwt dMin, t_real_qwt dMax)
{
	m_dZRange[0] = dMin; m_dZRange[1] = dMax;
	setInterval(Qt::ZAxis, QwtInterval(dMin, dMax));
}

void MyQwtRasterData::SetZRange()	// automatically determined range
{
	if(!m_pData) return;

	auto minmax = std::minmax_element(m_pData.get(), m_pData.get()+m_iW*m_iH);
	m_dZRange[0] = *minmax.first; m_dZRange[1] = *minmax.second;
	setInterval(Qt::ZAxis, QwtInterval(m_dZRange[0], m_dZRange[1]));
}


t_real_qwt MyQwtRasterData::value(t_real_qwt dx, t_real_qwt dy) const
{
	if(dx<m_dXRange[0] || dy<m_dYRange[0] ||
		dx>=m_dXRange[1] || dy>=m_dYRange[1])
		return t_real_qwt(0);

	std::size_t iX = tl::tic_trafo_inv(m_iW, m_dXRange[0], m_dXRange[1], 0, dx);
	std::size_t iY = tl::tic_trafo_inv(m_iH, m_dYRange[0], m_dYRange[1], 0, dy);

	return GetPixel(iX, iY);
}

// ----------------------------------------------------------------------------


void set_zoomer_base(QwtPlotZoomer *pZoomer,
	t_real_qwt dL, t_real_qwt dR, t_real_qwt dT, t_real_qwt dB,
	bool bMetaCall, QwtPlotWrapper* pPlotWrap)
{
	if(!pZoomer) return;

	QRectF rect;
	rect.setLeft(dL);	rect.setRight(dR);
	rect.setTop(dT);	rect.setBottom(dB);

	if(bMetaCall)
	{
		QMetaObject::invokeMethod(pZoomer, "zoom",
			Qt::ConnectionType::BlockingQueuedConnection,
			Q_ARG(QRectF, rect));

		if(pPlotWrap)
		{
			// use auxilliary slot in QwtPlotWrapper
			// since setZoomBase is not a slot in QwtPlotZoomer
			QMetaObject::invokeMethod(pPlotWrap, "setZoomBase",
				Qt::ConnectionType::BlockingQueuedConnection,
				Q_ARG(const QRectF&, rect));
		}
	}
	else
	{
		pZoomer->zoom(rect);
		pZoomer->setZoomBase(rect);
	}
}

void set_zoomer_base(QwtPlotZoomer *pZoomer,
	const std::vector<t_real_qwt>& vecX, const std::vector<t_real_qwt>& vecY,
	bool bMetaCall, QwtPlotWrapper* pPlotWrap)
{
	if(!pZoomer || !vecX.size() || !vecY.size())
		return;

	const auto minmaxX = std::minmax_element(vecX.begin(), vecX.end());
	const auto minmaxY = std::minmax_element(vecY.begin(), vecY.end());

	t_real_qwt dminmax[] = {*minmaxX.first, *minmaxX.second,
		*minmaxY.first, *minmaxY.second};

	if(tl::float_equal<t_real_qwt>(dminmax[0], dminmax[1]))
	{
		dminmax[0] -= dminmax[0]/10.;
		dminmax[1] += dminmax[1]/10.;
	}
	if(tl::float_equal<t_real_qwt>(dminmax[2], dminmax[3]))
	{
		dminmax[2] -= dminmax[2]/10.;
		dminmax[3] += dminmax[3]/10.;
	}

	set_zoomer_base(pZoomer,
		dminmax[0],dminmax[1],dminmax[2],dminmax[3],
		bMetaCall, pPlotWrap);
}


#include "qwthelper.moc"
