/*
 * Settings
 * @author tweber
 * @date 5-dec-2014
 * @license GPLv2
 */

#include "SettingsDlg.h"
#include "tlibs/string/string.h"
#include "tlibs/log/log.h"
#ifndef NO_3D
	#include "tlibs/gfx/gl.h"
#endif
#include "helper/globals.h"

#include <QFileDialog>
#include <QFontDialog>
#include <iostream>
#include <limits>


// -----------------------------------------------------------------------------


SettingsDlg::SettingsDlg(QWidget* pParent, QSettings* pSett)
	: QDialog(pParent), m_pSettings(pSett)
{
	setupUi(this);
	connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(ButtonBoxClicked(QAbstractButton*)));
	connect(btnGLFont, SIGNAL(clicked()), this, SLOT(SelectGLFont()));
	connect(btnGfxFont, SIGNAL(clicked()), this, SLOT(SelectGfxFont()));

	m_vecEdits =
	{
		t_tupEdit("net/sample_name", "nicos/sample/samplename", editSampleName),
		t_tupEdit("net/lattice", "nicos/sample/lattice", editSampleLattice),
		t_tupEdit("net/angles", "nicos/sample/angles", editSampleAngles),
		t_tupEdit("net/orient1", "nicos/sample/orient1", editSampleOrient1),
		t_tupEdit("net/orient2", "nicos/sample/orient2", editSampleOrient2),
		t_tupEdit("net/spacegroup", "nicos/sample/spacegroup", editSampleSG),
		t_tupEdit("net/psi0", "nicos/sample/psi0", editSamplePsi0),

		t_tupEdit("net/stheta", "nicos/sth/value", editSampleTheta),
		t_tupEdit("net/s2theta", "nicos/phi/value", editSample2Theta),

		t_tupEdit("net/mtheta", "nicos/m2th/value", editMonoTheta),
		t_tupEdit("net/m2theta", "nicos/m2tt/value", editMono2Theta),
		t_tupEdit("net/mono_d", "nicos/mono/dvalue", editMonoD),

		t_tupEdit("net/atheta", "nicos/ath/value", editAnaTheta),
		t_tupEdit("net/a2theta", "nicos/att/value", editAna2Theta),
		t_tupEdit("net/ana_d", "nicos/ana/dvalue", editAnaD),

		//t_tupEdit("net/stheta_aux", "nicos/sth/value", editRotTheta),
		//t_tupEdit("net/stheta_aux_alias", "nicos/sth/alias", editRotAlias),

		t_tupEdit("gl/font", "", editGLFont),
		t_tupEdit("main/font_gfx", "", editGfxFont)
	};

	m_vecChecks =
	{
		t_tupCheck("main/dlg_previews", 1, checkPreview),

		t_tupCheck("net/flip_orient2", 1, checkFlipOrient2),
	};

	m_vecSpins =
	{
		t_tupSpin("main/prec", g_iPrec, spinPrecGen),
		t_tupSpin("main/prec_gfx", g_iPrecGfx, spinPrecGfx),

		t_tupSpin("net/poll", 750, spinNetPoll),

		t_tupSpin("main/max_peaks", 10, spinBragg),
	};

	spinPrecGen->setMaximum(std::numeric_limits<double>::max_digits10);
	spinPrecGfx->setMaximum(std::numeric_limits<double>::max_digits10);

	SetDefaults(0);


	if(m_pSettings && m_pSettings->contains("settings/geo"))
		restoreGeometry(m_pSettings->value("settings/geo").toByteArray());

	LoadSettings();
}

SettingsDlg::~SettingsDlg()
{}


void SettingsDlg::SetDefaults(bool bOverwrite)
{
	if(!m_pSettings) return;

	for(const t_tupEdit& tup : m_vecEdits)
	{
		const std::string& strKey = std::get<0>(tup);
		const std::string& strDef = std::get<1>(tup);

		bool bKeyExists = m_pSettings->contains(strKey.c_str());
		if(bKeyExists && !bOverwrite) continue;

		m_pSettings->setValue(strKey.c_str(), strDef.c_str());
	}

	for(const t_tupCheck& tup : m_vecChecks)
	{
		const std::string& strKey = std::get<0>(tup);
		const bool bDef = std::get<1>(tup);

		bool bKeyExists = m_pSettings->contains(strKey.c_str());
		if(bKeyExists && !bOverwrite) continue;

		m_pSettings->setValue(strKey.c_str(), bDef);
	}

	for(const t_tupSpin& tup : m_vecSpins)
	{
		const std::string& strKey = std::get<0>(tup);
		const int iDef = std::get<1>(tup);

		bool bKeyExists = m_pSettings->contains(strKey.c_str());
		if(bKeyExists && !bOverwrite) continue;

		m_pSettings->setValue(strKey.c_str(), iDef);
	}
}


