/*
 * Neutron Properties Dialog
 * @author Tobias Weber
 * @date jul-2013, 28-may-2014
 */

#include "NeutronDlg.h"
#include <boost/units/io.hpp>

#include "../helper/string.h"
#include "../helper/math.h"
#include "../helper/misc.h"
#include "../helper/neutrons.hpp"

#include <sstream>
#include <iostream>
#include <map>
#include <vector>


NeutronDlg::NeutronDlg(QWidget* pParent, QSettings *pSett)
			: QDialog(pParent), m_pSettings(pSett)
{
	setupUi(this);
	setupConstants();

	QObject::connect(editLam, SIGNAL(textEdited(const QString&)), this, SLOT(CalcNeutronLam()));
	QObject::connect(editE, SIGNAL(textEdited(const QString&)), this, SLOT(CalcNeutronE()));
	QObject::connect(editOm, SIGNAL(textEdited(const QString&)), this, SLOT(CalcNeutronOm()));
	QObject::connect(editF, SIGNAL(textEdited(const QString&)), this, SLOT(CalcNeutronF()));
	QObject::connect(editV, SIGNAL(textEdited(const QString&)), this, SLOT(CalcNeutronv()));
	QObject::connect(editK, SIGNAL(textEdited(const QString&)), this, SLOT(CalcNeutronk()));
	QObject::connect(editT, SIGNAL(textEdited(const QString&)), this, SLOT(CalcNeutronT()));

	CalcNeutronLam();



	std::vector<QLineEdit*> editsDir = {editBraggDirN, editBraggDirLam, editBraggDirD, editBraggDirT, editBraggDirTT};
	std::vector<QLineEdit*> editsReci = {editBraggReciN, editBraggReciLam, editBraggReciQ, editBraggReciT, editBraggReciTT};
	std::vector<QRadioButton*> radioDir = {/*radioBraggDirN,*/ radioBraggDirLam, radioBraggDirD, radioBraggDirTT};
	std::vector<QRadioButton*> radioReci = {/*radioBraggReciN,*/ radioBraggReciLam, radioBraggReciQ, radioBraggReciTT};

	QObject::connect(editBraggDirT, SIGNAL(textEdited(const QString&)), this, SLOT(RealThetaEdited()));
	QObject::connect(editBraggReciT, SIGNAL(textEdited(const QString&)), this, SLOT(RecipThetaEdited()));
	QObject::connect(editBraggDirTT, SIGNAL(textEdited(const QString&)), this, SLOT(RealTwoThetaEdited()));
	QObject::connect(editBraggReciTT, SIGNAL(textEdited(const QString&)), this, SLOT(RecipTwoThetaEdited()));

	for(QLineEdit* pEdit : editsDir)
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(CalcBraggReal()));
	for(QLineEdit* pEdit : editsReci)
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(CalcBraggRecip()));
	for(QRadioButton* pRadio : radioDir)
	{
		QObject::connect(pRadio, SIGNAL(toggled(bool)), this, SLOT(EnableRealEdits()));
		QObject::connect(pRadio, SIGNAL(toggled(bool)), this, SLOT(CalcBraggReal()));
	}
	for(QRadioButton* pRadio : radioReci)
	{
		QObject::connect(pRadio, SIGNAL(toggled(bool)), this, SLOT(EnableRecipEdits()));
		QObject::connect(pRadio, SIGNAL(toggled(bool)), this, SLOT(CalcBraggRecip()));
	}


	EnableRealEdits();
	EnableRecipEdits();
	CalcBraggReal();
	CalcBraggRecip();
}

NeutronDlg::~NeutronDlg()
{}


// -----------------------------------------------------------------------------


void NeutronDlg::CalcNeutronLam()
{
	std::string strInput = editLam->text().toStdString();

	units::quantity<units::si::length> lam_n
					= str_to_var<double>(strInput) * angstrom;

	units::quantity<units::si::wavenumber> k_n = lam2k(lam_n);
	units::quantity<units::si::momentum> p_n = lam2p(lam_n);
	units::quantity<units::si::energy> E_n = p_n*p_n / (2.*co::m_n);

	editE->setText(var_to_str<double>(E_n / one_meV).c_str());
	editOm->setText(var_to_str<double>(E_n / co::hbar * units::si::second / 1e12).c_str());
	editF->setText(var_to_str<double>(E_n / co::h * units::si::second / 1e12).c_str());
	editK->setText(var_to_str<double>(k_n * angstrom).c_str());
	editV->setText(var_to_str<double>((p_n / co::m_n) * units::si::second / units::si::meter).c_str());
	editT->setText(var_to_str<double>((E_n / co::k_B) / units::si::kelvin).c_str());
}

