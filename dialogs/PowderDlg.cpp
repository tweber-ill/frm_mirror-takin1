/*
 * Powder Line Dialog
 * @author Tobias Weber
 * @date 2013, 2-dec-2014
 * @license GPLv2
 */

#include "PowderDlg.h"
#include "tlibs/math/lattice.h"
#include "tlibs/math/powder.h"
#include "tlibs/math/neutrons.hpp"
#include "tlibs/math/linalg.h"
#include "tlibs/string/string.h"
#include "tlibs/string/spec_char.h"

#include "helper/globals.h"
#include "helper/formfact.h"
#include "helper/qthelper.h"

#include <vector>
#include <string>
#include <iostream>
#include <boost/algorithm/string.hpp>

#include <QFileDialog>
#include <QMessageBox>
#include <qwt_picker_machine.h>

namespace ublas = boost::numeric::ublas;
namespace co = boost::units::si::constants::codata;


#define TABLE_ANGLE	0
#define TABLE_Q		1
#define TABLE_PEAK	2
#define TABLE_MULT	3
#define TABLE_FN	4
#define TABLE_IN	5
#define TABLE_FX	6
#define TABLE_IX	7

PowderDlg::PowderDlg(QWidget* pParent, QSettings* pSett)
			: QDialog(pParent), m_pSettings(pSett),
				m_pmapSpaceGroups(get_space_groups())
{
	this->setupUi(this);
	btnAtoms->setEnabled(g_bHasScatlens);


	// -------------------------------------------------------------------------
	// plot stuff
	QPen penGrid;
	penGrid.setColor(QColor(0x99,0x99,0x99));
	penGrid.setStyle(Qt::DashLine);

	QPen penCurve;
	penCurve.setColor(QColor(0,0,0x99));
	penCurve.setWidth(2);

	QColor colorBck(240, 240, 240, 255);
	for(QwtPlot *pPlt : {plotN, plotX})
		pPlt->setCanvasBackground(colorBck);


	m_pGrid = new QwtPlotGrid();
	m_pGrid->setPen(penGrid);
	m_pGrid->attach(plotN);

	m_pGridX = new QwtPlotGrid();
	m_pGridX->setPen(penGrid);
	m_pGridX->attach(plotX);

	m_pCurve = new QwtPlotCurve("Neutron Powder Pattern");
	m_pCurve->setPen(penCurve);
	m_pCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	m_pCurve->attach(plotN);

	m_pCurveX = new QwtPlotCurve("X-Ray Powder Pattern");
	m_pCurveX->setPen(penCurve);
	m_pCurveX->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	m_pCurveX->attach(plotX);

#if QWT_VER>=6
	m_pZoomer = new QwtPlotZoomer(plotN->canvas());
	m_pZoomer->setMaxStackDepth(-1);
	m_pZoomer->setEnabled(1);

	m_pZoomerX = new QwtPlotZoomer(plotX->canvas());
	m_pZoomerX->setMaxStackDepth(-1);
	m_pZoomerX->setEnabled(1);
#endif

	plotN->setAxisTitle(QwtPlot::xBottom, "Scattering Angle");
	plotN->setAxisTitle(QwtPlot::yLeft, "Intensity");
	plotX->setAxisTitle(QwtPlot::xBottom, "Scattering Angle");
	plotX->setAxisTitle(QwtPlot::yLeft, "Intensity");

	plotN->canvas()->setMouseTracking(1);
	m_pPicker = new QwtPlotPicker(plotN->xBottom, plotN->yLeft,
#if QWT_VER<6
		QwtPlotPicker::PointSelection,
#endif
		QwtPlotPicker::NoRubberBand,
#if QWT_VER>=6
		QwtPlotPicker::AlwaysOff,
#else
		QwtPlotPicker::AlwaysOn,
#endif
		plotN->canvas());

#if QWT_VER>=6
	m_pPicker->setStateMachine(new QwtPickerTrackerMachine());
	connect(m_pPicker, SIGNAL(moved(const QPointF&)), this, SLOT(cursorMoved(const QPointF&)));
#endif
	m_pPicker->setEnabled(1);


	plotX->canvas()->setMouseTracking(1);
	m_pPickerX = new QwtPlotPicker(plotX->xBottom, plotX->yLeft,
#if QWT_VER<6
		QwtPlotPicker::PointSelection,
#endif
		QwtPlotPicker::NoRubberBand,
#if QWT_VER>=6
		QwtPlotPicker::AlwaysOff,
#else
		QwtPlotPicker::AlwaysOn,
#endif
		plotX->canvas());

#if QWT_VER>=6
	m_pPickerX->setStateMachine(new QwtPickerTrackerMachine());
	connect(m_pPickerX, SIGNAL(moved(const QPointF&)), this, SLOT(cursorMoved(const QPointF&)));
#endif
	m_pPickerX->setEnabled(1);
	// -------------------------------------------------------------------------

	btnSave->setIcon(load_icon("res/document-save.svg"));
	btnLoad->setIcon(load_icon("res/document-open.svg"));

	tablePowderLines->horizontalHeader()->setVisible(true);
	tablePowderLines->verticalHeader()->setDefaultSectionSize(tablePowderLines->verticalHeader()->defaultSectionSize()*1.4);
	tablePowderLines->setColumnWidth(TABLE_ANGLE, 75);
	tablePowderLines->setColumnWidth(TABLE_Q, 75);
	tablePowderLines->setColumnWidth(TABLE_PEAK, 75);
	tablePowderLines->setColumnWidth(TABLE_MULT, 50);
	tablePowderLines->setColumnWidth(TABLE_FN, 75);
	tablePowderLines->setColumnWidth(TABLE_FX, 75);
	tablePowderLines->setColumnWidth(TABLE_IN, 75);
	tablePowderLines->setColumnWidth(TABLE_IX, 75);

	std::vector<QLineEdit*> vecEditsUC = {editA, editB, editC, editAlpha, editBeta, editGamma};
	for(QLineEdit* pEdit : vecEditsUC)
	{
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(CheckCrystalType()));
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(CalcPeaks()));
	}
	QObject::connect(spinLam, SIGNAL(valueChanged(double)), this, SLOT(CalcPeaks()));
	QObject::connect(spinOrder, SIGNAL(valueChanged(int)), this, SLOT(CalcPeaks()));
	QObject::connect(checkUniquePeaks, SIGNAL(toggled(bool)), this, SLOT(CalcPeaks()));

	QObject::connect(editSpaceGroupsFilter, SIGNAL(textChanged(const QString&)), this, SLOT(RepopulateSpaceGroups()));
	QObject::connect(comboSpaceGroups, SIGNAL(currentIndexChanged(int)), this, SLOT(SpaceGroupChanged()));

	connect(btnSaveTable, SIGNAL(clicked()), this, SLOT(SaveTable()));
	connect(btnSave, SIGNAL(clicked()), this, SLOT(SavePowder()));
	connect(btnLoad, SIGNAL(clicked()), this, SLOT(LoadPowder()));
	connect(btnAtoms, SIGNAL(clicked()), this, SLOT(ShowAtomDlg()));

	m_bDontCalc = 0;
	RepopulateSpaceGroups();
	CalcPeaks();


	if(m_pSettings && m_pSettings->contains("powder/geo"))
		restoreGeometry(m_pSettings->value("powder/geo").toByteArray());
}

