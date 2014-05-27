/*
 * Spurion Dialog
 * @author Tobias Weber
 * @date 26-may-2014
 */

#include "SpurionDlg.h"
#include "../helper/neutrons.hpp"
#include "../helper/string.h"
#include "../helper/spec_char.h"

#include <sstream>
#include <iostream>


SpurionDlg::SpurionDlg(QWidget* pParent)
		: QDialog(pParent)
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

			double dE_sp = get_inelastic_spurion(bFixedEi, dE*one_meV,
										iOrderMono, iOrderAna) / one_meV;

			vecSpurions.push_back(dE_sp);

			std::ostringstream ostrInfo;
			ostrInfo << "Mono order: " << iOrderMono
					<< ", Ana order: " << iOrderAna;
			vecInfo.push_back(ostrInfo.str());
		}
	}
	else
	{
		for(unsigned int iOrderMono=1; iOrderMono<=iMaxOrder; ++iOrderMono)
		for(unsigned int iOrderAna=1; iOrderAna<=iMaxOrder; ++iOrderAna)
		{
			double dE_sp = get_inelastic_spurion(bFixedEi, dE*one_meV,
										iOrderMono, iOrderAna) / one_meV;
			vecSpurions.push_back(dE_sp);

			std::ostringstream ostrInfo;
			ostrInfo << "Mono order: " << iOrderMono
					<< ", Ana order: " << iOrderAna;
			vecInfo.push_back(ostrInfo.str());
		}
	}

	const std::string& strDelta = get_spec_char_utf8("Delta");
	const std::string& strBullet = get_spec_char_utf8("bullet");

	std::ostringstream ostr;
	ostr << "Spurious inelastic signals for " + strDelta + "E = \n\n";
	for(unsigned int i=0; i<vecSpurions.size(); ++i)
	{
		const double dE_Sp = vecSpurions[i];
		const std::string& strInfo = vecInfo[i];

		ostr << "  " << strBullet << " ";
		ostr << ::var_to_str(dE_Sp, 4) << " meV";
		ostr << " (" << strInfo << ")\n";
	}

	textSpurions->setPlainText(QString::fromUtf8(ostr.str().c_str(), ostr.str().size()));
}

void SpurionDlg::paramsChanged(const RecipParams& parms)
{
	typedef units::quantity<units::si::wavenumber> wavenumber;
	typedef units::quantity<units::si::energy> energy;

	wavenumber ki = parms.dki / angstrom;
	wavenumber kf = parms.dkf / angstrom;
	energy Ei = k2E(ki);
	energy Ef = k2E(kf);

	m_dEi = Ei / one_meV;
	m_dEf = Ef / one_meV;

	Calc();
}

#include "SpurionDlg.moc"
