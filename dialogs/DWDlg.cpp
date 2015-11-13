/*
 * Scattering factors dialog (e.g. Debye-Waller factor)
 * @author tweber
 * @date 2013, jan-2015
 * @license GPLv2
 */

#include "DWDlg.h"

#include "tlibs/string/string.h"
#include "tlibs/math/neutrons.hpp"
#include "tlibs/math/atoms.h"

#include <boost/units/io.hpp>
#include <qwt_picker_machine.h>


DWDlg::DWDlg(QWidget* pParent, QSettings *pSettings)
		: QDialog(pParent), m_pSettings(pSettings)
{
	this->setupUi(this);


	QPen penGrid;
	penGrid.setColor(QColor(0x99,0x99,0x99));
	penGrid.setStyle(Qt::DashLine);

	QPen penCurve;
	penCurve.setColor(QColor(0,0,0x99));
	penCurve.setWidth(2);


	QColor colorBck(240, 240, 240, 255);
	for(QwtPlot *pPlt : {plot, plotBose, plotAna, plotLorentz})
		pPlt->setCanvasBackground(colorBck);


	// -------------------------------------------------------------------------
	// Bose Factor stuff
	std::vector<QDoubleSpinBox*> vecSpinBoxesBose = {spinBoseT, spinBoseEMin, spinBoseEMax};
	for(QDoubleSpinBox* pSpin : vecSpinBoxesBose)
		QObject::connect(pSpin, SIGNAL(valueChanged(double)), this, SLOT(CalcBose()));

	m_pGridBose = new QwtPlotGrid();
	m_pGridBose->setPen(penGrid);
	m_pGridBose->attach(plotBose);

	// positive Bose factor
	m_pCurveBosePos = new QwtPlotCurve("Boson Creation");
	QPen penCurveBosePos;
	penCurveBosePos.setColor(QColor(0,0,0x99));
	penCurveBosePos.setWidth(2);
	m_pCurveBosePos->setPen(penCurveBosePos);
	m_pCurveBosePos->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	m_pCurveBosePos->attach(plotBose);

	// negative Bose factor
	m_pCurveBoseNeg = new QwtPlotCurve("Boson Annihilation");
	QPen penCurveBoseNeg;
	penCurveBoseNeg.setColor(QColor(0x99,0,0));
	penCurveBoseNeg.setWidth(2);
	m_pCurveBoseNeg->setPen(penCurveBoseNeg);
	m_pCurveBoseNeg->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	m_pCurveBoseNeg->attach(plotBose);

	plotBose->canvas()->setMouseTracking(1);
	m_pPickerBose = new QwtPlotPicker(plotBose->xBottom, plotBose->yLeft,
#if QWT_VER<6
									QwtPlotPicker::PointSelection,
#endif
									QwtPlotPicker::NoRubberBand,
#if QWT_VER>=6
									QwtPlotPicker::AlwaysOff,
#else
									QwtPlotPicker::AlwaysOn,
#endif
									plotBose->canvas());

#if QWT_VER>=6
	m_pPickerBose->setStateMachine(new QwtPickerTrackerMachine());
	connect(m_pPickerBose, SIGNAL(moved(const QPointF&)), this, SLOT(cursorMoved(const QPointF&)));
#endif
	m_pPickerBose->setEnabled(1);

	m_pLegendBose = new QwtLegend();
	plotBose->insertLegend(m_pLegendBose, QwtPlot::TopLegend);

	plotBose->setAxisTitle(QwtPlot::xBottom, "E (meV)");
	plotBose->setAxisTitle(QwtPlot::yLeft, "Bose Factor");

	CalcBose();


	// -------------------------------------------------------------------------
	// DW Factor stuff
	std::vector<QDoubleSpinBox*> vecSpinBoxes = {spinAMU_deb, spinTD_deb, spinT_deb, spinMinQ_deb, spinMaxQ_deb};
	for(QDoubleSpinBox* pSpin : vecSpinBoxes)
		QObject::connect(pSpin, SIGNAL(valueChanged(double)), this, SLOT(CalcDW()));

	m_pGrid = new QwtPlotGrid();
	m_pGrid->setPen(penGrid);
	m_pGrid->attach(plot);

	m_pCurve = new QwtPlotCurve("Debye-Waller Factor");
	m_pCurve->setPen(penCurve);
	m_pCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	m_pCurve->attach(plot);

	plot->canvas()->setMouseTracking(1);
	m_pPicker = new QwtPlotPicker(plot->xBottom, plot->yLeft,
#if QWT_VER<6
									QwtPlotPicker::PointSelection,
#endif
									QwtPlotPicker::NoRubberBand,
#if QWT_VER>=6
									QwtPlotPicker::AlwaysOff,
#else
									QwtPlotPicker::AlwaysOn,
#endif
									plot->canvas());

#if QWT_VER>=6
	m_pPicker->setStateMachine(new QwtPickerTrackerMachine());
	connect(m_pPicker, SIGNAL(moved(const QPointF&)), this, SLOT(cursorMoved(const QPointF&)));
#endif
	m_pPicker->setEnabled(1);


	plot->setAxisTitle(QwtPlot::xBottom, "Q (1/A)");
	plot->setAxisTitle(QwtPlot::yLeft, "DW Factor");

	CalcDW();



	// -------------------------------------------------------------------------
	// Ana Factor stuff
	std::vector<QDoubleSpinBox*> vecSpinBoxesAna = {spinAnad, spinMinkf, spinMaxkf};
	for(QDoubleSpinBox* pSpin : vecSpinBoxesAna)
		QObject::connect(pSpin, SIGNAL(valueChanged(double)), this, SLOT(CalcAna()));

	m_pGridAna = new QwtPlotGrid();
	m_pGridAna->setPen(penGrid);
	m_pGridAna->attach(plotAna);

	m_pCurveAna = new QwtPlotCurve("Analyser Factor");
	m_pCurveAna->setPen(penCurve);
	m_pCurveAna->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	m_pCurveAna->attach(plotAna);

	plotAna->canvas()->setMouseTracking(1);
	m_pPickerAna = new QwtPlotPicker(plotAna->xBottom, plotAna->yLeft,
#if QWT_VER<6
									QwtPlotPicker::PointSelection,
#endif
									QwtPlotPicker::NoRubberBand,
#if QWT_VER>=6
									QwtPlotPicker::AlwaysOff,
#else
									QwtPlotPicker::AlwaysOn,
#endif
									plotAna->canvas());

#if QWT_VER>=6
	m_pPickerAna->setStateMachine(new QwtPickerTrackerMachine());
	connect(m_pPickerAna, SIGNAL(moved(const QPointF&)), this, SLOT(cursorMoved(const QPointF&)));
#endif
	m_pPickerAna->setEnabled(1);


	plotAna->setAxisTitle(QwtPlot::xBottom, "kf (1/A)");
	plotAna->setAxisTitle(QwtPlot::yLeft, "Intensity (a.u.)");

	CalcAna();



	// -------------------------------------------------------------------------
	// Lorentz Factor stuff
	std::vector<QDoubleSpinBox*> vecSpinBoxesLor = {spinMin2Th, spinMax2Th};
	for(QDoubleSpinBox* pSpin : vecSpinBoxesLor)
		QObject::connect(pSpin, SIGNAL(valueChanged(double)), this, SLOT(CalcLorentz()));
	QObject::connect(checkPol, SIGNAL(toggled(bool)), this, SLOT(CalcLorentz()));

	m_pGridLor = new QwtPlotGrid();
	m_pGridLor->setPen(penGrid);
	m_pGridLor->attach(plotLorentz);

	m_pCurveLor = new QwtPlotCurve("Lorentz Factor");
	m_pCurveLor->setPen(penCurve);
	m_pCurveLor->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	m_pCurveLor->attach(plotLorentz);

	plotLorentz->canvas()->setMouseTracking(1);
	m_pPickerLor = new QwtPlotPicker(plotLorentz->xBottom, plotLorentz->yLeft,
#if QWT_VER<6
									QwtPlotPicker::PointSelection,
#endif
									QwtPlotPicker::NoRubberBand,
#if QWT_VER>=6
									QwtPlotPicker::AlwaysOff,
#else
									QwtPlotPicker::AlwaysOn,
#endif
									plotLorentz->canvas());

#if QWT_VER>=6
	m_pPickerLor->setStateMachine(new QwtPickerTrackerMachine());
	connect(m_pPickerLor, SIGNAL(moved(const QPointF&)), this, SLOT(cursorMoved(const QPointF&)));
#endif
	m_pPickerLor->setEnabled(1);


	plotLorentz->setAxisTitle(QwtPlot::xBottom, "Scattering Angle (deg)");
	plotLorentz->setAxisTitle(QwtPlot::yLeft, "Lorentz Factor");

	CalcLorentz();
}


