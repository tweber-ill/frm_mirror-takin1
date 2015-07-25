/*
 * Goto Dialog
 * @author Tobias Weber
 * @date 15-oct-2014
 * @copyright GPLv2
 */

#include "GotoDlg.h"
#include "tlibs/math/neutrons.hpp"
#include "tlibs/string/string.h"
#include "tlibs/string/spec_char.h"
#include "helper/globals.h"

#include <QFileDialog>
#include <QMessageBox>

namespace units = boost::units;
namespace co = boost::units::si::constants::codata;


GotoDlg::GotoDlg(QWidget* pParent, QSettings* pSett) : QDialog(pParent), m_pSettings(pSett)
{
	this->setupUi(this);
	connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(ButtonBoxClicked(QAbstractButton*)));

	QObject::connect(btnAdd, SIGNAL(clicked()), this, SLOT(AddPosToList()));
	QObject::connect(btnDel, SIGNAL(clicked()), this, SLOT(RemPosFromList()));
	QObject::connect(btnLoad, SIGNAL(clicked()), this, SLOT(LoadList()));
	QObject::connect(btnSave, SIGNAL(clicked()), this, SLOT(SaveList()));
	QObject::connect(listSeq, SIGNAL(itemSelectionChanged()), this, SLOT(ListItemSelected()));
	QObject::connect(listSeq, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
					this, SLOT(ListItemDoubleClicked(QListWidgetItem*)));

	QObject::connect(editKi, SIGNAL(textEdited(const QString&)), this, SLOT(EditedKiKf()));
	QObject::connect(editKf, SIGNAL(textEdited(const QString&)), this, SLOT(EditedKiKf()));
	QObject::connect(editE, SIGNAL(textEdited(const QString&)), this, SLOT(EditedE()));
	QObject::connect(btnGetPos, SIGNAL(clicked()), this, SLOT(GetCurPos()));

	std::vector<QObject*> vecObjs {editH, editK, editL};
	for(QObject* pObj : vecObjs)
		QObject::connect(pObj, SIGNAL(textEdited(const QString&)), this, SLOT(CalcSample()));

	std::vector<QObject*> vecAngles {edit2ThetaM, editThetaM, edit2ThetaA, editThetaA, edit2ThetaS, editThetaS};
	for(QObject* pObj : vecAngles)
		QObject::connect(pObj, SIGNAL(textEdited(const QString&)), this, SLOT(EditedAngles()));
}

GotoDlg::~GotoDlg()
{
	ClearList();
}


void GotoDlg::CalcSample()
{
	m_bSampleOk = 0;

	std::vector<QLineEdit*> vecEdits{editThetaS, edit2ThetaS};
	for(QLineEdit* pEdit : vecEdits)
		pEdit->setText("");

	bool bHOk=0, bKOk=0, bLOk=0, bKiOk=0, bKfOk=0;

	double dH = editH->text().toDouble(&bHOk);
	double dK = editK->text().toDouble(&bKOk);
	double dL = editL->text().toDouble(&bLOk);
	double dKi = editKi->text().toDouble(&bKiOk);
	double dKf = editKf->text().toDouble(&bKfOk);

	if(!bHOk || !bKOk || !bLOk || !bKiOk || !bKfOk)
		return;

	ublas::vector<double> vecQ;
	bool bFailed = 0;
	try
	{
		tl::get_tas_angles(m_lattice,
					m_vec1, m_vec2,
					dKi, dKf,
					dH, dK, dL,
					m_bSenseS,
					&m_dSampleTheta, &m_dSample2Theta,
					&vecQ);

		if(tl::is_nan_or_inf<double>(m_dSample2Theta))
			throw tl::Err("Invalid sample 2theta.");
		if(tl::is_nan_or_inf<double>(m_dSampleTheta))
			throw tl::Err("Invalid sample theta.");
	}
	catch(const std::exception& ex)
	{
		editThetaS->setText("invalid");
		edit2ThetaS->setText("invalid");

		//log_err(ex.what());
		labelStatus->setText((std::string("Error: ") + ex.what()).c_str());
		bFailed = 1;
	}

	tl::set_eps_0(vecQ);
	tl::set_eps_0(m_dSample2Theta);
	tl::set_eps_0(m_dSampleTheta);

	const std::wstring strAA = tl::get_spec_char_utf16("AA") + tl::get_spec_char_utf16("sup-") + tl::get_spec_char_utf16("sup1");

	std::wostringstream ostrStatus;
	ostrStatus << "Q = " << vecQ << " " << strAA << ", |Q| = " << ublas::norm_2(vecQ) << " " << strAA;
	labelQ->setText(QString::fromWCharArray(ostrStatus.str().c_str()));

#ifndef NDEBUG
	try
	{
		ublas::vector<double> vecQ0 = vecQ;
		vecQ0.resize(2, true);
		double dAngleQVec0 = tl::vec_angle(vecQ0) / M_PI * 180.;
		tl::log_info("Angle Q Orient0: ", dAngleQVec0, " deg.");
	}
	catch(const std::exception& ex)
	{}
#endif

	if(bFailed) return;

	editThetaS->setText(tl::var_to_str(m_dSampleTheta/M_PI*180., g_iPrec).c_str());
	edit2ThetaS->setText(tl::var_to_str(m_dSample2Theta/M_PI*180., g_iPrec).c_str());

	m_bSampleOk = 1;

	if(m_bMonoAnaOk && m_bSampleOk)
		labelStatus->setText("Position OK.");
}