PowderDlg::~PowderDlg()
{
	for(QwtPlotGrid** pGrid : {&m_pGrid, &m_pGridX})
	{
		if(*pGrid)
		{
			delete *pGrid;
			*pGrid = nullptr;
		}
	}

	for(QwtPlotZoomer** pZoom : {&m_pZoomer, &m_pZoomerX})
	{
		if(*pZoom)
		{
			delete *pZoom;
			*pZoom = nullptr;
		}
	}
}


void PowderDlg::PlotPowderLines(const std::vector<const PowderLine*>& vecLines)
{
	const unsigned int NUM_POINTS = 512;

	using t_iter = typename std::vector<const PowderLine*>::const_iterator;
	std::pair<t_iter, t_iter> pairMinMax =
		boost::minmax_element(vecLines.begin(), vecLines.end(),
		[](const PowderLine* pLine1, const PowderLine* pLine2) ->bool
		{
			return pLine1->dAngle <= pLine2->dAngle;
		});

	double dMinTT = 0., dMaxTT = 0.;
	if(pairMinMax.first!=vecLines.end() && pairMinMax.second!=vecLines.end())
	{
		dMinTT = (*pairMinMax.first)->dAngle;
		dMaxTT = (*pairMinMax.second)->dAngle;

		dMinTT -= 10./180.*M_PI;
		dMaxTT += 10./180.*M_PI;
		if(dMinTT < 0.) dMinTT = 0.;
	}

	m_vecTT.clear();
	m_vecTTx.clear();
	m_vecInt.clear();
	m_vecIntx.clear();

	m_vecTT.reserve(NUM_POINTS);
	m_vecTTx.reserve(NUM_POINTS);
	m_vecInt.reserve(NUM_POINTS);
	m_vecIntx.reserve(NUM_POINTS);

	for(unsigned int iPt=0; iPt<NUM_POINTS; ++iPt)
	{
		double dTT = (dMinTT + (dMaxTT - dMinTT)/double(NUM_POINTS)*double(iPt));

		double dInt = 0., dIntX = 0.;
		for(const PowderLine *pLine : vecLines)
		{
			const double dPeakX = pLine->dAngle;

			double dPeakInt = pLine->dIn;
			double dPeakIntX = pLine->dIx;
			if(dPeakInt < 0.) dPeakInt = 1.;
			if(dPeakIntX < 0.) dPeakIntX = 1.;

			constexpr double dSig = 0.25;
			dInt += tl::gauss_model(dTT/M_PI*180., dPeakX/M_PI*180., dSig, dPeakInt, 0.);
			dIntX += tl::gauss_model(dTT/M_PI*180., dPeakX/M_PI*180., dSig, dPeakIntX, 0.);
		}

		m_vecTT.push_back(dTT /M_PI*180.);
		m_vecTTx.push_back(dTT /M_PI*180.);
		m_vecInt.push_back(dInt);
		m_vecIntx.push_back(dIntX);
	}

#if QWT_VER>=6
	m_pCurve->setRawSamples(m_vecTT.data(), m_vecInt.data(), m_vecTT.size());
	m_pCurveX->setRawSamples(m_vecTTx.data(), m_vecIntx.data(), m_vecTTx.size());
#else
	m_pCurve->setRawData(m_vecTT.data(), m_vecInt.data(), m_vecTT.size());
	m_pCurveX->setRawData(m_vecTTx.data(), m_vecIntx.data(), m_vecTTx.size());
#endif

	set_zoomer_base(m_pZoomer, m_vecTT, m_vecInt);
	set_zoomer_base(m_pZoomerX, m_vecTTx, m_vecIntx);

	plotN->replot();
	plotX->replot();
}


