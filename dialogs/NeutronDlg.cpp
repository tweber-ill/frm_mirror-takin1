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


NeutronDlg::NeutronDlg(QWidget* pParent)
			: QDialog(pParent)
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
}

NeutronDlg::~NeutronDlg()
{}


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

#include "NeutronDlg.moc"
