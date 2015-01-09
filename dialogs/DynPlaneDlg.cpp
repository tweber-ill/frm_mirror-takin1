/*
 * Dynamic Plane Dialog
 * @author tweber
 * @date 2013, jan-2015
 */

#include "DynPlaneDlg.h"

#include "../tlibs/string/string.h"
#include "../tlibs/math/neutrons.hpp"

#include <boost/units/io.hpp>
#include <qwt_picker_machine.h>

namespace units = boost::units;
namespace co = boost::units::si::constants::codata;


DynPlaneDlg::DynPlaneDlg(QWidget* pParent, QSettings *pSettings)
		: QDialog(pParent), m_pSettings(pSettings)
{
	this->setupUi(this);

	m_pGrid = new QwtPlotGrid();
	QPen penGrid;
	penGrid.setColor(QColor(0x99,0x99,0x99));
	penGrid.setStyle(Qt::DashLine);
	m_pGrid->setPen(penGrid);
	m_pGrid->attach(plot);

	m_pCurve = new QwtPlotCurve("Kinematic Plane (top)");
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
	plot->setAxisTitle(QwtPlot::yLeft, "E (meV)");


	QObject::connect(comboFixedE, SIGNAL(currentIndexChanged(int)), this, SLOT(FixedKiKfToggled()));

	std::vector<QDoubleSpinBox*> vecSpinBoxes = {spinEiEf, spinMinQ, spinMaxQ, spinAngle};
	for(QDoubleSpinBox* pSpin : vecSpinBoxes)
		QObject::connect(pSpin, SIGNAL(valueChanged(double)), this, SLOT(Calc()));
	QObject::connect(btnSync, SIGNAL(toggled(bool)), this, SLOT(Calc()));

	Calc();
}

DynPlaneDlg::~DynPlaneDlg()
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

void DynPlaneDlg::cursorMoved(const QPointF& pt)
{
	std::string strX = std::to_string(pt.x());
	std::string strY = std::to_string(pt.y());

	std::ostringstream ostr;
	ostr << "(" << strX << ", " << strY << ")";

	this->labelStatus->setText(ostr.str().c_str());
}

void DynPlaneDlg::Calc()
{
	const unsigned int NUM_POINTS = 512;

	const double dMinQ = spinMinQ->value();
	const double dMaxQ = spinMaxQ->value();
	const double dAngle = spinAngle->value() / 180. * M_PI;
	const bool bFixedKi = (comboFixedE->currentIndex()==0);

	if(btnSync->isChecked())
		spinEiEf->setValue(bFixedKi ? m_dEi : m_dEf);

	units::quantity<units::si::energy> EiEf = spinEiEf->value() * tl::one_meV;


	//m_pPlanePlot->clear();
	std::vector<double> vecQ[2], vecE[2];
	vecQ[0].reserve(NUM_POINTS); vecE[0].reserve(NUM_POINTS);
	vecQ[1].reserve(NUM_POINTS); vecE[1].reserve(NUM_POINTS);

	units::quantity<units::si::plane_angle> twotheta = dAngle * units::si::radians;

	for(unsigned int iPt=0; iPt<NUM_POINTS; ++iPt)
	{
		for(unsigned int iSign=0; iSign<=1; ++iSign)
		{
			units::quantity<units::si::wavenumber> Q = (dMinQ + (dMaxQ - dMinQ)/double(NUM_POINTS)*double(iPt)) / tl::angstrom;
			units::quantity<units::si::energy> dE = tl::kinematic_plane(bFixedKi, iSign, EiEf, Q, twotheta);

			double _dQ = Q * tl::angstrom;
			double _dE = dE / tl::one_meV;

			if(!std::isnan(_dQ) && !std::isnan(_dE) && !std::isinf(_dQ) && !std::isinf(_dE))
			{
				vecQ[iSign].push_back(Q * tl::angstrom);
				vecE[iSign].push_back(dE / tl::one_meV);
			}
		}
	}

	m_vecQ.clear();
	m_vecE.clear();

	m_vecQ.insert(m_vecQ.end(), vecQ[0].rbegin(), vecQ[0].rend());
	m_vecE.insert(m_vecE.end(), vecE[0].rbegin(), vecE[0].rend());

	m_vecQ.insert(m_vecQ.end(), vecQ[1].begin(), vecQ[1].end());
	m_vecE.insert(m_vecE.end(), vecE[1].begin(), vecE[1].end());


#ifdef USE_QWT6
	m_pCurve->setRawSamples(m_vecQ.data(), m_vecE.data(), m_vecQ.size());
#else
	m_pCurve->setRawData(m_vecQ.data(), m_vecE.data(), m_vecQ.size());
#endif

	plot->replot();
}

void DynPlaneDlg::RecipParamsChanged(const RecipParams& params)
{
	m_d2Theta = params.d2Theta;
	m_dEi = tl::k2E(params.dki/tl::angstrom)/tl::one_meV;
	m_dEf = tl::k2E(params.dkf/tl::angstrom)/tl::one_meV;

	Calc();
}

void DynPlaneDlg::FixedKiKfToggled()
{
	if(comboFixedE->currentIndex() == 0)
		labelFixedKiKf->setText("E_i (meV):");
	else
		labelFixedKiKf->setText("E_f (meV):");

	Calc();
}


void DynPlaneDlg::showEvent(QShowEvent *pEvt)
{
	if(m_pSettings && m_pSettings->contains("dyn_plane/geo"))
		restoreGeometry(m_pSettings->value("dyn_plane/geo").toByteArray());

	QDialog::showEvent(pEvt);
}

void DynPlaneDlg::accept()
{
	if(m_pSettings)
		m_pSettings->setValue("dyn_plane/geo", saveGeometry());

	QDialog::accept();
}


#include "DynPlaneDlg.moc"