void GotoDlg::CalcMonoAna()
{
	m_bMonoAnaOk = 0;

	std::vector<QLineEdit*> vecEdits{editThetaM, edit2ThetaM, editThetaA, edit2ThetaA};
	for(QLineEdit* pEdit : vecEdits)
		pEdit->setText("");

	bool bKiOk=0, bKfOk=0;
	double dKi = editKi->text().toDouble(&bKiOk);
	double dKf = editKf->text().toDouble(&bKfOk);
	if(!bKiOk || !bKfOk) return;

	tl::wavenumber ki = dKi / tl::angstrom;
	tl::wavenumber kf = dKf / tl::angstrom;

	bool bMonoOk = 0;
	bool bAnaOk = 0;

	try
	{
		m_dMono2Theta = tl::get_mono_twotheta(ki, m_dMono*tl::angstrom, m_bSenseM) / tl::radians;
		if(tl::is_nan_or_inf<double>(m_dMono2Theta))
			throw tl::Err("Invalid monochromator angle.");

		tl::set_eps_0(m_dMono2Theta);
		bMonoOk = 1;
	}
	catch(const std::exception& ex)
	{
		editThetaM->setText("invalid");
		edit2ThetaM->setText("invalid");

		//log_err(ex.what());
		labelStatus->setText((std::string("Error: ") + ex.what()).c_str());
	}

	try
	{
		m_dAna2Theta = tl::get_mono_twotheta(kf, m_dAna*tl::angstrom, m_bSenseA) / tl::radians;
		if(tl::is_nan_or_inf<double>(m_dAna2Theta))
			throw tl::Err("Invalid analysator angle.");

		tl::set_eps_0(m_dAna2Theta);
		bAnaOk = 1;
	}
	catch(const std::exception& ex)
	{
		editThetaA->setText("invalid");
		edit2ThetaA->setText("invalid");

		//log_err(ex.what());
		labelStatus->setText((std::string("Error: ") + ex.what()).c_str());
	}

	if(!bMonoOk || !bAnaOk)
		return;


	double dTMono = m_dMono2Theta / 2.;
	double dTAna = m_dAna2Theta / 2.;

	edit2ThetaM->setText(tl::var_to_str(m_dMono2Theta/M_PI*180., g_iPrec).c_str());
	editThetaM->setText(tl::var_to_str(dTMono/M_PI*180., g_iPrec).c_str());
	edit2ThetaA->setText(tl::var_to_str(m_dAna2Theta/M_PI*180., g_iPrec).c_str());
	editThetaA->setText(tl::var_to_str(dTAna/M_PI*180., g_iPrec).c_str());

	m_bMonoAnaOk = 1;

	if(m_bMonoAnaOk && m_bSampleOk)
		labelStatus->setText("Position OK.");
}

void GotoDlg::EditedKiKf()
{
	bool bKiOk=0, bKfOk=0;
	double dKi = editKi->text().toDouble(&bKiOk);
	double dKf = editKf->text().toDouble(&bKfOk);
	if(!bKiOk || !bKfOk) return;

	tl::energy Ei = tl::k2E(dKi / tl::angstrom);
	tl::energy Ef = tl::k2E(dKf / tl::angstrom);

	tl::energy E = Ei-Ef;
	double dE = E/tl::one_meV;
	tl::set_eps_0(dE);

	std::string strE = tl::var_to_str<double>(dE, g_iPrec);
	editE->setText(strE.c_str());

	CalcMonoAna();
	CalcSample();
}

