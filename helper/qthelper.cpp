/*
 * qt&qwt helpers
 * @author Tobias Weber
 * @date feb-2016
 * @license GPLv2
 */

#include "qthelper.h"
#include "tlibs/math/math.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <memory>

#include <qwt_picker_machine.h>
#include <QMouseEvent>
#include <iostream>


class MyQwtPlotZoomer : public QwtPlotZoomer
{
protected:
	virtual void widgetMouseReleaseEvent(QMouseEvent *pEvt) override
	{
		// filter out middle button event which is already used for panning
		if(pEvt->button() & Qt::MiddleButton)
			return;

		QwtPlotZoomer::widgetMouseReleaseEvent(pEvt);
	}

public:
	MyQwtPlotZoomer(QWidget* pWidget) : QwtPlotZoomer(pWidget) {}
	virtual ~MyQwtPlotZoomer() {}
};


QwtPlotWrapper::QwtPlotWrapper(QwtPlot *pPlot, unsigned int iNumCurves, bool bNoTrackerSignal)
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
	
	m_vecCurves.reserve(iNumCurves);
	for(unsigned int iCurve=0; iCurve<iNumCurves; ++iCurve)
	{
		QwtPlotCurve* pCurve = new QwtPlotCurve();
		pCurve->setPen(penCurve);
		pCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
		pCurve->attach(m_pPlot);
		m_vecCurves.push_back(pCurve);
	}
	
	m_pGrid = new QwtPlotGrid();
	m_pGrid->setPen(penGrid);
	m_pGrid->attach(m_pPlot);

	m_pPanner = new QwtPlotPanner(m_pPlot->canvas());
	m_pPanner->setMouseButton(Qt::MiddleButton);

#if QWT_VER>=6
	m_pZoomer = new MyQwtPlotZoomer(m_pPlot->canvas());
	m_pZoomer->setMaxStackDepth(-1);
	m_pZoomer->setEnabled(1);
#endif

	m_pPlot->canvas()->setMouseTracking(1);
	if(bNoTrackerSignal)
	{
		m_pPicker = new QwtPlotPicker(m_pPlot->xBottom, m_pPlot->yLeft,
#if QWT_VER<6
			QwtPlotPicker::PointSelection,
#endif
			QwtPlotPicker::NoRubberBand, 
			QwtPlotPicker::AlwaysOn, 
			m_pPlot->canvas());
	}
	else
	{
		m_pPicker = new QwtPlotPicker(m_pPlot->xBottom, m_pPlot->yLeft,
#if QWT_VER<6
			QwtPlotPicker::PointSelection,
#endif
			QwtPlotPicker::NoRubberBand,
#if QWT_VER>=6
			QwtPlotPicker::AlwaysOff,
#else
			QwtPlotPicker::AlwaysOn,
#endif
			m_pPlot->canvas());
	}

#if QWT_VER>=6
	m_pPicker->setStateMachine(new QwtPickerTrackerMachine());
	//connect(m_pPicker, SIGNAL(moved(const QPointF&)), this, SLOT(cursorMoved(const QPointF&)));
#endif
	m_pPicker->setEnabled(1);
}


QwtPlotWrapper::~QwtPlotWrapper()
{
	if(m_pPicker)
	{
		m_pPicker->setEnabled(0);
		delete m_pPicker;
		m_pPicker = nullptr;
	}
	if(m_pGrid)
	{
		delete m_pGrid;
		m_pGrid = nullptr;
	}
	if(m_pZoomer)
	{
		delete m_pZoomer;
		m_pZoomer = nullptr;
	}
	if(m_pPanner)
	{
		delete m_pPanner;
		m_pPanner = nullptr;
	}
}


#if QWT_VER>=6
	bool QwtPlotWrapper::HasTrackerSignal() const { return 1; }
#else
	bool QwtPlotWrapper::HasTrackerSignal() const { return 0; }
#endif


