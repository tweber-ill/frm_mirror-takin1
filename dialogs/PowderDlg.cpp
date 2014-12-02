/*
 * Powder Line Dialog
 * @author Tobias Weber
 * @date 2013, 2-dec-2014
 */
 
#include "PowderDlg.h"
#include "../helper/lattice.h"
#include "../helper/neutrons.hpp"
#include "../helper/linalg.h"
#include "../helper/string.h"

#include <vector>
#include <string>
#include <iostream>
#include <boost/algorithm/string.hpp>


struct PowderLine
{
	double dAngle;
	std::string strAngle;

	double dQ;
	std::string strQ;

	std::string strPeaks;
};


PowderDlg::PowderDlg(QWidget* pParent, QSettings* pSett) 
			: QDialog(pParent), m_pSettings(pSett), 
				m_pmapSpaceGroups(get_space_groups())
{
	this->setupUi(this);
	tablePowderLines->horizontalHeader()->setVisible(true);
	tablePowderLines->setColumnWidth(0, 75);
	tablePowderLines->setColumnWidth(1, 75);
	tablePowderLines->setColumnWidth(2, 250);
	
	std::vector<QLineEdit*> vecEditsUC = {editA, editB, editC, editAlpha, editBeta, editGamma};
	for(QLineEdit* pEdit : vecEditsUC)
	{
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(CheckCrystalType()));
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(CalcPeaks()));
	}
	QObject::connect(editLam, SIGNAL(textEdited(const QString&)), this, SLOT(CalcPeaks()));
	QObject::connect(spinOrder, SIGNAL(valueChanged(int)), this, SLOT(CalcPeaks()));

	QObject::connect(editSpaceGroupsFilter, SIGNAL(textChanged(const QString&)), this, SLOT(RepopulateSpaceGroups()));
	QObject::connect(comboSpaceGroups, SIGNAL(currentIndexChanged(int)), this, SLOT(SpaceGroupChanged()));
	
	RepopulateSpaceGroups();
	CalcPeaks();
}

PowderDlg::~PowderDlg()
{}