void PowderDlg::CalcPeaks()
{
	try
	{
		if(m_bDontCalc) return;
		static const unsigned int iPrec = g_iPrec;
		const bool bWantUniquePeaks = checkUniquePeaks->isChecked();

		const double dA = editA->text().toDouble();
		const double dB = editB->text().toDouble();
		const double dC = editC->text().toDouble();
		const double dAlpha = editAlpha->text().toDouble()/180.*M_PI;
		const double dBeta = editBeta->text().toDouble()/180.*M_PI;
		const double dGamma = editGamma->text().toDouble()/180.*M_PI;

		const double dLam = spinLam->value();
		const int iOrder = spinOrder->value();
		//tl::log_debug("Lambda = ", dLam, ", order = ", iOrder);

		tl::Lattice<double> lattice(dA, dB, dC, dAlpha, dBeta, dGamma);
		tl::Lattice<double> recip = lattice.GetRecip();

		const ublas::matrix<double> matA = lattice.GetMetric();
		const ublas::matrix<double> matB = recip.GetMetric();

		const SpaceGroup *pSpaceGroup = GetCurSpaceGroup();


		// ----------------------------------------------------------------------------
		// structure factor stuff
		ScatlenList lstsl;
		FormfactList lstff;

		std::vector<ublas::vector<double>> vecAllAtoms;
		std::vector<std::complex<double>> vecScatlens;
		std::vector<double> vecFormfacts;
		std::vector<std::string> vecElems;

		const std::vector<ublas::matrix<double>>* pvecSymTrafos = nullptr;
		if(pSpaceGroup)
			pvecSymTrafos = &pSpaceGroup->GetTrafos();

		if(pvecSymTrafos && pvecSymTrafos->size() && g_bHasFormfacts && g_bHasScatlens && m_vecAtoms.size())
		{
			for(unsigned int iAtom=0; iAtom<m_vecAtoms.size(); ++iAtom)
			{
				ublas::vector<double> vecAtom = m_vecAtoms[iAtom].vecPos;
				// homogeneous coordinates
				vecAtom.resize(4,1); vecAtom[3] = 1.;
				const std::string& strElem = m_vecAtoms[iAtom].strAtomName;

				std::vector<ublas::vector<double>> vecSymPos =
					tl::generate_atoms<ublas::matrix<double>, ublas::vector<double>, std::vector>
						(*pvecSymTrafos, vecAtom);

				const ScatlenList::elem_type* pElem = lstsl.Find(strElem);
				if(!pElem)
				{
					tl::log_err("Element \"", strElem, "\" not found in scattering length table.");
					vecAllAtoms.clear();
					vecScatlens.clear();
					break;
				}


				const std::complex<double> b = pElem->GetCoherent()/* / 10.*/;

				for(ublas::vector<double> vecThisAtom : vecSymPos)
				{
					vecThisAtom.resize(3,1);
					// converts from fractional coordinates
					vecThisAtom = matA*vecThisAtom;
					vecAllAtoms.push_back(std::move(vecThisAtom));
					vecScatlens.push_back(b);
					vecElems.push_back(strElem);
				}
			}
		}
		// ----------------------------------------------------------------------------


		std::map<std::string, PowderLine> mapPeaks;
		tl::Powder<int> powder;
		powder.SetRecipLattice(&recip);

		for(int ih=iOrder; ih>=-iOrder; --ih)
			for(int ik=iOrder; ik>=-iOrder; --ik)
				for(int il=iOrder; il>=-iOrder; --il)
				{
					if(ih==0 && ik==0 && il==0) continue;
					if(pSpaceGroup && !pSpaceGroup->HasReflection(ih, ik, il))
						continue;


					ublas::vector<double> vecBragg = recip.GetPos(ih, ik, il);
					double dQ = ublas::norm_2(vecBragg);
					if(tl::is_nan_or_inf<double>(dQ)) continue;

					double dAngle = tl::bragg_recip_twotheta(dQ/tl::angstrom, dLam*tl::angstrom, 1.) / tl::radians;
					if(tl::is_nan_or_inf<double>(dAngle)) continue;

					//std::cout << "Q = " << dQ << ", angle = " << (dAngle/M_PI*180.) << std::endl;


					ublas::vector<double> vechkl = tl::make_vec({double(ih), double(ik), double(il)});
					double dF = -1., dI = -1.;
					double dFx = -1., dIx = -1.;

					// ----------------------------------------------------------------------------
					// structure factor stuff
					if(vecScatlens.size())
					{
						std::complex<double> cF =
							tl::structfact<double, std::complex<double>, ublas::vector<double>, std::vector>
								(vecAllAtoms, vecBragg, vecScatlens);
						double dFsq = (std::conj(cF)*cF).real();
						dF = std::sqrt(dFsq);
						tl::set_eps_0(dF, g_dEps);

						double dLor = tl::lorentz_factor(dAngle);
						dI = dFsq*dLor;
					}



					vecFormfacts.clear();
					if(g_bHasFormfacts)
					{
						for(unsigned int iAtom=0; iAtom<vecAllAtoms.size(); ++iAtom)
						{
							//const t_vec& vecAtom = vecAllAtoms[iAtom];
							const FormfactList::elem_type* pElemff = lstff.Find(vecElems[iAtom]);

							if(pElemff == nullptr)
							{
								tl::log_err("Cannot get form factor for \"", vecElems[iAtom], "\".");
								vecFormfacts.clear();
								break;
							}

							double dFF = pElemff->GetFormfact(dQ);
							vecFormfacts.push_back(dFF);
						}
					}

					if(vecFormfacts.size())
					{
						std::complex<double> cFx =
							tl::structfact<double, double, ublas::vector<double>, std::vector>
								(vecAllAtoms, vecBragg, vecFormfacts);

						double dFxsq = (std::conj(cFx)*cFx).real();
						dFx = std::sqrt(dFxsq);
						tl::set_eps_0(dFx, g_dEps);

						double dLor = tl::lorentz_factor(dAngle)*tl::lorentz_pol_factor(dAngle);
						dIx = dFxsq*dLor;
					}
					// ----------------------------------------------------------------------------


					bool bHasPeak = 0;
					if(bWantUniquePeaks)
						bHasPeak = powder.HasUniquePeak(ih, ik, il, dF);
					powder.AddPeak(ih, ik, il, dF);
					if(bHasPeak)
						continue;


					// using angle and F as hash for the set
					std::ostringstream ostrAngle;
					ostrAngle.precision(iPrec);
					ostrAngle << tl::r2d(dAngle) << " " << dF;
					std::string strAngle = ostrAngle.str();
					//std::cout << strAngle << std::endl;

					std::ostringstream ostrPeak;
					ostrPeak << "(" << /*std::abs*/(ih) << /*std::abs*/(ik) << /*std::abs*/(il) << ")";

					if(mapPeaks[ostrAngle.str()].strPeaks.length()!=0)
						mapPeaks[strAngle].strPeaks += ", ";

					mapPeaks[strAngle].strPeaks += ostrPeak.str();
					mapPeaks[strAngle].dAngle = dAngle;
					mapPeaks[strAngle].dQ = dQ;

					mapPeaks[strAngle].h = /*std::abs*/(ih);
					mapPeaks[strAngle].k = /*std::abs*/(ik);
					mapPeaks[strAngle].l = /*std::abs*/(il);

					mapPeaks[strAngle].dFn = dF;
					mapPeaks[strAngle].dIn = dI;
					mapPeaks[strAngle].dFx = dFx;
					mapPeaks[strAngle].dIx = dIx;
				}

		//std::cout << powder << std::endl;
		std::vector<const PowderLine*> vecPowderLines;
		//std::cout << "number of peaks: " << mapPeaks.size() << std::endl;
		vecPowderLines.reserve(mapPeaks.size());


		for(auto& pair : mapPeaks)
		{
			pair.second.strAngle = tl::var_to_str<double>(tl::r2d(pair.second.dAngle), iPrec);
			pair.second.strQ = tl::var_to_str<double>(pair.second.dQ, iPrec);
			pair.second.iMult = powder.GetMultiplicity(pair.second.h, pair.second.k, pair.second.l, pair.second.dFn);

			pair.second.dIn *= double(pair.second.iMult);
			pair.second.dIx *= double(pair.second.iMult);

			vecPowderLines.push_back(&pair.second);
		}


		std::sort(vecPowderLines.begin(), vecPowderLines.end(),
			[](const PowderLine* pLine1, const PowderLine* pLine2) -> bool
				{ return pLine1->dAngle < pLine2->dAngle; });


		const int iNumRows = vecPowderLines.size();
		tablePowderLines->setRowCount(iNumRows);

		for(int iRow=0; iRow<iNumRows; ++iRow)
		{
			for(int iCol=0; iCol<8; ++iCol)
			{
				if(!tablePowderLines->item(iRow, iCol))
					tablePowderLines->setItem(iRow, iCol, new QTableWidgetItem());
			}

			QString strMult = tl::var_to_str(vecPowderLines[iRow]->iMult).c_str();
			QString strFn, strIn, strFx, strIx;
			if(vecPowderLines[iRow]->dFn >= 0.)
				strFn = tl::var_to_str(vecPowderLines[iRow]->dFn, iPrec).c_str();
			if(vecPowderLines[iRow]->dIn >= 0.)
				strIn = tl::var_to_str(vecPowderLines[iRow]->dIn, iPrec).c_str();
			if(vecPowderLines[iRow]->dFx >= 0.)
				strFx = tl::var_to_str(vecPowderLines[iRow]->dFx, iPrec).c_str();
			if(vecPowderLines[iRow]->dIx >= 0.)
				strIx = tl::var_to_str(vecPowderLines[iRow]->dIx, iPrec).c_str();

			tablePowderLines->item(iRow, TABLE_ANGLE)->setText(vecPowderLines[iRow]->strAngle.c_str());
			tablePowderLines->item(iRow, TABLE_Q)->setText(vecPowderLines[iRow]->strQ.c_str());
			tablePowderLines->item(iRow, TABLE_PEAK)->setText(vecPowderLines[iRow]->strPeaks.c_str());
			tablePowderLines->item(iRow, TABLE_MULT)->setText(strMult);
			tablePowderLines->item(iRow, TABLE_FN)->setText(strFn);
			tablePowderLines->item(iRow, TABLE_IN)->setText(strIn);
			tablePowderLines->item(iRow, TABLE_FX)->setText(strFx);
			tablePowderLines->item(iRow, TABLE_IX)->setText(strIx);
		}

		PlotPowderLines(vecPowderLines);
		labelStatus->setText("OK.");
	}
	catch(const std::exception& ex)
	{
		//labelStatus->setText(QString("Error: ") + ex.what());
		labelStatus->setText("Error.");
		tl::log_err("Cannot calculate powder peaks: ", ex.what());
	}
}


