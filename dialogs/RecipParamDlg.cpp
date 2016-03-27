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
#include "libs/globals.h"

namespace ublas = boost::numeric::ublas;


RecipParamDlg::RecipParamDlg(QWidget* pParent, QSettings* pSett)
	: QDialog(pParent), m_pSettings(pSett)
{
	this->setupUi(this);
	if(m_pSettings)
	{
		QFont font;
		if(m_pSettings->contains("main/font_gen") && font.fromString(m_pSettings->value("main/font_gen", "").toString()))
			setFont(font);
	}

	QObject::connect(editKi, SIGNAL(textChanged(const QString&)), this, SLOT(KiChanged()));
	QObject::connect(editKf, SIGNAL(textChanged(const QString&)), this, SLOT(KfChanged()));

	QObject::connect(btnUseG, SIGNAL(clicked()), this, SLOT(SetGOrigin()));
	QObject::connect(editOriginH, SIGNAL(textEdited(const QString&)), this, SLOT(OriginChanged()));
	QObject::connect(editOriginK, SIGNAL(textEdited(const QString&)), this, SLOT(OriginChanged()));
	QObject::connect(editOriginL, SIGNAL(textEdited(const QString&)), this, SLOT(OriginChanged()));
	QObject::connect(radioEm, SIGNAL(toggled(bool)), this, SLOT(OriginChanged()));


	if(m_pSettings)
	{
		if(m_pSettings->contains("recip_params/geo"))
			restoreGeometry(m_pSettings->value("recip_params/geo").toByteArray());
		if(m_pSettings->contains("recip_params/defs_Em"))
			radioEm->setChecked(m_pSettings->value("recip_params/defs_Em").toBool());
	}
}

RecipParamDlg::~RecipParamDlg()
{}

