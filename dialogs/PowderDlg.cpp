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


struct PowderLine
{
	double dAngle;
	std::string strAngle;

	double dQ;
	std::string strQ;

	std::string strPeaks;
};


PowderDlg::PowderDlg(QWidget* pParent, QSettings* pSett) 
			: QDialog(pParent), m_pSettings(pSett)
{
	this->setupUi(this);
	tablePowderLines->horizontalHeader()->setVisible(true);
	tablePowderLines->setColumnWidth(0, 75);
	tablePowderLines->setColumnWidth(1, 75);
	tablePowderLines->setColumnWidth(2, 250);
	
	std::vector<QLineEdit*> vecEdits = {editA, editB, editC, 
									editAlpha, editBeta, editGamma,
									editLam};
	for(QLineEdit* pEdit : vecEdits)
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(CalcPeaks()));
	QObject::connect(spinOrder, SIGNAL(valueChanged(int)), this, SLOT(CalcPeaks()));
	QObject::connect(comboSpaceGroups, SIGNAL(currentIndexChanged(int)), this, SLOT(CalcPeaks()));
	
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
	
	
	std::map<std::string, PowderLine> mapPeaks;

	for(int ih=0; ih<iOrder; ++ih)
		for(int ik=0; ik<iOrder; ++ik)
			for(int il=0; il<iOrder; ++il)
			{
				if(ih==0 && ik==0 && il ==0) continue;

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
