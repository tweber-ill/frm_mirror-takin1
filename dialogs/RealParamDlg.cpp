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

using t_real = t_real_glob;


RealParamDlg::RealParamDlg(QWidget* pParent, QSettings* pSett)
	: QDialog(pParent), m_pSettings(pSett)
{
	this->setupUi(this);
	if(m_pSettings)
	{
		QFont font;
		if(m_pSettings->contains("main/font_gen") && font.fromString(m_pSettings->value("main/font_gen", "").toString()))
			setFont(font);
	}

	if(m_pSettings && m_pSettings->contains("real_params/geo"))
		restoreGeometry(m_pSettings->value("real_params/geo").toByteArray());
}

RealParamDlg::~RealParamDlg()
{}

void RealParamDlg::paramsChanged(const RealParams& parms)
{
	this->edit2ThetaM->setText(tl::var_to_str<t_real>(tl::r2d(parms.dMonoTT), g_iPrec).c_str());
	this->editThetaM->setText(tl::var_to_str<t_real>(tl::r2d(parms.dMonoT), g_iPrec).c_str());

	this->edit2ThetaS->setText(tl::var_to_str<t_real>(tl::r2d(parms.dSampleTT), g_iPrec).c_str());
	this->editThetaS->setText(tl::var_to_str<t_real>(tl::r2d(parms.dSampleT), g_iPrec).c_str());

	this->edit2ThetaA->setText(tl::var_to_str<t_real>(tl::r2d(parms.dAnaTT), g_iPrec).c_str());
	this->editThetaA->setText(tl::var_to_str<t_real>(tl::r2d(parms.dAnaT), g_iPrec).c_str());


	this->editLenMonoSample->setText(tl::var_to_str<t_real>(parms.dLenMonoSample, g_iPrec).c_str());
	this->editLenSampleAna->setText(tl::var_to_str<t_real>(parms.dLenSampleAna, g_iPrec).c_str());
	this->editLenAnaDet->setText(tl::var_to_str<t_real>(parms.dLenAnaDet, g_iPrec).c_str());
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
