/*
 * Real Space Parameters
 * @author tweber
 * @date 29-mar-2014
 * @license GPLv2
 */

#include "RealParamDlg.h"
#include "tlibs/string/string.h"
#include "tlibs/math/math.h"
#include "tlibs/math/neutrons.hpp"
#include "helper/globals.h"


RealParamDlg::RealParamDlg(QWidget* pParent, QSettings* pSett)
	: QDialog(pParent), m_pSettings(pSett)
{
	this->setupUi(this);


	if(m_pSettings && m_pSettings->contains("real_params/geo"))
		restoreGeometry(m_pSettings->value("real_params/geo").toByteArray());
}

RealParamDlg::~RealParamDlg()
{}

void RealParamDlg::paramsChanged(const RealParams& parms)
{
	this->edit2ThetaM->setText(tl::var_to_str<double>(parms.dMonoTT / M_PI * 180., g_iPrec).c_str());
	this->editThetaM->setText(tl::var_to_str<double>(parms.dMonoT / M_PI * 180., g_iPrec).c_str());

	this->edit2ThetaS->setText(tl::var_to_str<double>(parms.dSampleTT / M_PI * 180., g_iPrec).c_str());
	this->editThetaS->setText(tl::var_to_str<double>(parms.dSampleT / M_PI * 180., g_iPrec).c_str());

	this->edit2ThetaA->setText(tl::var_to_str<double>(parms.dAnaTT / M_PI * 180., g_iPrec).c_str());
	this->editThetaA->setText(tl::var_to_str<double>(parms.dAnaT / M_PI * 180., g_iPrec).c_str());


	this->editLenMonoSample->setText(tl::var_to_str<double>(parms.dLenMonoSample, g_iPrec).c_str());
	this->editLenSampleAna->setText(tl::var_to_str<double>(parms.dLenSampleAna, g_iPrec).c_str());
	this->editLenAnaDet->setText(tl::var_to_str<double>(parms.dLenAnaDet, g_iPrec).c_str());
}


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
	QDialog::showEvent(pEvt);
}


#include "RealParamDlg.moc"
