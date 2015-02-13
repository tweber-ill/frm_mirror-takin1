/*
 * Spurion Dialog
 * @author Tobias Weber
 * @date 26-may-2014
 * @copyright GPLv2
 */

#include "SpurionDlg.h"
#include "../tlibs/math/neutrons.hpp"
#include "../tlibs/string/string.h"
#include "../tlibs/string/spec_char.h"

#include <sstream>
#include <iostream>


SpurionDlg::SpurionDlg(QWidget* pParent, QSettings *pSett)
		: QDialog(pParent), m_pSettings(pSett)
{
	setupUi(this);

	QObject::connect(radioFixedEi, SIGNAL(toggled(bool)),
					this, SLOT(ChangedKiKfMode()));

	QObject::connect(radioFixedEi, SIGNAL(toggled(bool)), this, SLOT(Calc()));
	QObject::connect(checkFilter, SIGNAL(toggled(bool)), this, SLOT(Calc()));
	QObject::connect(btnSync, SIGNAL(toggled(bool)), this, SLOT(Calc()));
	QObject::connect(spinE, SIGNAL(valueChanged(double)), this, SLOT(Calc()));
	QObject::connect(spinOrder, SIGNAL(valueChanged(int)), this, SLOT(Calc()));

	Calc();
}

SpurionDlg::~SpurionDlg()
{}


void SpurionDlg::ChangedKiKfMode()
{
	if(radioFixedEi->isChecked())
		labelE->setText("E_i (meV):");
	else
		labelE->setText("E_f (meV):");
}

void SpurionDlg::Calc()
{
	const bool bFixedEi = radioFixedEi->isChecked();

	if(btnSync->isChecked())
	{
		const double dSyncedE = bFixedEi ? m_dEi : m_dEf;
		spinE->setValue(dSyncedE);
	}

	double dE = spinE->value();

	const unsigned int iMaxOrder = (unsigned int)spinOrder->value();
	const bool bFilter = checkFilter->isChecked();

	std::vector<double> vecSpurions;
	std::vector<std::string> vecInfo;

	if(bFilter)
	{
		for(unsigned int iOrder=1; iOrder<=iMaxOrder; ++iOrder)
		{
			unsigned int iOrderMono=1, iOrderAna=1;
			if(bFixedEi)
				iOrderAna = iOrder;
			else
				iOrderMono = iOrder;

			double dE_sp = tl::get_inelastic_spurion(bFixedEi, dE*tl::one_meV,
										iOrderMono, iOrderAna) / tl::one_meV;

			if(dE_sp != 0.)
			{
				vecSpurions.push_back(dE_sp);

				std::ostringstream ostrInfo;
				ostrInfo << "Mono order: " << iOrderMono
						<< ", Ana order: " << iOrderAna;
				vecInfo.push_back(ostrInfo.str());
			}
		}
	}
	else
	{
		for(unsigned int iOrderMono=1; iOrderMono<=iMaxOrder; ++iOrderMono)
		for(unsigned int iOrderAna=1; iOrderAna<=iMaxOrder; ++iOrderAna)
		{
			double dE_sp = tl::get_inelastic_spurion(bFixedEi, dE*tl::one_meV,
										iOrderMono, iOrderAna) / tl::one_meV;

			if(dE_sp != 0.)
			{
				vecSpurions.push_back(dE_sp);

				std::ostringstream ostrInfo;
				ostrInfo << "Mono order: " << iOrderMono
						<< ", Ana order: " << iOrderAna;
				vecInfo.push_back(ostrInfo.str());
			}
		}
	}

	const std::string& strDelta = tl::get_spec_char_utf8("Delta");
	const std::string& strBullet = tl::get_spec_char_utf8("bullet");

	std::ostringstream ostr;
	ostr << "Spurious inelastic signals for " + strDelta + "E = \n\n";
	for(unsigned int i=0; i<vecSpurions.size(); ++i)
	{
		const double dE_Sp = vecSpurions[i];
		const std::string& strInfo = vecInfo[i];

		ostr << "  " << strBullet << " ";
		ostr << tl::var_to_str(dE_Sp, 4) << " meV";
		ostr << " (" << strInfo << ")\n";
	}

	textSpurions->setPlainText(QString::fromUtf8(ostr.str().c_str(), ostr.str().size()));
}

void SpurionDlg::paramsChanged(const RecipParams& parms)
{
	tl::wavenumber ki = parms.dki / tl::angstrom;
	tl::wavenumber kf = parms.dkf / tl::angstrom;
	tl::energy Ei = tl::k2E(ki);
	tl::energy Ef = tl::k2E(kf);

	m_dEi = Ei / tl::one_meV;
	m_dEf = Ef / tl::one_meV;

	Calc();
}


void SpurionDlg::accept()
{
	if(m_pSettings)
		m_pSettings->setValue("spurions/geo", saveGeometry());

	QDialog::accept();
}

void SpurionDlg::showEvent(QShowEvent *pEvt)
{
	if(m_pSettings && m_pSettings->contains("spurions/geo"))
		restoreGeometry(m_pSettings->value("spurions/geo").toByteArray());

	QDialog::showEvent(pEvt);
}

#include "SpurionDlg.moc"
