/*
 * Reciprocal Space Parameters
 * @author tweber
 * @date 26-mar-2014
 */

#include "RecipParamDlg.h"
#include "../helper/string.h"
#include "../helper/math.h"
#include "../helper/neutrons.hpp"


RecipParamDlg::RecipParamDlg(QWidget* pParent, QSettings* pSett)
	: QDialog(pParent), m_pSettings(pSett)
{
	this->setupUi(this);

	QObject::connect(editKi, SIGNAL(textChanged(const QString&)), this, SLOT(KiChanged()));
	QObject::connect(editKf, SIGNAL(textChanged(const QString&)), this, SLOT(KfChanged()));
}

RecipParamDlg::~RecipParamDlg()
{

}

void RecipParamDlg::paramsChanged(const RecipParams& parms)
{
	this->editKi->setText(var_to_str<double>(parms.dki).c_str());
	this->editKf->setText(var_to_str<double>(parms.dkf).c_str());
	this->editQ->setText(var_to_str<double>(parms.dQ).c_str());
	this->editq->setText(var_to_str<double>(parms.dq).c_str());
	this->editE->setText(var_to_str<double>(parms.dE).c_str());
	this->edit2Theta->setText(var_to_str<double>(parms.d2Theta / M_PI * 180.).c_str());
	this->editTheta->setText(var_to_str<double>(parms.dTheta / M_PI * 180.).c_str());
	this->editKiQ->setText(var_to_str<double>(parms.dKiQ / M_PI * 180.).c_str());
	this->editKfQ->setText(var_to_str<double>(parms.dKfQ / M_PI * 180.).c_str());
}


typedef units::quantity<units::si::wavenumber> wavenumber;
typedef units::quantity<units::si::energy> energy;
typedef units::quantity<units::si::length> length;
typedef units::quantity<units::si::velocity> velocity;

void RecipParamDlg::KiChanged()
{
	wavenumber ki = str_to_var<double>(editKi->text().toStdString()) / angstrom;
	energy Ei = k2E(ki);
	length lami = k2lam(ki);
	velocity vi = k2v(ki);

	editEi->setText(var_to_str<double>(Ei / one_meV).c_str());
	editLami->setText(var_to_str<double>(lami / angstrom).c_str());
	editVi->setText(var_to_str<double>(vi*units::si::seconds/units::si::meters).c_str());
}

void RecipParamDlg::KfChanged()
{
	wavenumber kf = str_to_var<double>(editKf->text().toStdString()) / angstrom;
	energy Ef = k2E(kf);
	length lamf = k2lam(kf);
	velocity vf = k2v(kf);

	editEf->setText(var_to_str<double>(Ef / one_meV).c_str());
	editLamf->setText(var_to_str<double>(lamf / angstrom).c_str());
	editVf->setText(var_to_str<double>(vf*units::si::seconds/units::si::meters).c_str());
}


void RecipParamDlg::closeEvent(QCloseEvent *pEvt)
{
	QDialog::closeEvent(pEvt);
}

void RecipParamDlg::accept()
{
	if(m_pSettings)
		m_pSettings->setValue("recip_params/geo", saveGeometry());

	QDialog::accept();
}

void RecipParamDlg::showEvent(QShowEvent *pEvt)
{
	if(m_pSettings && m_pSettings->contains("recip_params/geo"))
		restoreGeometry(m_pSettings->value("recip_params/geo").toByteArray());

	QDialog::showEvent(pEvt);
}


#include "RecipParamDlg.moc"