const SpaceGroup* PowderDlg::GetCurSpaceGroup() const
{
	SpaceGroup *pSpaceGroup = 0;
	int iSpaceGroupIdx = comboSpaceGroups->currentIndex();
	if(iSpaceGroupIdx != 0)
		pSpaceGroup = (SpaceGroup*)comboSpaceGroups->itemData(iSpaceGroupIdx).value<void*>();
	return pSpaceGroup;
}

void PowderDlg::SpaceGroupChanged()
{
	m_crystalsys = CrystalSystem::CRYS_NOT_SET;
	std::string strCryTy = "<not set>";

	const SpaceGroup *pSpaceGroup = GetCurSpaceGroup();
	if(pSpaceGroup)
	{
		m_crystalsys = pSpaceGroup->GetCrystalSystem();
		strCryTy = pSpaceGroup->GetCrystalSystemName();
	}
	editCrystalSystem->setText(strCryTy.c_str());

	CheckCrystalType();
	CalcPeaks();
}

void PowderDlg::RepopulateSpaceGroups()
{
	if(!m_pmapSpaceGroups)
		return;

	for(int iCnt=comboSpaceGroups->count()-1; iCnt>0; --iCnt)
		comboSpaceGroups->removeItem(iCnt);

	std::string strFilter = editSpaceGroupsFilter->text().toStdString();

	for(const t_mapSpaceGroups::value_type& pair : *m_pmapSpaceGroups)
	{
		const std::string& strName = pair.second.GetName();

		typedef const boost::iterator_range<std::string::const_iterator> t_striterrange;
		if(strFilter!="" &&
				!boost::ifind_first(t_striterrange(strName.begin(), strName.end()),
									t_striterrange(strFilter.begin(), strFilter.end())))
			continue;

		comboSpaceGroups->insertItem(comboSpaceGroups->count(),
									strName.c_str(),
									QVariant::fromValue((void*)&pair.second));
	}
}