void QwtPlotWrapper::SetData(const std::vector<double>& vecX, const std::vector<double>& vecY, 
	unsigned int iCurve, bool bReplot)
{
#if QWT_VER>=6
	m_vecCurves[iCurve]->setRawSamples(vecX.data(), vecY.data(), std::min<double>(vecX.size(), vecY.size()));
#else
	m_vecCurves[iCurve]->setRawData(vecX.data(), vecY.data(), std::min<double>(vecX.size(), vecY.size()));
#endif

	if(bReplot)
	{
		set_zoomer_base(m_pZoomer, vecX, vecY);
		m_pPlot->replot();
	}
}

// ----------------------------------------------------------------------------


bool save_table(const char* pcFile, const QTableWidget* pTable)
{
	std::ofstream ofstr(pcFile);
	if(!ofstr)
		return false;

	const int iNumCols = pTable->columnCount();
	const int iNumRows = pTable->rowCount();

	// item lengths
	std::unique_ptr<int[]> ptrMaxTxtLen(new int[iNumCols]);
	for(int iCol=0; iCol<iNumCols; ++iCol)
		ptrMaxTxtLen[iCol] = 0;

	for(int iCol=0; iCol<iNumCols; ++iCol)
	{
		const QTableWidgetItem *pItem = pTable->horizontalHeaderItem(iCol);
		ptrMaxTxtLen[iCol] = std::max(pItem ? pItem->text().length() : 0, ptrMaxTxtLen[iCol]);
	}

	for(int iRow=0; iRow<iNumRows; ++iRow)
		for(int iCol=0; iCol<iNumCols; ++iCol)
		{
			const QTableWidgetItem *pItem = pTable->item(iRow, iCol);
			ptrMaxTxtLen[iCol] = std::max(pItem ? pItem->text().length() : 0, ptrMaxTxtLen[iCol]);
		}


	// write items
	for(int iCol=0; iCol<iNumCols; ++iCol)
	{
		const QTableWidgetItem *pItem = pTable->horizontalHeaderItem(iCol);
		ofstr << std::setw(ptrMaxTxtLen[iCol]+4) << (pItem ? pItem->text().toStdString() : "");
	}
	ofstr << "\n";

	for(int iRow=0; iRow<iNumRows; ++iRow)
	{
		for(int iCol=0; iCol<iNumCols; ++iCol)
		{
			const QTableWidgetItem *pItem = pTable->item(iRow, iCol);
			ofstr << std::setw(ptrMaxTxtLen[iCol]+4) << (pItem ? pItem->text().toStdString() : "");
		}

		ofstr << "\n";
	}

	return true;
}


void set_zoomer_base(QwtPlotZoomer *pZoomer,
	const std::vector<double>& vecX, const std::vector<double>& vecY,
	bool bMetaCall)
{
	if(!pZoomer)
		return;

	const auto minmaxX = std::minmax_element(vecX.begin(), vecX.end());
	const auto minmaxY = std::minmax_element(vecY.begin(), vecY.end());
	//tl::log_debug("min: ", *minmaxX.first, " ", *minmaxY.first);
	//tl::log_debug("max: ", *minmaxX.second, " ", *minmaxY.second);

	double dminmax[] = {*minmaxX.first, *minmaxX.second,
		*minmaxY.first, *minmaxY.second};

	if(tl::float_equal(dminmax[0], dminmax[1]))
	{
		dminmax[0] -= dminmax[0]/10.;
		dminmax[1] += dminmax[1]/10.;
	}
	if(tl::float_equal(dminmax[2], dminmax[3]))
	{
		dminmax[2] -= dminmax[2]/10.;
		dminmax[3] += dminmax[3]/10.;
	}

	QRectF rect;
	rect.setLeft(dminmax[0]);
	rect.setRight(dminmax[1]);
	rect.setBottom(dminmax[3]);
	rect.setTop(dminmax[2]);

	if(bMetaCall)
	{
		QMetaObject::invokeMethod(pZoomer, "zoom",
			Qt::ConnectionType::DirectConnection,
			Q_ARG(QRectF, rect));
		// unfortunately not a metafunction...
		//QMetaObject::invokeMethod(pZoomer, "setZoomBase", 
		//	Qt::ConnectionType::DirectConnection,
		//	Q_ARG(QRectF, rect));
	}
	else
	{
		pZoomer->zoom(rect);
		pZoomer->setZoomBase(rect);
	}
}