void NeutronDlg::CalcNeutronk()
{
	std::string strInput = editK->text().toStdString();

	units::quantity<units::si::wavenumber> k_n
			= str_to_var<double>(strInput) / angstrom;
	units::quantity<units::si::length> lam_n = k2lam(k_n);
	units::quantity<units::si::momentum> p_n = lam2p(lam_n);
	units::quantity<units::si::energy> E_n = p_n*p_n / (2.*co::m_n);

	editLam->setText(var_to_str<double>(lam_n / angstrom).c_str());
	editE->setText(var_to_str<double>(E_n / one_meV).c_str());
	editOm->setText(var_to_str<double>(E_n / co::hbar * units::si::second / 1e12).c_str());
	editF->setText(var_to_str<double>(E_n / co::h * units::si::second / 1e12).c_str());
	editV->setText(var_to_str<double>((p_n / co::m_n) * units::si::second / units::si::meter).c_str());
	editT->setText(var_to_str<double>((E_n / co::k_B) / units::si::kelvin).c_str());
}

void NeutronDlg::CalcNeutronv()
{
	std::string strInput = editV->text().toStdString();

	units::quantity<units::si::velocity> v_n
			= str_to_var<double>(strInput) * units::si::meter / units::si::second;
	units::quantity<units::si::wavenumber> k_n = v2k(v_n);
	units::quantity<units::si::length> lam_n = k2lam(k_n);
	units::quantity<units::si::momentum> p_n = lam2p(lam_n);
	units::quantity<units::si::energy> E_n = p_n*p_n / (2.*co::m_n);

	editLam->setText(var_to_str<double>(lam_n / angstrom).c_str());
	editE->setText(var_to_str<double>(E_n / one_meV).c_str());
	editOm->setText(var_to_str<double>(E_n / co::hbar * units::si::second / 1e12).c_str());
	editF->setText(var_to_str<double>(E_n / co::h * units::si::second / 1e12).c_str());
	editK->setText(var_to_str<double>(k_n * angstrom).c_str());
	editT->setText(var_to_str<double>((E_n / co::k_B) / units::si::kelvin).c_str());
}

void NeutronDlg::CalcNeutronE()
{
	std::string strInput = editE->text().toStdString();

	bool bImag = 0;
	units::quantity<units::si::energy> E_n
			= str_to_var<double>(strInput) * one_meV;
	units::quantity<units::si::wavenumber> k_n = E2k(E_n, bImag);
	units::quantity<units::si::length> lam_n = k2lam(k_n);
	units::quantity<units::si::momentum> p_n = lam2p(lam_n);

	editOm->setText(var_to_str<double>(E_n / co::hbar * units::si::second / 1e12).c_str());
	editF->setText(var_to_str<double>(E_n / co::h * units::si::second / 1e12).c_str());
	editLam->setText(var_to_str<double>(lam_n / angstrom).c_str());
	editK->setText(var_to_str<double>(k_n * angstrom).c_str());
	editV->setText(var_to_str<double>((p_n / co::m_n) * units::si::second / units::si::meter).c_str());
	editT->setText(var_to_str<double>((E_n / co::k_B) / units::si::kelvin).c_str());
}

void NeutronDlg::CalcNeutronOm()
{
	std::string strInput = editOm->text().toStdString();

	bool bImag = 0;
	units::quantity<units::si::energy> E_n
			= str_to_var<double>(strInput) / units::si::second * co::hbar * 1e12;
	units::quantity<units::si::wavenumber> k_n = E2k(E_n, bImag);
	units::quantity<units::si::length> lam_n = k2lam(k_n);
	units::quantity<units::si::momentum> p_n = lam2p(lam_n);

	editE->setText(var_to_str<double>(E_n / one_meV).c_str());
	editF->setText(var_to_str<double>(E_n / co::h * units::si::second / 1e12).c_str());
	editLam->setText(var_to_str<double>(lam_n / angstrom).c_str());
	editK->setText(var_to_str<double>(k_n * angstrom).c_str());
	editV->setText(var_to_str<double>((p_n / co::m_n) * units::si::second / units::si::meter).c_str());
	editT->setText(var_to_str<double>((E_n / co::k_B) / units::si::kelvin).c_str());
}