void PowderDlg::CalcPeaks()
{
	static const unsigned int iPrec = 8;
	
	const double dA = editA->text().toDouble();
	const double dB = editB->text().toDouble();
	const double dC = editC->text().toDouble();
	const double dAlpha = editAlpha->text().toDouble()/180.*M_PI;
	const double dBeta = editBeta->text().toDouble()/180.*M_PI;
	const double dGamma = editGamma->text().toDouble()/180.*M_PI;
	
	const double dLam = editLam->text().toDouble();
	const int iOrder = spinOrder->value();
	
	Lattice<double> lattice(dA, dB, dC, dAlpha, dBeta, dGamma);
	Lattice<double> recip = lattice.GetRecip();
	
	const SpaceGroup *pSpaceGroup = GetCurSpaceGroup();
	
	std::map<std::string, PowderLine> mapPeaks;

	for(int ih=0; ih<iOrder; ++ih)
		for(int ik=0; ik<iOrder; ++ik)
			for(int il=0; il<iOrder; ++il)
			{
				if(ih==0 && ik==0 && il==0) continue;
				if(pSpaceGroup && !pSpaceGroup->HasReflection(ih, ik, il)) continue;

				ublas::vector<double> vecBragg = recip.GetPos(ih, ik, il);
				double dQ = ublas::norm_2(vecBragg);
				if(is_nan_or_inf<double>(dQ)) continue;
				
				double dAngle = bragg_recip_twotheta(dQ/angstrom, dLam*angstrom, 1.) / units::si::radians;
				if(is_nan_or_inf<double>(dAngle)) continue;
				
				//std::cout << "Q = " << dQ << ", angle = " << (dAngle/M_PI*180.) << std::endl;

				std::ostringstream ostrAngle;
				ostrAngle.precision(iPrec);
				ostrAngle << (dAngle/M_PI*180.);

				std::ostringstream ostrPeak;
				ostrPeak << "(" << ih << " " << ik << " " << il << ") ";

				mapPeaks[ostrAngle.str()].strPeaks += ostrPeak.str();
				mapPeaks[ostrAngle.str()].dAngle = dAngle;
				mapPeaks[ostrAngle.str()].dQ = dQ;
			}

	std::vector<const PowderLine*> vecPowderLines;
	vecPowderLines.reserve(mapPeaks.size());


	for(auto& pair : mapPeaks)
	{
		pair.second.strAngle = pair.first;
		pair.second.strQ = var_to_str<double>(pair.second.dQ, iPrec);
		
		vecPowderLines.push_back(&pair.second);
	}


	std::sort(vecPowderLines.begin(), vecPowderLines.end(), 
				[](const PowderLine* pLine1, const PowderLine* pLine2) -> bool
					{ return pLine1->dAngle <= pLine2->dAngle; });
					

	const int iNumRows = vecPowderLines.size();
	tablePowderLines->setRowCount(iNumRows);

	for(int iRow=0; iRow<iNumRows; ++iRow)
	{
		for(int iCol=0; iCol<3; ++iCol)
		{
			if(!tablePowderLines->item(iRow, iCol))
				tablePowderLines->setItem(iRow, iCol, new QTableWidgetItem());
		}

		tablePowderLines->item(iRow, 0)->setText(vecPowderLines[iRow]->strAngle.c_str());
		tablePowderLines->item(iRow, 1)->setText(vecPowderLines[iRow]->strQ.c_str());
		tablePowderLines->item(iRow, 2)->setText(vecPowderLines[iRow]->strPeaks.c_str());
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
	const char* pcCryTy = "<not set>";
	
	const SpaceGroup *pSpaceGroup = GetCurSpaceGroup();
	if(pSpaceGroup)
	{
		m_crystalsys = pSpaceGroup->GetCrystalSystem();
		pcCryTy = pSpaceGroup->GetCrystalSystemName();
	}
	editCrystalSystem->setText(pcCryTy);
	
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
	switch(m_crystalsys)
	{
		case CRYS_CUBIC:
			editA->setEnabled(1);
			editB->setEnabled(0);
			editC->setEnabled(0);
			editAlpha->setEnabled(0);
			editBeta->setEnabled(0);
			editGamma->setEnabled(0);

			editB->setText(editA->text());
			editC->setText(editA->text());
			editAlpha->setText("90");
			editBeta->setText("90");
			editGamma->setText("90");
			break;
			
		case CRYS_HEXAGONAL:
			editA->setEnabled(1);
			editB->setEnabled(0);
			editC->setEnabled(1);
			editAlpha->setEnabled(0);
			editBeta->setEnabled(0);
			editGamma->setEnabled(0);

			editB->setText(editA->text());
			editAlpha->setText("90");
			editBeta->setText("90");
			editGamma->setText("120");
			break;
			
		case CRYS_MONOCLINIC:
			editA->setEnabled(1);
			editB->setEnabled(1);
			editC->setEnabled(1);
			editAlpha->setEnabled(1);
			editBeta->setEnabled(0);
			editGamma->setEnabled(0);

			editBeta->setText("90");
			editGamma->setText("90");
			break;
			
		case CRYS_ORTHORHOMBIC:
			editA->setEnabled(1);
			editB->setEnabled(1);
			editC->setEnabled(1);
			editAlpha->setEnabled(0);
			editBeta->setEnabled(0);
			editGamma->setEnabled(0);

			editAlpha->setText("90");
			editBeta->setText("90");
			editGamma->setText("90");
			break;
			
		case CRYS_TETRAGONAL:
			editA->setEnabled(1);
			editB->setEnabled(0);
			editC->setEnabled(1);
			editAlpha->setEnabled(0);
			editBeta->setEnabled(0);
			editGamma->setEnabled(0);

			editB->setText(editA->text());
			editAlpha->setText("90");
			editBeta->setText("90");
			editGamma->setText("90");
			break;

		case CRYS_TRIGONAL:
			editA->setEnabled(1);
			editB->setEnabled(0);
			editC->setEnabled(0);
			editAlpha->setEnabled(1);
			editBeta->setEnabled(0);
			editGamma->setEnabled(0);

			editB->setText(editA->text());
			editC->setText(editA->text());
			editBeta->setText(editAlpha->text());
			editGamma->setText(editAlpha->text());
			break;

		case CRYS_TRICLINIC:
		case CRYS_NOT_SET:
		default:
			editA->setEnabled(1);
			editB->setEnabled(1);
			editC->setEnabled(1);
			editAlpha->setEnabled(1);
			editBeta->setEnabled(1);
			editGamma->setEnabled(1);
			break;
	}
}


void PowderDlg::accept()
{
	if(m_pSettings)
		m_pSettings->setValue("powder/geo", saveGeometry());

	QDialog::accept();
}

void PowderDlg::showEvent(QShowEvent *pEvt)
{
	if(m_pSettings && m_pSettings->contains("powder/geo"))
		restoreGeometry(m_pSettings->value("powder/geo").toByteArray());

	QDialog::showEvent(pEvt);
}

#include "PowderDlg.moc"