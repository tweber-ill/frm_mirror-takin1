/*
 * Dynamic Plane Dialog
 * @author tweber
 * @date 2013, jan-2015
 * @license GPLv2
 */

#include "DynPlaneDlg.h"
#include "tlibs/string/string.h"
#include "tlibs/math/neutrons.hpp"
#include <boost/units/io.hpp>


DynPlaneDlg::DynPlaneDlg(QWidget* pParent, QSettings *pSettings)
		: QDialog(pParent), m_pSettings(pSettings)
{
	this->setupUi(this);
	if(m_pSettings)
	{
		QFont font;
		if(m_pSettings->contains("main/font_gen") && font.fromString(m_pSettings->value("main/font_gen", "").toString()))
			setFont(font);
	}


	m_plotwrap.reset(new QwtPlotWrapper(plot));
	m_plotwrap->GetCurve(0)->setTitle("Kinematic Plane");
	m_plotwrap->GetPlot()->setAxisTitle(QwtPlot::xBottom, "Q (1/A)");
	m_plotwrap->GetPlot()->setAxisTitle(QwtPlot::yLeft, "E (meV)");
	if(m_plotwrap->HasTrackerSignal())
		connect(m_plotwrap->GetPicker(), SIGNAL(moved(const QPointF&)), this, SLOT(cursorMoved(const QPointF&)));


	QObject::connect(comboFixedE, SIGNAL(currentIndexChanged(int)), this, SLOT(FixedKiKfToggled()));

	std::vector<QDoubleSpinBox*> vecSpinBoxes = {spinEiEf, spinMinQ, spinMaxQ, spinAngle};
	for(QDoubleSpinBox* pSpin : vecSpinBoxes)
		QObject::connect(pSpin, SIGNAL(valueChanged(double)), this, SLOT(Calc()));
	QObject::connect(btnSync, SIGNAL(toggled(bool)), this, SLOT(Calc()));

	Calc();


	if(m_pSettings && m_pSettings->contains("dyn_plane/geo"))
		restoreGeometry(m_pSettings->value("dyn_plane/geo").toByteArray());
}

DynPlaneDlg::~DynPlaneDlg()
{}

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

	tl::energy EiEf = spinEiEf->value() * tl::one_meV;


	//m_pPlanePlot->clear();
	std::vector<double> vecQ[2], vecE[2];
	vecQ[0].reserve(NUM_POINTS); vecE[0].reserve(NUM_POINTS);
	vecQ[1].reserve(NUM_POINTS); vecE[1].reserve(NUM_POINTS);

	tl::angle twotheta = dAngle * tl::radians;

	for(unsigned int iPt=0; iPt<NUM_POINTS; ++iPt)
	{
		for(unsigned int iSign=0; iSign<=1; ++iSign)
		{
			tl::wavenumber Q = (dMinQ + (dMaxQ - dMinQ)/double(NUM_POINTS)*double(iPt)) / tl::angstrom;
			tl::energy dE = tl::kinematic_plane(bFixedKi, iSign, EiEf, Q, twotheta);

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

	m_plotwrap->SetData(m_vecQ, m_vecE);
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
	QDialog::showEvent(pEvt);
}

void DynPlaneDlg::accept()
{
	if(m_pSettings)
		m_pSettings->setValue("dyn_plane/geo", saveGeometry());

	QDialog::accept();
}


#include "DynPlaneDlg.moc"
