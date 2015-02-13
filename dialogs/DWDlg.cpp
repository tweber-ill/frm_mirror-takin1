/*
 * Debye-Waller Dialog
 * @author tweber
 * @date 2013, jan-2015
 * @copyright GPLv2
 */

#include "DWDlg.h"

#include "../tlibs/string/string.h"
#include "../tlibs/math/neutrons.hpp"

#include <boost/units/io.hpp>
#include <qwt_picker_machine.h>


DWDlg::DWDlg(QWidget* pParent, QSettings *pSettings)
		: QDialog(pParent), m_pSettings(pSettings)
{
	this->setupUi(this);

	std::vector<QDoubleSpinBox*> vecSpinBoxes = {spinAMU_deb, spinTD_deb, spinT_deb, spinMinQ_deb, spinMaxQ_deb};
	for(QDoubleSpinBox* pSpin : vecSpinBoxes)
		QObject::connect(pSpin, SIGNAL(valueChanged(double)), this, SLOT(Calc()));

	m_pGrid = new QwtPlotGrid();
	QPen penGrid;
	penGrid.setColor(QColor(0x99,0x99,0x99));
	penGrid.setStyle(Qt::DashLine);
	m_pGrid->setPen(penGrid);
	m_pGrid->attach(plot);

	m_pCurve = new QwtPlotCurve("Debye-Waller Factor");
	QPen penCurve;
	penCurve.setColor(QColor(0,0,0x99));
	penCurve.setWidth(2);
	m_pCurve->setPen(penCurve);
	m_pCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	m_pCurve->attach(plot);

	plot->canvas()->setMouseTracking(1);
	m_pPicker = new QwtPlotPicker(plot->xBottom, plot->yLeft,
#ifndef USE_QWT6
									QwtPlotPicker::PointSelection,
#endif
									QwtPlotPicker::NoRubberBand,
#ifdef USE_QWT6
									QwtPlotPicker::AlwaysOff,
#else
									QwtPlotPicker::AlwaysOn,
#endif
									plot->canvas());

#ifdef USE_QWT6
	m_pPicker->setStateMachine(new QwtPickerTrackerMachine());
	connect(m_pPicker, SIGNAL(moved(const QPointF&)), this, SLOT(cursorMoved(const QPointF&)));
#endif
	m_pPicker->setEnabled(1);


	plot->setAxisTitle(QwtPlot::xBottom, "Q (1/A)");
	plot->setAxisTitle(QwtPlot::yLeft, "DW Factor");

	Calc();
}

DWDlg::~DWDlg()
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
}

void DWDlg::cursorMoved(const QPointF& pt)
{
	std::string strX = std::to_string(pt.x());
	std::string strY = std::to_string(pt.y());

	std::ostringstream ostr;
	ostr << "(" << strX << ", " << strY << ")";

	this->labelStatus->setText(ostr.str().c_str());
}

void DWDlg::Calc()
{
	const unsigned int NUM_POINTS = 512;

	double dMinQ = spinMinQ_deb->value();
	double dMaxQ = spinMaxQ_deb->value();

	tl::temp T = spinT_deb->value() * tl::kelvin;
	tl::temp T_D = spinTD_deb->value() * tl::kelvin;
	tl::mass M = spinAMU_deb->value() * tl::amu;

	m_vecQ.clear();
	m_vecDeb.clear();

	m_vecQ.reserve(NUM_POINTS);
	m_vecDeb.reserve(NUM_POINTS);

	bool bHasZetaSq = 0;
	for(unsigned int iPt=0; iPt<NUM_POINTS; ++iPt)
	{
		tl::wavenumber Q = (dMinQ + (dMaxQ - dMinQ)/double(NUM_POINTS)*double(iPt)) / tl::angstrom;
		double dDWF = 0.;
		auto zetasq = 1.*tl::angstrom*tl::angstrom;

		if(T <= T_D)
			dDWF = tl::debye_waller_low_T(T_D, T, M, Q, &zetasq);
		else
			dDWF = tl::debye_waller_high_T(T_D, T, M, Q, &zetasq);

		m_vecQ.push_back(Q * tl::angstrom);

		if(!bHasZetaSq)
		{
			std::string strZetaSq = tl::var_to_str(double(tl::units::sqrt(zetasq) / tl::angstrom));
			editZetaSq->setText(strZetaSq.c_str());

			bHasZetaSq = 1;
		}

		m_vecDeb.push_back(dDWF);
	}

#ifdef USE_QWT6
	m_pCurve->setRawSamples(m_vecQ.data(), m_vecDeb.data(), m_vecQ.size());
#else
	m_pCurve->setRawData(m_vecQ.data(), m_vecDeb.data(), m_vecQ.size());
#endif

	plot->replot();
}


void DWDlg::showEvent(QShowEvent *pEvt)
{
	if(m_pSettings && m_pSettings->contains("dw/geo"))
		restoreGeometry(m_pSettings->value("dw/geo").toByteArray());

	QDialog::showEvent(pEvt);
}

void DWDlg::accept()
{
	if(m_pSettings)
		m_pSettings->setValue("dw/geo", saveGeometry());

	QDialog::accept();
}


#include "DWDlg.moc"