void SettingsDlg::LoadSettings()
{
	if(!m_pSettings) return;

	for(const t_tupEdit& tup : m_vecEdits)
	{
		const std::string& strKey = std::get<0>(tup);
		const std::string& strDef = std::get<1>(tup);
		QLineEdit* pEdit = std::get<2>(tup);

		QString strVal = m_pSettings->value(strKey.c_str(), strDef.c_str()).toString();
		pEdit->setText(strVal);
	}

	for(const t_tupCheck& tup : m_vecChecks)
	{
		const std::string& strKey = std::get<0>(tup);
		bool bDef = std::get<1>(tup);
		QCheckBox* pCheck = std::get<2>(tup);

		bool bVal = m_pSettings->value(strKey.c_str(), bDef).toBool();
		pCheck->setChecked(bVal);
	}

	for(const t_tupSpin& tup : m_vecSpins)
	{
		const std::string& strKey = std::get<0>(tup);
		int iDef = std::get<1>(tup);
		QSpinBox* pSpin = std::get<2>(tup);

		int iVal = m_pSettings->value(strKey.c_str(), iDef).toInt();
		pSpin->setValue(iVal);
	}

	SetGlobals();
}

void SettingsDlg::SaveSettings()
{
	if(!m_pSettings) return;

	for(const t_tupEdit& tup : m_vecEdits)
	{
		const std::string& strKey = std::get<0>(tup);
		QLineEdit* pEdit = std::get<2>(tup);

		m_pSettings->setValue(strKey.c_str(), pEdit->text());
	}

	for(const t_tupCheck& tup : m_vecChecks)
	{
		const std::string& strKey = std::get<0>(tup);
		QCheckBox* pCheck = std::get<2>(tup);

		m_pSettings->setValue(strKey.c_str(), pCheck->isChecked());
	}

	for(const t_tupSpin& tup : m_vecSpins)
	{
		const std::string& strKey = std::get<0>(tup);
		QSpinBox* pSpin = std::get<2>(tup);

		m_pSettings->setValue(strKey.c_str(), pSpin->value());
	}

	SetGlobals();
}


void SettingsDlg::SetGlobals() const
{
	g_iPrec = spinPrecGen->value();
	g_iPrecGfx = spinPrecGfx->value();

	g_dEps = std::pow(10., -double(g_iPrec));
	g_dEpsGfx = std::pow(10., -double(g_iPrecGfx));

	QString strGfxFont = editGfxFont->text();
	if(strGfxFont.length() != 0)
	{
		QFont font;
		if(font.fromString(strGfxFont))
			g_fontGfx = font;
	}

	QString strGLFont = editGLFont->text();
	if(strGLFont.length() != 0)
	{
		QFont font;
		if(font.fromString(strGLFont))
			g_fontGL = font;
	}
}


void SettingsDlg::SelectGLFont()
{
	bool bOk;
	QFont fontNew = QFontDialog::getFont(&bOk, g_fontGL, this);
	if(bOk)
	{
		g_fontGL = fontNew;
		editGLFont->setText(fontNew.toString());
	}
}

void SettingsDlg::SelectGfxFont()
{
	bool bOk;
	QFont fontNew = QFontDialog::getFont(&bOk, g_fontGfx, this);
	if(bOk)
	{
		g_fontGfx = fontNew;
		editGfxFont->setText(fontNew.toString());
	}
}


void SettingsDlg::showEvent(QShowEvent *pEvt)
{
	QDialog::showEvent(pEvt);
}

void SettingsDlg::ButtonBoxClicked(QAbstractButton *pBtn)
{
	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::ApplyRole ||
	   buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		SaveSettings();
	}
	else if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::ResetRole)
	{
		SetDefaults(1);
		LoadSettings();
	}
	else if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::RejectRole)
	{
		QDialog::reject();
	}

	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		if(m_pSettings)
			m_pSettings->setValue("settings/geo", saveGeometry());

		QDialog::accept();
	}
}

#include "SettingsDlg.moc"