void PowderDlg::CheckCrystalType()
{
	set_crystal_system_edits(m_crystalsys, editA, editB, editC,
		editAlpha, editBeta, editGamma);
}

void PowderDlg::SavePowder()
{
	const std::string strXmlRoot("taz/");

	QString strDirLast = ".";
	if(m_pSettings)
		m_pSettings->value("powder/last_dir", ".").toString();
	QString qstrFile = QFileDialog::getSaveFileName(this,
								"Save Powder Configuration",
								strDirLast,
								"TAZ files (*.taz *.TAZ)");

	if(qstrFile == "")
		return;

	std::string strFile = qstrFile.toStdString();
	std::string strDir = tl::get_dir(strFile);

	std::map<std::string, std::string> mapConf;
	Save(mapConf, strXmlRoot);

	tl::Prop<std::string> xml;
	xml.Add(mapConf);

	bool bOk = xml.Save(strFile.c_str(), tl::PropType::XML);
	if(!bOk)
		QMessageBox::critical(this, "Error", "Could not save powder file.");

	if(bOk && m_pSettings)
		m_pSettings->setValue("powder/last_dir", QString(strDir.c_str()));
}

void PowderDlg::LoadPowder()
{
	const std::string strXmlRoot("taz/");

	QString strDirLast = ".";
	if(m_pSettings)
		strDirLast = m_pSettings->value("powder/last_dir", ".").toString();
	QString qstrFile = QFileDialog::getOpenFileName(this,
							"Open Powder Configuration",
							strDirLast,
							"TAZ files (*.taz *.TAZ)");
	if(qstrFile == "")
		return;


	std::string strFile = qstrFile.toStdString();
	std::string strDir = tl::get_dir(strFile);

	tl::Prop<std::string> xml;
	if(!xml.Load(strFile.c_str(), tl::PropType::XML))
	{
		QMessageBox::critical(this, "Error", "Could not load powder file.");
		return;
	}

	Load(xml, strXmlRoot);
	if(m_pSettings)
		m_pSettings->setValue("powder/last_dir", QString(strDir.c_str()));
}

