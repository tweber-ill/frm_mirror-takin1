/*
 * Goto Dialog
 * @author Tobias Weber
 * @date 15-oct-2014
 */

#include "GotoDlg.h"
#include "../helper/neutrons.hpp"
#include "../helper/lattice.h"
#include "../helper/string.h"


using energy = units::quantity<units::si::energy>;
using wavenumber = units::quantity<units::si::wavenumber>;


GotoDlg::GotoDlg(QWidget* pParent) : QDialog(pParent)
{
	this->setupUi(this);

	QObject::connect(editKi, SIGNAL(textEdited(const QString&)), this, SLOT(EditedKiKf()));
	QObject::connect(editKf, SIGNAL(textEdited(const QString&)), this, SLOT(EditedKiKf()));
	QObject::connect(editE, SIGNAL(textEdited(const QString&)), this, SLOT(EditedE()));

	std::vector<QObject*> vecObjs {editH, editK, editL};
	for(QObject* pObj : vecObjs)
		QObject::connect(pObj, SIGNAL(textEdited(const QString&)), this, SLOT(Calc()));
}

GotoDlg::~GotoDlg()
{}


void GotoDlg::Calc()
{
	bool bHOk=0, bKOk=0, bLOk=0, bKiOk=0, bKfOk=0;

	double dH = editH->text().toDouble(&bHOk);
	double dK = editK->text().toDouble(&bKOk);
	double dL = editL->text().toDouble(&bLOk);
	double dKi = editKi->text().toDouble(&bKiOk);
	double dKf = editKf->text().toDouble(&bKfOk);

	if(!bHOk || !bKOk || !bLOk || !bKiOk || !bKfOk)
		return;

	wavenumber ki = dKi / angstrom;
	wavenumber kf = dKf / angstrom;

	// TODO
}


void GotoDlg::EditedKiKf()
{
	bool bKiOk=0, bKfOk=0;
	double dKi = editKi->text().toDouble(&bKiOk);
	double dKf = editKf->text().toDouble(&bKfOk);
	if(!bKiOk || !bKfOk) return;

	energy Ei = k2E(dKi / angstrom);
	energy Ef = k2E(dKf / angstrom);

	energy E = Ei-Ef;
	double dE = E/one_meV;

	std::string strE = var_to_str<double>(dE);
	editE->setText(strE.c_str());

	Calc();
}

void GotoDlg::EditedE()
{
	bool bOk = 0;
	double dE = editE->text().toDouble(&bOk);
	if(!bOk) return;
	energy E = dE * one_meV;

	bool bImag=0;
	wavenumber k_E = E2k(E, bImag);
	double dSign = 1.;
	if(bImag) dSign = -1.;

	if(radioFixedKi->isChecked())
	{
		bool bKOk = 0;
		double dKi = editKi->text().toDouble(&bKOk);
		if(!bKOk) return;

		wavenumber ki = dKi / angstrom;
		wavenumber kf = units::sqrt(ki*ki - dSign*k_E*k_E);

		double dKf = kf*angstrom;
		std::string strKf = var_to_str<double>(dKf);
		editKf->setText(strKf.c_str());
	}
	else
	{
		bool bKOk = 0;
		double dKf = editKf->text().toDouble(&bKOk);
		if(!bKOk) return;

		wavenumber kf = dKf / angstrom;
		wavenumber ki = units::sqrt(kf*kf + dSign*k_E*k_E);

		double dKi = ki*angstrom;
		std::string strKi = var_to_str<double>(dKi);
		editKi->setText(strKi.c_str());
	}

	Calc();
}


#include "GotoDlg.moc"
