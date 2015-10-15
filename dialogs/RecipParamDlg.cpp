/*
 * Reciprocal Space Parameters
 * @author tweber
 * @date 26-mar-2014
 * @license GPLv2
 */

#include "RecipParamDlg.h"
#include "tlibs/string/string.h"
#include "tlibs/math/math.h"
#include "tlibs/math/linalg.h"
#include "tlibs/math/neutrons.hpp"
#include "helper/globals.h"

namespace ublas = boost::numeric::ublas;


RecipParamDlg::RecipParamDlg(QWidget* pParent, QSettings* pSett)
	: QDialog(pParent), m_pSettings(pSett)
{
	this->setupUi(this);

	QObject::connect(editKi, SIGNAL(textChanged(const QString&)), this, SLOT(KiChanged()));
	QObject::connect(editKf, SIGNAL(textChanged(const QString&)), this, SLOT(KfChanged()));
}

RecipParamDlg::~RecipParamDlg()
{}

void RecipParamDlg::paramsChanged(const RecipParams& parms)
{
	double dQ = parms.dQ;
	if(parms.d2Theta < 0.)
		dQ = -dQ;

	double dq_rlu = std::sqrt(parms.q_rlu[0]*parms.q_rlu[0]
				+ parms.q_rlu[1]*parms.q_rlu[1]
				+ parms.q_rlu[2]*parms.q_rlu[2]);

	this->editKi->setText(tl::var_to_str<double>(parms.dki, g_iPrec).c_str());
	this->editKf->setText(tl::var_to_str<double>(parms.dkf, g_iPrec).c_str());
	this->editQ->setText(tl::var_to_str<double>(dQ, g_iPrec).c_str());
	this->editq->setText(tl::var_to_str<double>(parms.dq, g_iPrec).c_str());
	this->editqrlu->setText(tl::var_to_str<double>(dq_rlu, g_iPrec).c_str());
	this->editE->setText(tl::var_to_str<double>(parms.dE, g_iPrec).c_str());
	this->edit2Theta->setText(tl::var_to_str<double>(parms.d2Theta / M_PI * 180., g_iPrec).c_str());
	this->editTheta->setText(tl::var_to_str<double>(parms.dTheta / M_PI * 180., g_iPrec).c_str());
	this->editKiQ->setText(tl::var_to_str<double>(parms.dKiQ / M_PI * 180., g_iPrec).c_str());
	this->editKfQ->setText(tl::var_to_str<double>(parms.dKfQ / M_PI * 180., g_iPrec).c_str());

	this->editQx->setText(tl::var_to_str<double>(-parms.Q[0], g_iPrec).c_str());
	this->editQy->setText(tl::var_to_str<double>(-parms.Q[1], g_iPrec).c_str());
	this->editQz->setText(tl::var_to_str<double>(-parms.Q[2], g_iPrec).c_str());
	this->editQxrlu->setText(tl::var_to_str<double>(-parms.Q_rlu[0], g_iPrec).c_str());
	this->editQyrlu->setText(tl::var_to_str<double>(-parms.Q_rlu[1], g_iPrec).c_str());
	this->editQzrlu->setText(tl::var_to_str<double>(-parms.Q_rlu[2], g_iPrec).c_str());

	this->editqx->setText(tl::var_to_str<double>(-parms.q[0], g_iPrec).c_str());
	this->editqy->setText(tl::var_to_str<double>(-parms.q[1], g_iPrec).c_str());
	this->editqz->setText(tl::var_to_str<double>(-parms.q[2], g_iPrec).c_str());
	this->editqxrlu->setText(tl::var_to_str<double>(-parms.q_rlu[0], g_iPrec).c_str());
	this->editqyrlu->setText(tl::var_to_str<double>(-parms.q_rlu[1], g_iPrec).c_str());
	this->editqzrlu->setText(tl::var_to_str<double>(-parms.q_rlu[2], g_iPrec).c_str());

	this->editGx->setText(tl::var_to_str<double>(-parms.G[0], g_iPrec).c_str());
	this->editGy->setText(tl::var_to_str<double>(-parms.G[1], g_iPrec).c_str());
	this->editGz->setText(tl::var_to_str<double>(-parms.G[2], g_iPrec).c_str());
	this->editGxrlu->setText(tl::var_to_str<double>(-parms.G_rlu[0], g_iPrec).c_str());
	this->editGyrlu->setText(tl::var_to_str<double>(-parms.G_rlu[1], g_iPrec).c_str());
	this->editGzrlu->setText(tl::var_to_str<double>(-parms.G_rlu[2], g_iPrec).c_str());


	// focus
	ublas::vector<double> vecG = tl::make_vec({-parms.G_rlu[0], -parms.G_rlu[1], -parms.G_rlu[2]});
	ublas::vector<double> vecUp = tl::make_vec({parms.orient_up[0], parms.orient_up[1], parms.orient_up[2]});
	ublas::vector<double> vecFm = tl::cross_3(vecG, vecUp);
	vecFm /= ublas::norm_2(vecFm);
	
	tl::set_eps_0(vecFm);
	
	this->editFmx->setText(tl::var_to_str<double>(vecFm[0], g_iPrec).c_str());
	this->editFmy->setText(tl::var_to_str<double>(vecFm[1], g_iPrec).c_str());
	this->editFmz->setText(tl::var_to_str<double>(vecFm[2], g_iPrec).c_str());

	this->editFpx->setText(tl::var_to_str<double>(-vecFm[0], g_iPrec).c_str());
	this->editFpy->setText(tl::var_to_str<double>(-vecFm[1], g_iPrec).c_str());
	this->editFpz->setText(tl::var_to_str<double>(-vecFm[2], g_iPrec).c_str());
}


void RecipParamDlg::KiChanged()
{
	tl::wavenumber ki = tl::str_to_var<double>(editKi->text().toStdString()) / tl::angstrom;
	tl::energy Ei = tl::k2E(ki);
	tl::length lami = tl::k2lam(ki);
	tl::velocity vi = tl::k2v(ki);

	editEi->setText(tl::var_to_str<double>(Ei / tl::one_meV, g_iPrec).c_str());
	editLami->setText(tl::var_to_str<double>(lami / tl::angstrom, g_iPrec).c_str());
	editVi->setText(tl::var_to_str<double>(vi*tl::seconds/tl::meters, g_iPrec).c_str());
}

void RecipParamDlg::KfChanged()
{
	tl::wavenumber kf = tl::str_to_var<double>(editKf->text().toStdString()) / tl::angstrom;
	tl::energy Ef = tl::k2E(kf);
	tl::length lamf = tl::k2lam(kf);
	tl::velocity vf = tl::k2v(kf);

	editEf->setText(tl::var_to_str<double>(Ef / tl::one_meV, g_iPrec).c_str());
	editLamf->setText(tl::var_to_str<double>(lamf / tl::angstrom, g_iPrec).c_str());
	editVf->setText(tl::var_to_str<double>(vf*tl::seconds/tl::meters, g_iPrec).c_str());
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