void NeutronDlg::CalcNeutronF()
{
	std::string strInput = editF->text().toStdString();

	bool bImag = 0;
	units::quantity<units::si::energy> E_n
			= str_to_var<double>(strInput) / units::si::second * co::h * 1e12;
	units::quantity<units::si::wavenumber> k_n = E2k(E_n, bImag);
	units::quantity<units::si::length> lam_n = k2lam(k_n);
	units::quantity<units::si::momentum> p_n = lam2p(lam_n);

	editE->setText(var_to_str<double>(E_n / one_meV).c_str());
	editOm->setText(var_to_str<double>(E_n / co::hbar * units::si::second / 1e12).c_str());
	editLam->setText(var_to_str<double>(lam_n / angstrom).c_str());
	editK->setText(var_to_str<double>(k_n * angstrom).c_str());
	editV->setText(var_to_str<double>((p_n / co::m_n) * units::si::second / units::si::meter).c_str());
	editT->setText(var_to_str<double>((E_n / co::k_B) / units::si::kelvin).c_str());
}

void NeutronDlg::CalcNeutronT()
{
	std::string strInput = editT->text().toStdString();

	bool bImag;
	units::quantity<units::si::temperature> T_n
		= str_to_var<double>(strInput) * units::si::kelvin;
	units::quantity<units::si::energy> E_n = T_n * co::k_B;
	units::quantity<units::si::wavenumber> k_n = E2k(E_n, bImag);
	units::quantity<units::si::length> lam_n = k2lam(k_n);
	units::quantity<units::si::momentum> p_n = lam2p(lam_n);

	editLam->setText(var_to_str<double>(lam_n / angstrom).c_str());
	editK->setText(var_to_str<double>(k_n * angstrom).c_str());
	editV->setText(var_to_str<double>((p_n / co::m_n) * units::si::second / units::si::meter).c_str());
	editE->setText(var_to_str<double>(E_n / one_meV).c_str());
	editOm->setText(var_to_str<double>(E_n / co::hbar * units::si::second / 1e12).c_str());
	editF->setText(var_to_str<double>(E_n / co::h * units::si::second / 1e12).c_str());
}


// -----------------------------------------------------------------------------