void PowderDlg::Save(std::map<std::string, std::string>& mapConf, const std::string& strXmlRoot)
{
	mapConf[strXmlRoot + "sample/a"] = editA->text().toStdString();
	mapConf[strXmlRoot + "sample/b"] = editB->text().toStdString();
	mapConf[strXmlRoot + "sample/c"] = editC->text().toStdString();

	mapConf[strXmlRoot + "sample/alpha"] = editAlpha->text().toStdString();
	mapConf[strXmlRoot + "sample/beta"] = editBeta->text().toStdString();
	mapConf[strXmlRoot + "sample/gamma"] = editGamma->text().toStdString();

	mapConf[strXmlRoot + "powder/maxhkl"] = tl::var_to_str<int>(spinOrder->value());
	mapConf[strXmlRoot + "powder/lambda"] = tl::var_to_str<double>(spinLam->value());

	mapConf[strXmlRoot + "sample/spacegroup"] = comboSpaceGroups->currentText().toStdString();

	// atom positions
	mapConf[strXmlRoot + "sample/atoms/num"] = tl::var_to_str(m_vecAtoms.size());
	for(unsigned int iAtom=0; iAtom<m_vecAtoms.size(); ++iAtom)
	{
		const AtomPos& atom = m_vecAtoms[iAtom];

		std::string strAtomNr = tl::var_to_str(iAtom);
		mapConf[strXmlRoot + "sample/atoms/" + strAtomNr + "/name"] =
			atom.strAtomName;
		mapConf[strXmlRoot + "sample/atoms/" + strAtomNr + "/x"] =
			tl::var_to_str(atom.vecPos[0]);
		mapConf[strXmlRoot + "sample/atoms/" + strAtomNr + "/y"] =
			tl::var_to_str(atom.vecPos[1]);
		mapConf[strXmlRoot + "sample/atoms/" + strAtomNr + "/z"] =
			tl::var_to_str(atom.vecPos[2]);
	}
}