void GotoDlg::EditedE()
{
	bool bOk = 0;
	double dE = editE->text().toDouble(&bOk);
	if(!bOk) return;
	tl::energy E = dE * tl::one_meV;

	bool bImag=0;
	tl::wavenumber k_E = tl::E2k(E, bImag);
	double dSign = 1.;
	if(bImag) dSign = -1.;

	if(radioFixedKi->isChecked())
	{
		bool bKOk = 0;
		double dKi = editKi->text().toDouble(&bKOk);
		if(!bKOk) return;

		tl::wavenumber ki = dKi / tl::angstrom;
		tl::wavenumber kf = units::sqrt(ki*ki - dSign*k_E*k_E);

		double dKf = kf*tl::angstrom;
		tl::set_eps_0(dKf);

		std::string strKf = tl::var_to_str<double>(dKf, g_iPrec);
		editKf->setText(strKf.c_str());
	}
	else
	{
		bool bKOk = 0;
		double dKf = editKf->text().toDouble(&bKOk);
		if(!bKOk) return;

		tl::wavenumber kf = dKf / tl::angstrom;
		tl::wavenumber ki = units::sqrt(kf*kf + dSign*k_E*k_E);

		double dKi = ki*tl::angstrom;
		tl::set_eps_0(dKi);

		std::string strKi = tl::var_to_str<double>(dKi, g_iPrec);
		editKi->setText(strKi.c_str());
	}

	CalcMonoAna();
	CalcSample();
}

// calc. tas angles -> hkl
void GotoDlg::EditedAngles()
{
	bool bthmOk;
	double th_m = edit2ThetaM->text().toDouble(&bthmOk)/2. / 180.*M_PI;
	tl::set_eps_0(th_m);
	if(bthmOk)
		editThetaM->setText(tl::var_to_str<double>(th_m/M_PI*180., g_iPrec).c_str());

	bool bthaOk;
	double th_a = edit2ThetaA->text().toDouble(&bthaOk)/2. / 180.*M_PI;
	tl::set_eps_0(th_a);
	if(bthaOk)
		editThetaA->setText(tl::var_to_str<double>(th_a/M_PI*180., g_iPrec).c_str());

	bool bthsOk, bttsOk;
	double th_s = editThetaS->text().toDouble(&bthsOk) / 180.*M_PI;
	double tt_s = edit2ThetaS->text().toDouble(&bttsOk) / 180.*M_PI;

	if(!bthmOk || !bthaOk || !bthsOk || !bttsOk)
		return;


	double h,k,l;
	double dKi, dKf, dE;
	ublas::vector<double> vecQ;
	bool bFailed = 0;
	try
	{
		tl::get_hkl_from_tas_angles<double>(m_lattice,
								m_vec1, m_vec2,
								m_dMono, m_dAna,
								th_m, th_a, th_s, tt_s,
								m_bSenseM, m_bSenseA, m_bSenseS,
								&h, &k, &l,
								&dKi, &dKf, &dE, 0,
								&vecQ);

		if(tl::is_nan_or_inf<double>(h) || tl::is_nan_or_inf<double>(k) || tl::is_nan_or_inf<double>(l))
			throw tl::Err("Invalid hkl.");
	}
	catch(const std::exception& ex)
	{
		//log_err(ex.what());
		labelStatus->setText((std::string("Error: ") + ex.what()).c_str());
		bFailed = 1;
	}

	const std::wstring strAA = tl::get_spec_char_utf16("AA") + tl::get_spec_char_utf16("sup-") + tl::get_spec_char_utf16("sup1");
	std::wostringstream ostrStatus;
	ostrStatus << "Q = " << vecQ << " " << strAA << ", |Q| = " << ublas::norm_2(vecQ) << " " << strAA;
	labelQ->setText(QString::fromWCharArray(ostrStatus.str().c_str()));

	if(bFailed) return;

	for(double* d : {&h,&k,&l, &dKi,&dKf,&dE})
		tl::set_eps_0(*d);

	editH->setText(tl::var_to_str<double>(h, g_iPrec).c_str());
	editK->setText(tl::var_to_str<double>(k, g_iPrec).c_str());
	editL->setText(tl::var_to_str<double>(l, g_iPrec).c_str());

	editKi->setText(tl::var_to_str<double>(dKi, g_iPrec).c_str());
	editKf->setText(tl::var_to_str<double>(dKf, g_iPrec).c_str());
	editE->setText(tl::var_to_str<double>(dE, g_iPrec).c_str());

	m_dMono2Theta = th_m*2.;
	m_dAna2Theta = th_a*2.;
	m_dSample2Theta = tt_s;
	m_dSampleTheta = th_s;
	m_bMonoAnaOk = 1;
	m_bSampleOk = 1;

	if(m_bMonoAnaOk && m_bSampleOk)
		labelStatus->setText("Position OK.");
}

