/*
 * Real Space Parameters
 * @author tweber
 * @date 29-mar-2014
 */

#include "RealParamDlg.h"
#include "../helper/string.h"
#include "../helper/math.h"
#include "../helper/neutrons.hpp"


RealParamDlg::RealParamDlg(QWidget* pParent, QSettings* pSett)
	: QDialog(pParent), m_pSettings(pSett)
{
	this->setupUi(this);
}

RealParamDlg::~RealParamDlg()
{

}

void RealParamDlg::paramsChanged(const RealParams& parms)
{
	this->edit2ThetaM->setText(var_to_str<double>(parms.dMonoTT / M_PI * 180.).c_str());
	this->editThetaM->setText(var_to_str<double>(parms.dMonoT / M_PI * 180.).c_str());

	this->edit2ThetaS->setText(var_to_str<double>(parms.dSampleTT / M_PI * 180.).c_str());
	this->editThetaS->setText(var_to_str<double>(parms.dSampleT / M_PI * 180.).c_str());

	this->edit2ThetaA->setText(var_to_str<double>(parms.dAnaTT / M_PI * 180.).c_str());
	this->editThetaA->setText(var_to_str<double>(parms.dAnaT / M_PI * 180.).c_str());


	this->editLenMonoSample->setText(var_to_str<double>(parms.dLenMonoSample).c_str());
	this->editLenSampleAna->setText(var_to_str<double>(parms.dLenSampleAna).c_str());
	this->editLenAnaDet->setText(var_to_str<double>(parms.dLenAnaDet).c_str());
}


typedef units::quantity<units::si::wavenumber> wavenumber;
typedef units::quantity<units::si::energy> energy;
typedef units::quantity<units::si::length> length;
typedef units::quantity<units::si::velocity> velocity;

void RealParamDlg::closeEvent(QCloseEvent *pEvt)
{
	QDialog::closeEvent(pEvt);
}

void RealParamDlg::accept()
{
	if(m_pSettings)
		m_pSettings->setValue("real_params/geo", saveGeometry());

	QDialog::accept();
}

void RealParamDlg::showEvent(QShowEvent *pEvt)
{
	if(m_pSettings && m_pSettings->contains("real_params/geo"))
		restoreGeometry(m_pSettings->value("real_params/geo").toByteArray());

	QDialog::showEvent(pEvt);
}


#include "RealParamDlg.moc"