void PowderDlg::Load(tl::Prop<std::string>& xml, const std::string& strXmlRoot)
{
	m_bDontCalc = 1;
	bool bOk=0;

	editA->setText(std::to_string(xml.Query<double>((strXmlRoot + "sample/a").c_str(), 5., &bOk)).c_str());
	editB->setText(std::to_string(xml.Query<double>((strXmlRoot + "sample/b").c_str(), 5., &bOk)).c_str());
	editC->setText(std::to_string(xml.Query<double>((strXmlRoot + "sample/c").c_str(), 5., &bOk)).c_str());

	editAlpha->setText(std::to_string(xml.Query<double>((strXmlRoot + "sample/alpha").c_str(), 90., &bOk)).c_str());
	editBeta->setText(std::to_string(xml.Query<double>((strXmlRoot + "sample/beta").c_str(), 90., &bOk)).c_str());
	editGamma->setText(std::to_string(xml.Query<double>((strXmlRoot + "sample/gamma").c_str(), 90., &bOk)).c_str());

	spinOrder->setValue(xml.Query<int>((strXmlRoot + "powder/maxhkl").c_str(), 10, &bOk));
	spinLam->setValue(xml.Query<double>((strXmlRoot + "powder/lambda").c_str(), 5., &bOk));

	std::string strSpaceGroup = xml.Query<std::string>((strXmlRoot + "sample/spacegroup").c_str(), "", &bOk);
	tl::trim(strSpaceGroup);
	if(bOk)
	{
		editSpaceGroupsFilter->clear();
		int iSGIdx = comboSpaceGroups->findText(strSpaceGroup.c_str());
		if(iSGIdx >= 0)
			comboSpaceGroups->setCurrentIndex(iSGIdx);
	}


	// atom positions
	m_vecAtoms.clear();
	unsigned int iNumAtoms = xml.Query<unsigned int>((strXmlRoot + "sample/atoms/num").c_str(), 0, &bOk);
	if(bOk)
	{
		m_vecAtoms.reserve(iNumAtoms);

		for(unsigned int iAtom=0; iAtom<iNumAtoms; ++iAtom)
		{
			AtomPos atom;
			atom.vecPos.resize(3,0);

			std::string strNr = tl::var_to_str(iAtom);
			atom.strAtomName = xml.Query<std::string>((strXmlRoot + "sample/atoms/" + strNr + "/name").c_str(), "");
			atom.vecPos[0] = xml.Query<double>((strXmlRoot + "sample/atoms/" + strNr + "/x").c_str(), 0.);
			atom.vecPos[1] = xml.Query<double>((strXmlRoot + "sample/atoms/" + strNr + "/y").c_str(), 0.);
			atom.vecPos[2] = xml.Query<double>((strXmlRoot + "sample/atoms/" + strNr + "/z").c_str(), 0.);

			m_vecAtoms.push_back(atom);
		}
	}


	m_bDontCalc = 0;
	CalcPeaks();
}