void GotoDlg::GetCurPos()
{
	if(!m_bHasParamsRecip)
		return;

	tl::set_eps_0(m_paramsRecip.dki);
	tl::set_eps_0(m_paramsRecip.dkf);
	tl::set_eps_0(m_paramsRecip.Q_rlu[0]);
	tl::set_eps_0(m_paramsRecip.Q_rlu[1]);
	tl::set_eps_0(m_paramsRecip.Q_rlu[2]);

	editKi->setText(tl::var_to_str(m_paramsRecip.dki, g_iPrec).c_str());
	editKf->setText(tl::var_to_str(m_paramsRecip.dkf, g_iPrec).c_str());

	editH->setText(tl::var_to_str(-m_paramsRecip.Q_rlu[0], g_iPrec).c_str());
	editK->setText(tl::var_to_str(-m_paramsRecip.Q_rlu[1], g_iPrec).c_str());
	editL->setText(tl::var_to_str(-m_paramsRecip.Q_rlu[2], g_iPrec).c_str());

	EditedKiKf();
	CalcMonoAna();
	CalcSample();
}

void GotoDlg::RecipParamsChanged(const RecipParams& params)
{
	m_bHasParamsRecip = 1;
	m_paramsRecip = params;
}

void GotoDlg::ButtonBoxClicked(QAbstractButton* pBtn)
{
	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::ApplyRole ||
	   buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		ApplyCurPos();
	}
	else if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::RejectRole)
	{
		reject();
	}

	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		if(m_pSettings)
			m_pSettings->setValue("goto_pos/geo", saveGeometry());

		QDialog::accept();
	}
}

void GotoDlg::showEvent(QShowEvent *pEvt)
{
	if(m_pSettings && m_pSettings->contains("goto_pos/geo"))
		restoreGeometry(m_pSettings->value("goto_pos/geo").toByteArray());

	QDialog::showEvent(pEvt);
}


//------------------------------------------------------------------------------

struct HklPos
{
	double dh, dk, dl;
	double dki, dkf;
	double dE;
};

bool GotoDlg::ApplyCurPos()
{
	if(!m_bMonoAnaOk || !m_bSampleOk)
		return false;

	CrystalOptions crys;
	TriangleOptions triag;

	triag.bChangedMonoTwoTheta = 1;
	triag.dMonoTwoTheta = this->m_dMono2Theta;

	triag.bChangedAnaTwoTheta = 1;
	triag.dAnaTwoTheta = this->m_dAna2Theta;

	triag.bChangedTheta = 1;
	triag.dTheta = this->m_dSampleTheta;

	triag.bChangedAngleKiVec0 = 1;

	double dSampleTheta = m_dSampleTheta;
	if(!m_bSenseS) dSampleTheta = -dSampleTheta;
	triag.dAngleKiVec0 = M_PI/2. - dSampleTheta;
	//log_info("kivec0 = ", triag.dAngleKiVec0/M_PI*180.);
	//log_info("th = ", m_dSampleTheta/M_PI*180.);

	triag.bChangedTwoTheta = 1;
	triag.dTwoTheta = this->m_dSample2Theta;
	// TODO: correct hack in taz.cpp
	if(!m_bSenseS) triag.dTwoTheta = -triag.dTwoTheta;

	emit vars_changed(crys, triag);
	return true;
}

bool GotoDlg::GotoPos(QListWidgetItem* pItem, bool bApply)
{
	if(!pItem) return false;
	HklPos* pPos = (HklPos*)pItem->data(Qt::UserRole).value<void*>();
	if(!pPos) return false;

	editH->setText(tl::var_to_str(pPos->dh, g_iPrec).c_str());
	editK->setText(tl::var_to_str(pPos->dk, g_iPrec).c_str());
	editL->setText(tl::var_to_str(pPos->dl, g_iPrec).c_str());
	editKi->setText(tl::var_to_str(pPos->dki, g_iPrec).c_str());
	editKf->setText(tl::var_to_str(pPos->dkf, g_iPrec).c_str());

	EditedKiKf();
	CalcSample();

	if(bApply)
		return ApplyCurPos();
	return m_bMonoAnaOk && m_bSampleOk;
}

