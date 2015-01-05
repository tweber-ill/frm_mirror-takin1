/*
 * Settings
 * @author tweber
 * @date 5-dec-2014
 */

#include "SettingsDlg.h"
#include "../helper/string.h"
#include "../helper/log.h"
#include "../helper/gl.h"

#include <QtGui/QFileDialog>
#include <iostream>


// -----------------------------------------------------------------------------


SettingsDlg::SettingsDlg(QWidget* pParent, QSettings* pSett)
	: QDialog(pParent), m_pSettings(pSett)
{
	setupUi(this);
	connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(ButtonBoxClicked(QAbstractButton*)));
	connect(btnGLFont, SIGNAL(clicked()), this, SLOT(SelectFont()));


	std::string strDefFont = FontMap::get_font_file("dejavusansmono.ttf");
	if(strDefFont == "")
	{
		log_warn("Default gl font not available.");
		strDefFont = "/usr/share/fonts/dejavu/DejaVuSansMono.ttf";
	}

	m_vecEdits =
	{
		t_tupEdit("net/sample_name", "nicos/sample/samplename", editSampleName),
		t_tupEdit("net/lattice", "nicos/sample/lattice", editSampleLattice),
		t_tupEdit("net/angles", "nicos/sample/angles", editSampleAngles),
		t_tupEdit("net/orient1", "nicos/sample/orient1", editSampleOrient1),
		t_tupEdit("net/orient2", "nicos/sample/orient2", editSampleOrient2),
		t_tupEdit("net/spacegroup", "nicos/sample/spacegroup", editSampleSG),
		t_tupEdit("net/psi0", "nicos/sample/psi0", editSamplePsi0),

		t_tupEdit("net/stheta", "nicos/om/value", editSampleTheta),
		t_tupEdit("net/s2theta", "nicos/phi/value", editSample2Theta),

		t_tupEdit("net/mtheta", "nicos/m2th/value", editMonoTheta),
		t_tupEdit("net/m2theta", "nicos/m2tt/value", editMono2Theta),
		t_tupEdit("net/mono_d", "nicos/mono/dvalue", editMonoD),

		t_tupEdit("net/atheta", "nicos/ath/value", editAnaTheta),
		t_tupEdit("net/a2theta", "nicos/att/value", editAna2Theta),
		t_tupEdit("net/ana_d", "nicos/ana/dvalue", editAnaD),

		t_tupEdit("net/stheta_aux", "nicos/sth/value", editRotTheta),
		t_tupEdit("net/stheta_aux_alias", "nicos/sth/alias", editRotAlias),


		t_tupEdit("gl/font", strDefFont, editGLFont)
	};

	SetDefaults(0);
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
}

void SettingsDlg::SelectFont()
{
	if(!m_pSettings) return;

	QString strDirLast = m_pSettings->value("gl/last_font_dir", ".").toString();
	QString strFile = QFileDialog::getOpenFileName(this,
							"Select Font...",
							strDirLast,
							"TTF files (*.ttf *.TTF)");
	if(strFile != "")
	{
		//m_pSettings->setValue("gl/font", strFile);
		editGLFont->setText(strFile);

		std::string strDir = get_dir(strFile.toStdString());
		m_pSettings->setValue("gl/last_font_dir", QString(strDir.c_str()));
	}
}

void SettingsDlg::showEvent(QShowEvent *pEvt)
{
	if(m_pSettings && m_pSettings->contains("settings/geo"))
		restoreGeometry(m_pSettings->value("settings/geo").toByteArray());

	LoadSettings();
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
