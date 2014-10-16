/*
 * Goto Dialog
 * @author Tobias Weber
 * @date 15-oct-2014
 */

#include "GotoDlg.h"
#include "../helper/neutrons.hpp"
#include "../helper/string.h"


using energy = units::quantity<units::si::energy>;
using wavenumber = units::quantity<units::si::wavenumber>;


GotoDlg::GotoDlg(QWidget* pParent) : QDialog(pParent)
{
	this->setupUi(this);
	connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(ButtonBoxClicked(QAbstractButton*)));

	QObject::connect(editKi, SIGNAL(textEdited(const QString&)), this, SLOT(EditedKiKf()));
	QObject::connect(editKf, SIGNAL(textEdited(const QString&)), this, SLOT(EditedKiKf()));
	QObject::connect(editE, SIGNAL(textEdited(const QString&)), this, SLOT(EditedE()));

	std::vector<QObject*> vecObjs {editH, editK, editL};
	for(QObject* pObj : vecObjs)
		QObject::connect(pObj, SIGNAL(textEdited(const QString&)), this, SLOT(CalcSample()));
}

GotoDlg::~GotoDlg()
{}


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

	try
	{
		::get_tas_angles(m_lattice,
					m_vec1, m_vec2,
					dKi, dKf,
					dH, dK, dL,
					m_bSenseS,
					&m_dSampleTheta, &m_dSample2Theta);

		if(::isinf(m_dSample2Theta) || ::isnan(m_dSample2Theta))
			throw Err("Invalid sample 2theta.");
		if(::isinf(m_dSampleTheta) || ::isnan(m_dSampleTheta))
			throw Err("Invalid sample theta.");
	}
	catch(const std::exception& ex)
	{
		editThetaS->setText("invalid");
		edit2ThetaS->setText("invalid");

		log_err(ex.what());
		return;
	}

	editThetaS->setText(var_to_str(m_dSampleTheta/M_PI*180.).c_str());
	edit2ThetaS->setText(var_to_str(m_dSample2Theta/M_PI*180.).c_str());

	m_bSampleOk = 1;
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

	wavenumber ki = dKi / angstrom;
	wavenumber kf = dKf / angstrom;

	bool bMonoOk = 0;
	bool bAnaOk = 0;

	try
	{
		m_dMono2Theta = get_mono_twotheta(ki, m_dMono*angstrom, m_bSenseM) / units::si::radians;
		if(::isinf(m_dMono2Theta) || ::isnan(m_dMono2Theta))
			throw Err("Invalid monochromator angle.");
		bMonoOk = 1;
	}
	catch(const std::exception& ex)
	{
		editThetaM->setText("invalid");
		edit2ThetaM->setText("invalid");

		log_err(ex.what());
	}

	try
	{
		m_dAna2Theta = get_mono_twotheta(kf, m_dAna*angstrom, m_bSenseA) / units::si::radians;
		if(::isinf(m_dAna2Theta) || ::isnan(m_dAna2Theta))
			throw Err("Invalid analysator angle.");
		bAnaOk = 1;
	}
	catch(const std::exception& ex)
	{
		editThetaA->setText("invalid");
		edit2ThetaA->setText("invalid");

		log_err(ex.what());
	}

	if(!bMonoOk || !bAnaOk)
		return;


	double dTMono = m_dMono2Theta / 2.;
	double dTAna = m_dAna2Theta / 2.;

	edit2ThetaM->setText(var_to_str(m_dMono2Theta/M_PI*180.).c_str());
	editThetaM->setText(var_to_str(dTMono/M_PI*180.).c_str());
	edit2ThetaA->setText(var_to_str(m_dAna2Theta/M_PI*180.).c_str());
	editThetaA->setText(var_to_str(dTAna/M_PI*180.).c_str());

	m_bMonoAnaOk = 1;
}

void GotoDlg::EditedKiKf()
{
	bool bKiOk=0, bKfOk=0;
	double dKi = editKi->text().toDouble(&bKiOk);
	double dKf = editKf->text().toDouble(&bKfOk);
	if(!bKiOk || !bKfOk) return;

	energy Ei = k2E(dKi / angstrom);
	energy Ef = k2E(dKf / angstrom);

	energy E = Ei-Ef;
	double dE = E/one_meV;

	std::string strE = var_to_str<double>(dE);
	editE->setText(strE.c_str());

	CalcMonoAna();
	CalcSample();
}

void GotoDlg::EditedE()
{
	bool bOk = 0;
	double dE = editE->text().toDouble(&bOk);
	if(!bOk) return;
	energy E = dE * one_meV;

	bool bImag=0;
	wavenumber k_E = E2k(E, bImag);
	double dSign = 1.;
	if(bImag) dSign = -1.;

	if(radioFixedKi->isChecked())
	{
		bool bKOk = 0;
		double dKi = editKi->text().toDouble(&bKOk);
		if(!bKOk) return;

		wavenumber ki = dKi / angstrom;
		wavenumber kf = units::sqrt(ki*ki - dSign*k_E*k_E);

		double dKf = kf*angstrom;
		std::string strKf = var_to_str<double>(dKf);
		editKf->setText(strKf.c_str());
	}
	else
	{
		bool bKOk = 0;
		double dKf = editKf->text().toDouble(&bKOk);
		if(!bKOk) return;

		wavenumber kf = dKf / angstrom;
		wavenumber ki = units::sqrt(kf*kf + dSign*k_E*k_E);

		double dKi = ki*angstrom;
		std::string strKi = var_to_str<double>(dKi);
		editKi->setText(strKi.c_str());
	}

	CalcMonoAna();
	CalcSample();
}


void GotoDlg::ButtonBoxClicked(QAbstractButton* pBtn)
{
	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::ApplyRole ||
	   buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		if(m_bMonoAnaOk && m_bSampleOk)
		{
			CrystalOptions crys;
			TriangleOptions triag;

			triag.bChangedMonoTwoTheta = 1;
			triag.dMonoTwoTheta = this->m_dMono2Theta;

			triag.bChangedAnaTwoTheta = 1;
			triag.dAnaTwoTheta = this->m_dAna2Theta;

			//triag.bChangedTheta = 1;
			//triag.dTheta = this->m_dSampleTheta;

			triag.bChangedAngleKiVec0 = 1;
			triag.dAngleKiVec0 = M_PI/2. - this->m_dSampleTheta;

			triag.bChangedTwoTheta = 1;
			triag.dTwoTheta = this->m_dSample2Theta;

			emit vars_changed(crys, triag);
		}
	}
	else if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::RejectRole)
	{
		reject();
	}

	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		QDialog::accept();
	}
}


#include "GotoDlg.moc"