bool GotoDlg::GotoPos(unsigned int iItem)
{
	if(int(iItem) >= listSeq->count())
		return false;
	return GotoPos(listSeq->item(int(iItem)), 1);
}

void GotoDlg::ListItemSelected()
{
	QListWidgetItem *pItem = listSeq->currentItem();
	GotoPos(pItem, 0);
}

void GotoDlg::ListItemDoubleClicked(QListWidgetItem* pItem)
{
	bool bOk = GotoPos(pItem, 1);
	if(!bOk)
		QMessageBox::critical(this, "Error", "Invalid position.");
}

void GotoDlg::AddPosToList(double dh, double dk, double dl, double dki, double dkf)
{
	HklPos *pPos = new HklPos;

	pPos->dh = dh;
	pPos->dk = dk;
	pPos->dl = dl;
	pPos->dki = dki;
	pPos->dkf = dkf;
	pPos->dE = (tl::k2E(pPos->dki/tl::angstrom) - tl::k2E(pPos->dkf/tl::angstrom))/tl::meV;

	tl::set_eps_0(pPos->dh);
	tl::set_eps_0(pPos->dk);
	tl::set_eps_0(pPos->dl);
	tl::set_eps_0(pPos->dki);
	tl::set_eps_0(pPos->dkf);
	tl::set_eps_0(pPos->dE);

	const std::wstring strAA = tl::get_spec_char_utf16("AA") + tl::get_spec_char_utf16("sup-") + tl::get_spec_char_utf16("sup1");

	std::wostringstream ostrHKL;
	ostrHKL.precision(4);
	ostrHKL << "(" << pPos->dh << ", " << pPos->dk << ", " << pPos->dl << ") rlu\n";
	ostrHKL << "ki = " << pPos->dki << " " << strAA;
	ostrHKL << ", kf = " << pPos->dkf << " " << strAA << "\n";
	ostrHKL << "E = " << pPos->dE << " meV";

	QString qstr = QString::fromWCharArray(ostrHKL.str().c_str());
	QListWidgetItem* pItem = new QListWidgetItem(qstr, listSeq);
	pItem->setData(Qt::UserRole, QVariant::fromValue<void*>(pPos));
}

void GotoDlg::AddPosToList()
{
	double dh = tl::str_to_var<double>(editH->text().toStdString());
	double dk = tl::str_to_var<double>(editK->text().toStdString());
	double dl = tl::str_to_var<double>(editL->text().toStdString());
	double dki = tl::str_to_var<double>(editKi->text().toStdString());
	double dkf = tl::str_to_var<double>(editKf->text().toStdString());

	AddPosToList(dh, dk, dl, dki, dkf);
}

void GotoDlg::RemPosFromList()
{
	QListWidgetItem *pItem = listSeq->currentItem();
	if(pItem)
	{
		HklPos* pPos = (HklPos*)pItem->data(Qt::UserRole).value<void*>();
		if(pPos) delete pPos;
		delete pItem;
	}
}

void GotoDlg::ClearList()
{
	while(listSeq->count())
	{
		QListWidgetItem *pItem = listSeq->item(0);
		if(!pItem) break;

		HklPos* pPos = (HklPos*)pItem->data(Qt::UserRole).value<void*>();
		if(pPos) delete pPos;
		delete pItem;
	}
}


void GotoDlg::LoadList()
{
	const std::string strXmlRoot("taz/");

	QString strDirLast = ".";
	if(m_pSettings)
		strDirLast = m_pSettings->value("goto_pos/last_dir", ".").toString();
	QString qstrFile = QFileDialog::getOpenFileName(this,
							"Load Positions",
							strDirLast,
							"TAZ files (*.taz *.TAZ)");
	if(qstrFile == "")
		return;


	std::string strFile = qstrFile.toStdString();
	std::string strDir = tl::get_dir(strFile);

	tl::Xml xml;
	if(!xml.Load(strFile.c_str()))
	{
		QMessageBox::critical(this, "Error", "Could not load positions.");
		return;
	}

	Load(xml, strXmlRoot);
	if(m_pSettings)
		m_pSettings->setValue("goto_pos/last_dir", QString(strDir.c_str()));
}