DWDlg::~DWDlg()
{
	for(QwtPlotPicker** pPicker : {&m_pPicker, &m_pPickerAna, &m_pPickerBose, &m_pPickerLor})
	{
		if(*pPicker)
		{
			(*pPicker)->setEnabled(0);
			delete *pPicker;
			*pPicker = nullptr;
		}
	}

	for(QwtPlotGrid** pGrid : {&m_pGrid, &m_pGridAna, &m_pGridBose, &m_pGridLor})
	{
		if(*pGrid)
		{
			delete *pGrid;
			*pGrid = nullptr;
		}
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


void DWDlg::CalcBose()
{
	const unsigned int NUM_POINTS = 512;

	const double dMinE = spinBoseEMin->value();
	const double dMaxE = spinBoseEMax->value();

	const tl::temp T = spinBoseT->value() * tl::kelvin;

	m_vecBoseE.clear();
	m_vecBoseIntPos.clear();
	m_vecBoseIntNeg.clear();

	m_vecBoseE.reserve(NUM_POINTS);
	m_vecBoseIntPos.reserve(NUM_POINTS);
	m_vecBoseIntNeg.reserve(NUM_POINTS);

	for(unsigned int iPt=0; iPt<NUM_POINTS; ++iPt)
	{
		tl::energy E = (dMinE + (dMaxE - dMinE)/double(NUM_POINTS)*double(iPt)) * tl::meV;
		m_vecBoseE.push_back(E / tl::meV);

		m_vecBoseIntPos.push_back(tl::bose(E, T));
		m_vecBoseIntNeg.push_back(tl::bose(-E, T));
	}

#if QWT_VER>=6
	m_pCurveBosePos->setRawSamples(m_vecBoseE.data(), m_vecBoseIntPos.data(), m_vecBoseE.size());
	m_pCurveBoseNeg->setRawSamples(m_vecBoseE.data(), m_vecBoseIntNeg.data(), m_vecBoseE.size());
#else
	m_pCurveBosePos->setRawData(m_vecBoseE.data(), m_vecBoseIntPos.data(), m_vecBoseE.size());
	m_pCurveBoseNeg->setRawData(m_vecBoseE.data(), m_vecBoseIntNeg.data(), m_vecBoseE.size());
#endif

	plotBose->replot();
}


void DWDlg::CalcLorentz()
{
	const unsigned int NUM_POINTS = 512;

	const double dMin2th = spinMin2Th->value() / 180.*M_PI;
	const double dMax2th = spinMax2Th->value() / 180.*M_PI;

	const bool bPol = checkPol->isChecked();

	m_vecLor2th.clear();
	m_vecLor.clear();

	m_vecLor2th.reserve(NUM_POINTS);
	m_vecLor.reserve(NUM_POINTS);

	for(unsigned int iPt=0; iPt<NUM_POINTS; ++iPt)
	{
		double d2th = (dMin2th + (dMax2th - dMin2th)/double(NUM_POINTS)*double(iPt));
		double dLor = tl::lorentz_factor(d2th);
		if(bPol)
			dLor *= tl::lorentz_pol_factor(d2th);

		m_vecLor2th.push_back(d2th /M_PI*180.);
		m_vecLor.push_back(dLor);
	}

#if QWT_VER>=6
	m_pCurveLor->setRawSamples(m_vecLor2th.data(), m_vecLor.data(), m_vecLor2th.size());
#else
	m_pCurveLor->setRawData(m_vecLor2th.data(), m_vecLor.data(), m_vecLor2th.size());
#endif

	plotLorentz->replot();
}


void DWDlg::CalcDW()
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

#if QWT_VER>=6
	m_pCurve->setRawSamples(m_vecQ.data(), m_vecDeb.data(), m_vecQ.size());
#else
	m_pCurve->setRawData(m_vecQ.data(), m_vecDeb.data(), m_vecQ.size());
#endif

	plot->replot();
}


void DWDlg::CalcAna()
{
	const unsigned int NUM_POINTS = 512;

	const tl::length d = spinAnad->value()*tl::angstrom;
	const double dMinKf = spinMinkf->value();
	const double dMaxKf = spinMaxkf->value();

	double dAngMax = 0.5*tl::get_mono_twotheta(dMinKf/tl::angstrom, d, 1) / tl::radians / M_PI * 180.;
	double dAngMin = 0.5*tl::get_mono_twotheta(dMaxKf/tl::angstrom, d, 1) / tl::radians / M_PI * 180.;

	editAngMin->setText(tl::var_to_str(dAngMin).c_str());
	editAngMax->setText(tl::var_to_str(dAngMax).c_str());

	m_veckf.clear();
	m_vecInt.clear();

	m_veckf.reserve(NUM_POINTS);
	m_vecInt.reserve(NUM_POINTS);

	for(unsigned int iPt=0; iPt<NUM_POINTS; ++iPt)
	{
		tl::wavenumber kf = (dMinKf + (dMaxKf - dMinKf)/double(NUM_POINTS)*double(iPt)) / tl::angstrom;
		double dEffic = tl::ana_effic_factor(kf, d);

		m_veckf.push_back(kf * tl::angstrom);
		m_vecInt.push_back(dEffic);
	}

#if QWT_VER>=6
	m_pCurveAna->setRawSamples(m_veckf.data(), m_vecInt.data(), m_veckf.size());
#else
	m_pCurveAna->setRawData(m_veckf.data(), m_vecInt.data(), m_veckf.size());
#endif

	plotAna->replot();
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