void PowderDlg::SaveTable()
{
	QString strDirLast = m_pSettings ? m_pSettings->value("powder/last_dir_table", ".").toString() : ".";
	QString strFile = QFileDialog::getSaveFileName(this,
		"Save Table", strDirLast, "Data files (*.dat *.DAT)");

	if(strFile != "")
	{
		if(!save_table(strFile.toStdString().c_str(), tablePowderLines))
			QMessageBox::critical(this, "Error", "Could not save table data.");
	}
}


void PowderDlg::ApplyAtoms(const std::vector<AtomPos>& vecAtoms)
{
	m_vecAtoms = vecAtoms;
	CalcPeaks();
}

void PowderDlg::ShowAtomDlg()
{
	if(!m_pAtomsDlg)
	{
		m_pAtomsDlg = new AtomsDlg(this, m_pSettings);
		m_pAtomsDlg->setWindowTitle(m_pAtomsDlg->windowTitle() + QString(" (Powder)"));

		QObject::connect(m_pAtomsDlg, SIGNAL(ApplyAtoms(const std::vector<AtomPos>&)),
			this, SLOT(ApplyAtoms(const std::vector<AtomPos>&)));
	}

	m_pAtomsDlg->SetAtoms(m_vecAtoms);
	m_pAtomsDlg->show();
	m_pAtomsDlg->activateWindow();
}

void PowderDlg::cursorMoved(const QPointF& pt)
{
	const double dX = pt.x();
	const double dY = pt.y();

	const std::wstring strTh = tl::get_spec_char_utf16("theta");
	std::wostringstream ostr;
	ostr.precision(g_iPrecGfx);
	ostr << L"2" << strTh << L" = " << dX << L", ";
	ostr << L"I = " << dY;

	labelStatus->setText(QString::fromWCharArray(ostr.str().c_str()));
}

void PowderDlg::accept()
{
	if(m_pSettings)
		m_pSettings->setValue("powder/geo", saveGeometry());

	QDialog::accept();
}

void PowderDlg::showEvent(QShowEvent *pEvt)
{
	QDialog::showEvent(pEvt);
}

#include "PowderDlg.moc"