void RecipParamDlg::paramsChanged(const RecipParams& parms)
{
	m_params = parms;

	double dQ = m_params.dQ;
	if(m_params.d2Theta < 0.)
		dQ = -dQ;

	double dq_rlu = std::sqrt(m_params.q_rlu[0]*m_params.q_rlu[0]
				+ m_params.q_rlu[1]*m_params.q_rlu[1]
				+ m_params.q_rlu[2]*m_params.q_rlu[2]);

	this->editKi->setText(tl::var_to_str<double>(m_params.dki, g_iPrec).c_str());
	this->editKf->setText(tl::var_to_str<double>(m_params.dkf, g_iPrec).c_str());
	this->editQ->setText(tl::var_to_str<double>(dQ, g_iPrec).c_str());
	this->editq->setText(tl::var_to_str<double>(m_params.dq, g_iPrec).c_str());
	this->editqrlu->setText(tl::var_to_str<double>(dq_rlu, g_iPrec).c_str());
	this->editE->setText(tl::var_to_str<double>(m_params.dE, g_iPrec).c_str());
	this->edit2Theta->setText(tl::var_to_str<double>(m_params.d2Theta / M_PI * 180., g_iPrec).c_str());
	this->editTheta->setText(tl::var_to_str<double>(m_params.dTheta / M_PI * 180., g_iPrec).c_str());
	this->editKiQ->setText(tl::var_to_str<double>(m_params.dKiQ / M_PI * 180., g_iPrec).c_str());
	this->editKfQ->setText(tl::var_to_str<double>(m_params.dKfQ / M_PI * 180., g_iPrec).c_str());

	this->editQx->setText(tl::var_to_str<double>(-m_params.Q[0], g_iPrec).c_str());
	this->editQy->setText(tl::var_to_str<double>(-m_params.Q[1], g_iPrec).c_str());
	this->editQz->setText(tl::var_to_str<double>(-m_params.Q[2], g_iPrec).c_str());
	this->editQxrlu->setText(tl::var_to_str<double>(-m_params.Q_rlu[0], g_iPrec).c_str());
	this->editQyrlu->setText(tl::var_to_str<double>(-m_params.Q_rlu[1], g_iPrec).c_str());
	this->editQzrlu->setText(tl::var_to_str<double>(-m_params.Q_rlu[2], g_iPrec).c_str());

	this->editqx->setText(tl::var_to_str<double>(-m_params.q[0], g_iPrec).c_str());
	this->editqy->setText(tl::var_to_str<double>(-m_params.q[1], g_iPrec).c_str());
	this->editqz->setText(tl::var_to_str<double>(-m_params.q[2], g_iPrec).c_str());
	this->editqxrlu->setText(tl::var_to_str<double>(-m_params.q_rlu[0], g_iPrec).c_str());
	this->editqyrlu->setText(tl::var_to_str<double>(-m_params.q_rlu[1], g_iPrec).c_str());
	this->editqzrlu->setText(tl::var_to_str<double>(-m_params.q_rlu[2], g_iPrec).c_str());

	this->editGx->setText(tl::var_to_str<double>(-m_params.G[0], g_iPrec).c_str());
	this->editGy->setText(tl::var_to_str<double>(-m_params.G[1], g_iPrec).c_str());
	this->editGz->setText(tl::var_to_str<double>(-m_params.G[2], g_iPrec).c_str());
	this->editGxrlu->setText(tl::var_to_str<double>(-m_params.G_rlu[0], g_iPrec).c_str());
	this->editGyrlu->setText(tl::var_to_str<double>(-m_params.G_rlu[1], g_iPrec).c_str());
	this->editGzrlu->setText(tl::var_to_str<double>(-m_params.G_rlu[2], g_iPrec).c_str());


	// focus
	ublas::vector<double> vecG = tl::make_vec({-m_params.G_rlu[0], -m_params.G_rlu[1], -m_params.G_rlu[2]});
	ublas::vector<double> vecUp = tl::make_vec({m_params.orient_up[0], m_params.orient_up[1], m_params.orient_up[2]});
	ublas::vector<double> vecFm = tl::cross_3(vecG, vecUp);
	vecFm /= ublas::norm_2(vecFm);
	tl::set_eps_0(vecFm, g_dEps);

	this->editFmx->setText(tl::var_to_str<double>(vecFm[0], g_iPrec).c_str());
	this->editFmy->setText(tl::var_to_str<double>(vecFm[1], g_iPrec).c_str());
	this->editFmz->setText(tl::var_to_str<double>(vecFm[2], g_iPrec).c_str());

	this->editFpx->setText(tl::var_to_str<double>(-vecFm[0], g_iPrec).c_str());
	this->editFpy->setText(tl::var_to_str<double>(-vecFm[1], g_iPrec).c_str());
	this->editFpz->setText(tl::var_to_str<double>(-vecFm[2], g_iPrec).c_str());


	/*if(editOriginH->text().trimmed()=="" || editOriginK->text().trimmed()=="" || editOriginL->text().trimmed()=="")
		SetGOrigin();*/
	OriginChanged();
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


void RecipParamDlg::SetGOrigin()
{
	editOriginH->setText(editGxrlu->text());
	editOriginK->setText(editGyrlu->text());
	editOriginL->setText(editGzrlu->text());

	OriginChanged();
}

void RecipParamDlg::OriginChanged()
{
	const bool bEm = radioEm->isChecked();
	const double dH = tl::str_to_var<double>(editOriginH->text().toStdString());
	const double dK = tl::str_to_var<double>(editOriginK->text().toStdString());
	const double dL = tl::str_to_var<double>(editOriginL->text().toStdString());
	const double dSq = dH*dH + dK*dK + dL*dL;

	const ublas::vector<double> vecQ = tl::make_vec({dH, dK, dL});
	ublas::vector<double> vecUp = tl::make_vec({m_params.orient_up[0], m_params.orient_up[1], m_params.orient_up[2]});
	ublas::vector<double> vecFm = tl::cross_3(vecQ, vecUp);
	vecUp /= ublas::norm_2(vecUp);
	vecFm /= ublas::norm_2(vecFm);

	if(!bEm) vecFm = -vecFm;
	tl::set_eps_0(vecUp, g_dEps);
	tl::set_eps_0(vecFm, g_dEps);


	std::ostringstream ostrDefs;
	ostrDefs << "import numpy as np\n\n\n";
	ostrDefs << "origin = np.array([" << dH << ", " << dK << ", " << dL << "])\n\n";

	ostrDefs << "longdir = origin";
	if(!tl::float_equal(dSq, 1.))
		ostrDefs << " / np.sqrt(" << dSq << ")";
	ostrDefs << "\n";

	ostrDefs << "transdir = np.array([" << vecFm[0] << ", " << vecFm[1] << ", " << vecFm[2] << "])\n";
	ostrDefs << "updir = np.array([" << vecUp[0] << ", " << vecUp[1] << ", " << vecUp[2] << "])\n\n\n";

	ostrDefs << "# define some arbitrary direction in the scattering plane here:\n";
	ostrDefs << "# userangle = 45. / 180. * np.pi\n";
	ostrDefs << "# userdir = longdir*np.cos(userangle) + transdir*np.sin(userangle)\n";

	editDefs->setText(ostrDefs.str().c_str());
}


void RecipParamDlg::closeEvent(QCloseEvent *pEvt)
{
	QDialog::closeEvent(pEvt);
}

void RecipParamDlg::accept()
{
	if(m_pSettings)
	{
		m_pSettings->setValue("recip_params/defs_Em", radioEm->isChecked());
		m_pSettings->setValue("recip_params/geo", saveGeometry());
	}

	QDialog::accept();
}

void RecipParamDlg::showEvent(QShowEvent *pEvt)
{
	QDialog::showEvent(pEvt);
}


#include "RecipParamDlg.moc"