void NeutronDlg::setupConstants()
{
	struct Constant
	{
		std::string strSymbol;
		std::string strName;

		std::string strVal;
	};

	std::vector<Constant> vecConsts;

	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << one_eV;

		Constant constant;
		constant.strSymbol = "eV";
		constant.strName = "1 electron volt";
		constant.strVal = insert_before<std::string>(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::h;

		Constant constant;
		constant.strSymbol = "h";
		constant.strName = "Planck constant";
		constant.strVal = insert_before<std::string>(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << (co::h / one_eV) << " eV";

		Constant constant;
		constant.strSymbol = "h";
		constant.strName = "Planck constant";
		constant.strVal = insert_before<std::string>(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::hbar;

		Constant constant;
		constant.strSymbol = "hbar";
		constant.strName = "Planck constant";
		constant.strVal = insert_before<std::string>(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << (co::hbar / one_eV) << " eV";

		Constant constant;
		constant.strSymbol = "hbar";
		constant.strName = "Planck constant";
		constant.strVal = insert_before<std::string>(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::m_n;

		Constant constant;
		constant.strSymbol = "m_n";
		constant.strName = "Neutron mass";
		constant.strVal = insert_before<std::string>(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::g_n;

		Constant constant;
		constant.strSymbol = "g_n";
		constant.strName = "Neutron g";
		constant.strVal = insert_before<std::string>(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::gamma_n;

		Constant constant;
		constant.strSymbol = "gamma_n";
		constant.strName = "Neutron gyromagnetic ratio";
		constant.strVal = insert_before<std::string>(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::mu_n;

		Constant constant;
		constant.strSymbol = "mu_n";
		constant.strName = "Neutron magnetic moment";
		constant.strVal = insert_before<std::string>(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::mu_N;

		Constant constant;
		constant.strSymbol = "mu_N";
		constant.strName = "Nuclear magneton";
		constant.strVal = insert_before<std::string>(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::c;

		Constant constant;
		constant.strSymbol = "c";
		constant.strName = "Vacuum speed of light";
		constant.strVal = insert_before<std::string>(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}


	tableConst->setColumnCount(2);
	tableConst->setRowCount(vecConsts.size());
	tableConst->setColumnWidth(1, 200);
	//tableConst->verticalHeader()->setDefaultSectionSize(tableConst->verticalHeader()->minimumSectionSize()+2);


	for(unsigned int iConst=0; iConst<vecConsts.size(); ++iConst)
	{
		const Constant& constant = vecConsts[iConst];

		QTableWidgetItem *pConstSym = new QTableWidgetItem();
		pConstSym->setText(constant.strSymbol.c_str());
		tableConst->setVerticalHeaderItem(iConst, pConstSym);

		QTableWidgetItem *pConstName = new QTableWidgetItem();
		pConstName->setText(constant.strName.c_str());
		tableConst->setItem(iConst,0,pConstName);

		QTableWidgetItem *pConstVal = new QTableWidgetItem();
		pConstVal->setText(constant.strVal.c_str());
		tableConst->setItem(iConst,1,pConstVal);

		pConstName->setFlags(pConstName->flags() & ~Qt::ItemIsEditable);
		//pConstVal->setFlags(pConstVal->flags() & ~Qt::ItemIsEditable);
	}
}


// -----------------------------------------------------------------------------

void NeutronDlg::EnableRealEdits()
{
	//void (QLineEdit::*pFunc)(bool) = &QLineEdit::setDisabled;
	void (QLineEdit::*pFunc)(bool) = &QLineEdit::setReadOnly;

	(editBraggDirLam->*pFunc)(0);
	(editBraggDirD->*pFunc)(0);
	(editBraggDirT->*pFunc)(0);
	(editBraggDirTT->*pFunc)(0);

	if(radioBraggDirLam->isChecked())
		(editBraggDirLam->*pFunc)(1);
	else if(radioBraggDirD->isChecked())
		(editBraggDirD->*pFunc)(1);
	else if(radioBraggDirTT->isChecked())
	{
		(editBraggDirT->*pFunc)(1);
		(editBraggDirTT->*pFunc)(1);
	}
}

void NeutronDlg::EnableRecipEdits()
{
	//void (QLineEdit::*pFunc)(bool) = &QLineEdit::setDisabled;
	void (QLineEdit::*pFunc)(bool) = &QLineEdit::setReadOnly;

	(editBraggReciLam->*pFunc)(0);
	(editBraggReciQ->*pFunc)(0);
	(editBraggReciT->*pFunc)(0);
	(editBraggReciTT->*pFunc)(0);

	if(radioBraggReciLam->isChecked())
		(editBraggReciLam->*pFunc)(1);
	else if(radioBraggReciQ->isChecked())
		(editBraggReciQ->*pFunc)(1);
	else if(radioBraggReciTT->isChecked())
	{
		(editBraggReciT->*pFunc)(1);
		(editBraggReciTT->*pFunc)(1);
	}
}


void NeutronDlg::SetEditTT(QLineEdit *pEditT, QLineEdit *pEditTT)
{
	std::string strT = pEditT->text().toStdString();
	double dT = str_to_var<double>(strT);
	std::string strTT = var_to_str(dT*2.);
	pEditTT->setText(strTT.c_str());
}

void NeutronDlg::RealThetaEdited()
{ SetEditTT(editBraggDirT, editBraggDirTT); }
void NeutronDlg::RecipThetaEdited()
{ SetEditTT(editBraggReciT, editBraggReciTT); }


void NeutronDlg::SetEditT(QLineEdit *pEditT, QLineEdit *pEditTT)
{
	std::string strTT = pEditTT->text().toStdString();
	double dTT = str_to_var<double>(strTT);
	std::string strT = var_to_str(dTT*0.5);
	pEditT->setText(strT.c_str());
}

void NeutronDlg::RealTwoThetaEdited()
{ SetEditT(editBraggDirT, editBraggDirTT); }
void NeutronDlg::RecipTwoThetaEdited()
{ SetEditT(editBraggReciT, editBraggReciTT); }


void NeutronDlg::CalcBraggReal()
{
	std::string strN = editBraggDirN->text().toStdString();
	std::string strLam = editBraggDirLam->text().toStdString();
	std::string strD = editBraggDirD->text().toStdString();
	std::string strTT = editBraggDirTT->text().toStdString();

	int iOrder = str_to_var<int>(strN);
	units::quantity<units::si::length> lam = str_to_var<double>(strLam)*angstrom;
	units::quantity<units::si::length> d = str_to_var<double>(strD)*angstrom;
	units::quantity<units::si::plane_angle> tt = str_to_var<double>(strTT)/180.*M_PI*units::si::radians;

	if(radioBraggDirLam->isChecked())
	{
		lam = ::bragg_real_lam(d, tt, double(iOrder));
		std::string strLam = var_to_str(double(lam/angstrom));
		editBraggDirLam->setText(strLam.c_str());
	}
	else if(radioBraggDirD->isChecked())
	{
		d = ::bragg_real_d(lam, tt, double(iOrder));
		std::string strD = var_to_str(double(d/angstrom));
		editBraggDirD->setText(strD.c_str());
	}
	else if(radioBraggDirTT->isChecked())
	{
		tt = ::bragg_real_twotheta(d, lam, double(iOrder));
		std::string strTT = var_to_str(double(tt/units::si::radian) /M_PI*180.);
		std::string strT = var_to_str(double(0.5*tt/units::si::radian) /M_PI*180.);
		editBraggDirTT->setText(strTT.c_str());
		editBraggDirT->setText(strT.c_str());
	}
}

void NeutronDlg::CalcBraggRecip()
{
	std::string strN = editBraggReciN->text().toStdString();
	std::string strLam = editBraggReciLam->text().toStdString();
	std::string strQ = editBraggReciQ->text().toStdString();
	std::string strTT = editBraggReciTT->text().toStdString();

	int iOrder = str_to_var<int>(strN);
	units::quantity<units::si::length> lam = str_to_var<double>(strLam)*angstrom;
	units::quantity<units::si::wavenumber> Q = str_to_var<double>(strQ)/angstrom;
	units::quantity<units::si::plane_angle> tt = str_to_var<double>(strTT)/180.*M_PI*units::si::radians;

	if(radioBraggReciLam->isChecked())
	{
		lam = ::bragg_recip_lam(Q, tt, double(iOrder));
		std::string strLam = var_to_str(double(lam/angstrom));
		editBraggReciLam->setText(strLam.c_str());
	}
	else if(radioBraggReciQ->isChecked())
	{
		Q = ::bragg_recip_Q(lam, tt, double(iOrder));
		std::string strQ = var_to_str(double(Q*angstrom));
		editBraggReciQ->setText(strQ.c_str());
	}
	else if(radioBraggReciTT->isChecked())
	{
		tt = ::bragg_recip_twotheta(Q, lam, double(iOrder));
		std::string strTT = var_to_str(double(tt/units::si::radian) /M_PI*180.);
		std::string strT = var_to_str(double(0.5*tt/units::si::radian) /M_PI*180.);
		editBraggReciTT->setText(strTT.c_str());
		editBraggReciT->setText(strT.c_str());
	}
}


// -----------------------------------------------------------------------------


void NeutronDlg::accept()
{
	if(m_pSettings)
	{
		m_pSettings->setValue("neutron_props/geo", saveGeometry());
		m_pSettings->setValue("neutron_props/lam", editLam->text());
	}

	QDialog::accept();
}

void NeutronDlg::showEvent(QShowEvent *pEvt)
{
	if(m_pSettings)
	{
		if(m_pSettings->contains("neutron_props/geo"))
			restoreGeometry(m_pSettings->value("neutron_props/geo").toByteArray());

		bool bOk = 0;
		double dLam = m_pSettings->value("neutron_props/lam").toDouble(&bOk);
		if(!bOk)
			dLam = 5.;

		std::string strLam = var_to_str(dLam);
		editLam->setText(strLam.c_str());

		CalcNeutronLam();
	}

	QDialog::showEvent(pEvt);
}

#include "NeutronDlg.moc"