void GotoDlg::SaveList()
{
	const std::string strXmlRoot("taz/");

	QString strDirLast = ".";
	if(m_pSettings)
		m_pSettings->value("goto_pos/last_dir", ".").toString();
	QString qstrFile = QFileDialog::getSaveFileName(this,
								"Save Positions",
								strDirLast,
								"TAZ files (*.taz *.TAZ)");

	if(qstrFile == "")
		return;

	std::string strFile = qstrFile.toStdString();
	std::string strDir = tl::get_dir(strFile);

	std::map<std::string, std::string> mapConf;
	Save(mapConf, strXmlRoot);

	bool bOk = tl::Xml::SaveMap(strFile.c_str(), mapConf);
	if(!bOk)
		QMessageBox::critical(this, "Error", "Could not save positions.");

	if(bOk && m_pSettings)
		m_pSettings->setValue("goto_pos/last_dir", QString(strDir.c_str()));
}

void GotoDlg::Save(std::map<std::string, std::string>& mapConf, const std::string& strXmlRoot)
{
	mapConf[strXmlRoot + "goto_pos/h"] = editH->text().toStdString();
	mapConf[strXmlRoot + "goto_pos/k"] = editK->text().toStdString();
	mapConf[strXmlRoot + "goto_pos/l"] = editL->text().toStdString();
	mapConf[strXmlRoot + "goto_pos/ki"] = editKi->text().toStdString();
	mapConf[strXmlRoot + "goto_pos/kf"] = editKf->text().toStdString();
	mapConf[strXmlRoot + "goto_pos/cki"] = radioFixedKi->isChecked()?"1":"0";

	// favlist
	for(int iItem=0; iItem<listSeq->count(); ++iItem)
	{
		const QListWidgetItem *pItem = listSeq->item(iItem);
		if(!pItem) continue;
		const HklPos* pPos = (HklPos*)pItem->data(Qt::UserRole).value<void*>();
		if(!pPos) continue;

		std::ostringstream ostrItemBase;
		ostrItemBase << "goto_favlist/pos_" << iItem << "/";
		std::string strItemBase = ostrItemBase.str();

		mapConf[strXmlRoot + strItemBase + "h"] = tl::var_to_str(pPos->dh);
		mapConf[strXmlRoot + strItemBase + "k"] = tl::var_to_str(pPos->dk);
		mapConf[strXmlRoot + strItemBase + "l"] = tl::var_to_str(pPos->dl);
		mapConf[strXmlRoot + strItemBase + "ki"] = tl::var_to_str(pPos->dki);
		mapConf[strXmlRoot + strItemBase + "kf"] = tl::var_to_str(pPos->dkf);
	}
}

void GotoDlg::Load(tl::Xml& xml, const std::string& strXmlRoot)
{
	bool bOk=0;

	editH->setText(std::to_string(xml.Query<double>((strXmlRoot + "goto_pos/h").c_str(), 1., &bOk)).c_str());
	editK->setText(std::to_string(xml.Query<double>((strXmlRoot + "goto_pos/k").c_str(), 0., &bOk)).c_str());
	editL->setText(std::to_string(xml.Query<double>((strXmlRoot + "goto_pos/l").c_str(), 0., &bOk)).c_str());
	editKi->setText(std::to_string(xml.Query<double>((strXmlRoot + "goto_pos/ki").c_str(), 1.4, &bOk)).c_str());
	editKf->setText(std::to_string(xml.Query<double>((strXmlRoot + "goto_pos/kf").c_str(), 1.4, &bOk)).c_str());
	radioFixedKi->setChecked(xml.Query<bool>((strXmlRoot + "goto_pos/cki").c_str(), 0, &bOk));

	// favlist
	ClearList();
	unsigned int iItem=0;
	while(1)
	{
		std::ostringstream ostrItemBase;
		ostrItemBase << "goto_favlist/pos_" << iItem << "/";
		std::string strItemBase = ostrItemBase.str();

		if(!xml.Exists((strXmlRoot + strItemBase).c_str()))
			break;

		double dh = xml.Query<double>((strXmlRoot + strItemBase + "h").c_str(), 0., &bOk);
		double dk = xml.Query<double>((strXmlRoot + strItemBase + "k").c_str(), 0., &bOk);
		double dl = xml.Query<double>((strXmlRoot + strItemBase + "l").c_str(), 0., &bOk);
		double dki = xml.Query<double>((strXmlRoot + strItemBase + "ki").c_str(), 0., &bOk);
		double dkf = xml.Query<double>((strXmlRoot + strItemBase + "kf").c_str(), 0., &bOk);

		AddPosToList(dh, dk, dl, dki, dkf);
		++iItem;
	}

	EditedKiKf();
	CalcSample();
}


#include "GotoDlg.moc"
