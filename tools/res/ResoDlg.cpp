/*
 * resolution calculation
 * @author tweber
 * @date may-2013, apr-2014
 */

#include "ResoDlg.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <map>

//#include "../../main/settings.h"
#include "helper/string.h"
#include "helper/spec_char.h"
#include "helper/misc.h"
#include "helper/math.h"

#include <QtGui/QPainter>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QGridLayout>


ResoDlg::ResoDlg(QWidget *pParent, QSettings* pSettings)
			: QDialog(pParent), m_bDontCalc(1), m_pSettings(pSettings)
{
	setupUi(this);
	m_vecSpinBoxes = {spinMonod, spinMonoMosaic, spinAnad,
						spinAnaMosaic, spinSampleMosaic, spinkfix,
						spinE, spinQ, spinHCollMono, spinHCollBSample,
						spinHCollASample, spinHCollAna, spinVCollMono,
						spinVCollBSample, spinVCollASample, spinVCollAna,
						spinMonoRefl, spinAnaEffic,

						spinMonoW, spinMonoH, spinMonoThick, spinMonoCurvH, spinMonoCurvV,
						spinSampleW_Q, spinSampleW_perpQ, spinSampleH,
						spinAnaW, spinAnaH, spinAnaThick, spinAnaCurvH, spinAnaCurvV,
						spinSrcW, spinSrcH,
						spinGuideDivH, spinGuideDivV,
						spinDetW, spinDetH,
						spinDistMonoSample, spinDistSampleAna, spinDistAnaDet, spinDistSrcMono};

	m_vecSpinNames = {"reso/mono_d", "reso/mono_mosaic", "reso/ana_d",
					"reso/ana_mosaic", "reso/sample_mosaic", "reso/k_fix",
					"reso/E", "reso/Q", "reso/h_coll_mono", "reso/h_coll_before_sample",
					"reso/h_coll_after_sample", "reso/h_coll_ana",
					"reso/v_coll_mono", "reso/v_coll_before_sample",
					"reso/v_coll_after_sample", "reso/v_coll_ana",
					"reso/mono_refl", "reso/ana_effic",

					"reso/pop_mono_w", "reso/pop_mono_h", "reso/pop_mono_thick", "reso/pop_mono_curvh", "reso/pop_mono_curvv",
					"reso/pop_sample_wq", "reso/pop_sampe_wperpq", "reso/pop_sample_h",
					"reso/pop_ana_w", "reso/pop_ana_h", "reso/pop_ana_thick", "reso/pop_ana_curvh", "reso/pop_ana_curvv",
					"reso/pop_src_w", "reso/pop_src_h",
					"reso/pop_guide_divh", "reso/pop_guide_divv",
					"reso/pop_det_w", "reso/pop_det_h",
					"reso/pop_dist_mono_sample", "reso/pop_dist_sample_ana", "reso/pop_dist_ana_det", "reso/pop_dist_src_mono"};


	m_vecCheckBoxes = {checkAnaCurvH, checkAnaCurvV, checkMonoCurvH, checkMonoCurvV};
	m_vecCheckNames = {"reso/pop_ana_use_curvh", "reso/pop_ana_use_curvv", "reso/pop_mono_use_curvh", "reso/pop_mono_use_curvv"};


	m_vecRadioPlus = {radioFixedki, radioMonoScatterPlus, radioAnaScatterPlus,
						radioSampleScatterPlus, radioConstMon, radioCN,
						radioSampleCub, radioSrcRect, radioDetRect};
	m_vecRadioMinus = {radioFixedkf, radioMonoScatterMinus, radioAnaScatterMinus,
						radioSampleScatterMinus, radioConstTime, radioPop,
						radioSampleCyl, radioSrcCirc, radioDetCirc};
	m_vecRadioNames = {"reso/check_fixed_ki", "reso/mono_scatter_sense", "reso/ana_scatter_sense",
						"reso/sample_scatter_sense", "reso/meas_const_mon",
						"reso/algo",
						"reso/pop_sample_cuboid", "reso/pop_source_rect", "reso/pop_det_rect"};


	UpdateUI();
	QObject::connect(radioFixedki, SIGNAL(toggled(bool)), this, SLOT(UpdateUI()));

	ReadLastConfig();

	QObject::connect(groupGuide, SIGNAL(toggled(bool)), this, SLOT(Calc()));

	QCheckBox* pCheckBoxes[] = {checkAnaCurvH, checkAnaCurvV, checkMonoCurvH, checkMonoCurvV};
	for(QCheckBox* pbox : pCheckBoxes)
		QObject::connect(pbox, SIGNAL(toggled(bool)), this, SLOT(Calc()));

	for(QDoubleSpinBox* pSpinBox : m_vecSpinBoxes)
		QObject::connect(pSpinBox, SIGNAL(valueChanged(double)), this, SLOT(Calc()));
	for(QRadioButton* pRadio : m_vecRadioPlus)
		QObject::connect(pRadio, SIGNAL(toggled(bool)), this, SLOT(Calc()));


	connect(checkElli4dAutoCalc, SIGNAL(stateChanged(int)), this, SLOT(checkAutoCalcElli4dChanged()));
	connect(btnCalcElli4d, SIGNAL(clicked()), this, SLOT(CalcElli4d()));
	connect(btnMCGenerate, SIGNAL(clicked()), this, SLOT(MCGenerate()));
	connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(ButtonBoxClicked(QAbstractButton*)));

	m_bDontCalc = 0;
	Calc();
}

ResoDlg::~ResoDlg()
{}


void ResoDlg::UpdateUI()
{
	const std::string& strAA = get_spec_char_utf8("AA") +
								get_spec_char_utf8("sup-")+
								get_spec_char_utf8("sup1");

	std::string strKi = std::string("k_i (") + strAA + "):";
	std::string strKf = std::string("k_f (") + strAA + "):";

	if(radioFixedki->isChecked())
	{
		labelkfix->setText(QString::fromUtf8(strKi.c_str()));
		labelkvar->setText(QString::fromUtf8(strKf.c_str()));
	}
	else
	{
		labelkfix->setText(QString::fromUtf8(strKf.c_str()));
		labelkvar->setText(QString::fromUtf8(strKi.c_str()));
	}
}

void ResoDlg::Calc()
{
	m_bEll4dCurrent = 0;

	if(m_bDontCalc)
		return;

	const units::quantity<units::si::length> angstrom = 1e-10 * units::si::meter;

	PopParams& cn = m_pop;
	CNResults &res = m_res;

	// CN
	cn.mono_d = spinMonod->value() * angstrom;
	cn.mono_mosaic = spinMonoMosaic->value() / (180.*60.) * M_PI * units::si::radians;
	cn.ana_d = spinAnad->value() * angstrom;
	cn.ana_mosaic = spinAnaMosaic->value() / (180.*60.) * M_PI * units::si::radians;
	cn.sample_mosaic = spinSampleMosaic->value() / (180.*60.) * M_PI * units::si::radians;

	cn.bki_fix = radioFixedki->isChecked();
	cn.ki = cn.kf = spinkfix->value() / angstrom;
	cn.E = spinE->value() * (1e-3 * codata::e * units::si::volts);
	cn.Q = /*std::fabs*/(spinQ->value()) / angstrom;

	cn.dmono_sense = (radioMonoScatterPlus->isChecked() ? +1. : -1.);
	cn.dana_sense = (radioAnaScatterPlus->isChecked() ? +1. : -1.);
	cn.dsample_sense = (radioSampleScatterPlus->isChecked() ? +1. : -1.);
	//if(spinQ->value() < 0.)
	//	cn.dsample_sense = -cn.dsample_sense;

	cn.coll_h_pre_mono = spinHCollMono->value() / (180.*60.) * M_PI * units::si::radians;
	cn.coll_h_pre_sample = spinHCollBSample->value() / (180.*60.) * M_PI * units::si::radians;
	cn.coll_h_post_sample = spinHCollASample->value() / (180.*60.) * M_PI * units::si::radians;
	cn.coll_h_post_ana = spinHCollAna->value() / (180.*60.) * M_PI * units::si::radians;

	cn.coll_v_pre_mono = spinVCollMono->value() / (180.*60.) * M_PI * units::si::radians;
	cn.coll_v_pre_sample = spinVCollBSample->value() / (180.*60.) * M_PI * units::si::radians;
	cn.coll_v_post_sample = spinVCollASample->value() / (180.*60.) * M_PI * units::si::radians;
	cn.coll_v_post_ana = spinVCollAna->value() / (180.*60.) * M_PI * units::si::radians;

	cn.dmono_refl = spinMonoRefl->value();
	cn.dana_effic = spinAnaEffic->value();
	cn.bConstMon = radioConstMon->isChecked();

	// Pop
	cn.mono_w = spinMonoW->value()*0.01*units::si::meter;
	cn.mono_h = spinMonoH->value()*0.01*units::si::meter;
	cn.mono_thick = spinMonoThick->value()*0.01*units::si::meter;
	cn.mono_curvh = spinMonoCurvH->value()*0.01*units::si::meter;
	cn.mono_curvv = spinMonoCurvV->value()*0.01*units::si::meter;
	cn.bMonoIsCurvedH = checkMonoCurvH->isChecked();
	cn.bMonoIsCurvedV = checkMonoCurvV->isChecked();

	cn.ana_w = spinAnaW->value()*0.01*units::si::meter;
	cn.ana_h = spinAnaH->value()*0.01*units::si::meter;
	cn.ana_thick = spinAnaThick->value()*0.01*units::si::meter;
	cn.ana_curvh = spinAnaCurvH->value()*0.01*units::si::meter;
	cn.ana_curvv = spinAnaCurvV->value()*0.01*units::si::meter;
	cn.bAnaIsCurvedH = checkAnaCurvH->isChecked();
	cn.bAnaIsCurvedV = checkAnaCurvV->isChecked();

	cn.bSampleCub = radioSampleCub->isChecked();
	cn.sample_w_q = spinSampleW_Q->value()*0.01*units::si::meter;
	cn.sample_w_perpq = spinSampleW_perpQ->value()*0.01*units::si::meter;
	cn.sample_h = spinSampleH->value()*0.01*units::si::meter;

	cn.bSrcRect = radioSrcRect->isChecked();
	cn.src_w = spinSrcW->value()*0.01*units::si::meter;
	cn.src_h = spinSrcH->value()*0.01*units::si::meter;

	cn.bDetRect = radioDetRect->isChecked();
	cn.det_w = spinDetW->value()*0.01*units::si::meter;
	cn.det_h = spinDetH->value()*0.01*units::si::meter;

	cn.bGuide = groupGuide->isChecked();
	cn.guide_div_h = spinGuideDivH->value() / (180.*60.) * M_PI * units::si::radians;
	cn.guide_div_v = spinGuideDivV->value() / (180.*60.) * M_PI * units::si::radians;

	cn.dist_mono_sample = spinDistMonoSample->value()*0.01*units::si::meter;
	cn.dist_sample_ana = spinDistSampleAna->value()*0.01*units::si::meter;
	cn.dist_ana_det = spinDistAnaDet->value()*0.01*units::si::meter;
	cn.dist_src_mono = spinDistSrcMono->value()*0.01*units::si::meter;

	const bool bUseCN = radioCN->isChecked();
	res = (bUseCN ? calc_cn(cn) : calc_pop(cn));

	//if(m_pInstDlg) m_pInstDlg->SetParams(cn, res);
	//if(m_pScatterDlg) m_pScatterDlg->SetParams(cn, res);

	if(res.bOk)
	{
		// --------------------------------------------------------------------------------
		// Vanadium width
		Ellipse ellVa = calc_res_ellipse(res.reso, res.Q_avg, 0, 3, 1, 2, -1);
		//std::cout << ellVa.phi/M_PI*180. << std::endl;
		double dVanadiumFWHM = ellVa.y_hwhm*2.;
		if(std::fabs(ellVa.phi) >= M_PI/4.)
			dVanadiumFWHM = ellVa.x_hwhm*2.;
		// --------------------------------------------------------------------------------

		const std::string& strAA_1 = get_spec_char_utf8("AA")
						+ get_spec_char_utf8("sup-")
						+ get_spec_char_utf8("sup1");
		const std::string& strAA_3 = get_spec_char_utf8("AA")
						+ get_spec_char_utf8("sup-")
						+ get_spec_char_utf8("sup3");

		std::ostringstream ostrRes;

		//ostrRes << std::scientific;
		ostrRes.precision(8);
		ostrRes << "Resolution Volume: " << res.dR0 << " meV " << strAA_3 << "\n\n";
		ostrRes << "Bragg FWHMs:\n";
		ostrRes << "\tQ_para: " << res.dBraggFWHMs[0] << " " << strAA_1 << "\n";
		ostrRes << "\tQ_ortho: " << res.dBraggFWHMs[1] << " " << strAA_1 << "\n";
		ostrRes << "\tQ_z: " << res.dBraggFWHMs[2] << " " << strAA_1 << "\n";
		ostrRes << "\tE: " << res.dBraggFWHMs[3] << " meV\n\n";
		ostrRes << "Vanadium FWHM: " << dVanadiumFWHM << " meV\n";
		ostrRes << "\n\n";
		ostrRes << "Resolution Matrix (Q_para, Q_ortho, Q_z, E): \n\n";

		for(unsigned int i=0; i<res.reso.size1(); ++i)
		{
			for(unsigned int j=0; j<res.reso.size2(); ++j)
				ostrRes << std::setw(18) << res.reso(i,j);

			if(i!=res.reso.size1()-1)
				ostrRes << "\n";
		}

		labelStatus->setText("Calculation successful.");
		labelResult->setText(QString::fromUtf8(ostrRes.str().c_str()));

		std::ostringstream ostrkvar;

		double dKVar = cn.bki_fix?(cn.kf*angstrom):(cn.ki*angstrom);
		//ostrkvar.precision(4);
		ostrkvar << dKVar;
		labelkvar_val->setText(ostrkvar.str().c_str());


		if(checkElli4dAutoCalc->isChecked())
		{
			CalcElli4d();
			m_bEll4dCurrent = 1;
		}

		EmitResults();
	}
	else
	{
		QString strErr = "Error: ";
		strErr += res.strErr.c_str();
		labelStatus->setText(QString("<font color='red'>") + strErr + QString("</font>"));

		labelkvar_val->setText("<error>");
	}
}

void ResoDlg::EmitResults()
{
	emit ResoResults(m_res.reso, m_res.Q_avg);
}

void ResoDlg::WriteLastConfig()
{
	if(!m_pSettings)
		return;

	for(unsigned int iSpinBox=0; iSpinBox<m_vecSpinBoxes.size(); ++iSpinBox)
		m_pSettings->setValue(m_vecSpinNames[iSpinBox].c_str(), m_vecSpinBoxes[iSpinBox]->value());
	for(unsigned int iRadio=0; iRadio<m_vecRadioPlus.size(); ++iRadio)
		m_pSettings->setValue(m_vecRadioNames[iRadio].c_str(), m_vecRadioPlus[iRadio]->isChecked());
	for(unsigned int iCheck=0; iCheck<m_vecCheckBoxes.size(); ++iCheck)
		m_pSettings->setValue(m_vecCheckNames[iCheck].c_str(), m_vecCheckBoxes[iCheck]->isChecked());

	m_pSettings->setValue("reso/use_guide", groupGuide->isChecked());
}

void ResoDlg::ReadLastConfig()
{
	if(!m_pSettings)
		return;

	bool bOldDontCalc = m_bDontCalc;
	m_bDontCalc = 1;

	for(unsigned int iSpinBox=0; iSpinBox<m_vecSpinBoxes.size(); ++iSpinBox)
	{
		if(!m_pSettings->contains(m_vecSpinNames[iSpinBox].c_str()))
			continue;
		m_vecSpinBoxes[iSpinBox]->setValue(m_pSettings->value(m_vecSpinNames[iSpinBox].c_str()).value<double>());
	}

	for(unsigned int iCheckBox=0; iCheckBox<m_vecCheckBoxes.size(); ++iCheckBox)
	{
		if(!m_pSettings->contains(m_vecCheckNames[iCheckBox].c_str()))
			continue;
		m_vecCheckBoxes[iCheckBox]->setChecked(m_pSettings->value(m_vecCheckNames[iCheckBox].c_str()).value<bool>());
	}

	for(unsigned int iRadio=0; iRadio<m_vecRadioPlus.size(); ++iRadio)
	{
		if(!m_pSettings->contains(m_vecRadioNames[iRadio].c_str()))
			continue;

		bool bChecked = m_pSettings->value(m_vecRadioNames[iRadio].c_str()).value<bool>();
		if(bChecked)
		{
			m_vecRadioPlus[iRadio]->setChecked(1);
			//m_vecRadioMinus[iRadio]->setChecked(0);;
		}
		else
		{
			//m_vecRadioPlus[iRadio]->setChecked(0);
			m_vecRadioMinus[iRadio]->setChecked(1);;
		}
	}

	groupGuide->setChecked(m_pSettings->value("reso/use_guide").value<bool>());

	m_bDontCalc = bOldDontCalc;
	Calc();
}

void ResoDlg::Save(std::map<std::string, std::string>& mapConf, const std::string& strXmlRoot)
{
	for(unsigned int iSpinBox=0; iSpinBox<m_vecSpinBoxes.size(); ++iSpinBox)
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << m_vecSpinBoxes[iSpinBox]->value();

		mapConf[strXmlRoot + m_vecSpinNames[iSpinBox]] = ostrVal.str();
	}

	for(unsigned int iCheckBox=0; iCheckBox<m_vecCheckBoxes.size(); ++iCheckBox)
		mapConf[strXmlRoot + m_vecCheckNames[iCheckBox]] = (m_vecCheckBoxes[iCheckBox]->isChecked() ? "1" : "0");

	for(unsigned int iRadio=0; iRadio<m_vecRadioPlus.size(); ++iRadio)
		mapConf[strXmlRoot + m_vecRadioNames[iRadio]] = (m_vecRadioPlus[iRadio]->isChecked() ? "1" : "0");

	mapConf[strXmlRoot + "reso/use_guide"] = groupGuide->isChecked() ? "1" : "0";
}

void ResoDlg::Load(Xml& xml, const std::string& strXmlRoot)
{
	bool bOldDontCalc = m_bDontCalc;
	m_bDontCalc = 1;

	bool bOk=0;
	for(unsigned int iSpinBox=0; iSpinBox<m_vecSpinBoxes.size(); ++iSpinBox)
		m_vecSpinBoxes[iSpinBox]->setValue(xml.Query<double>((strXmlRoot+m_vecSpinNames[iSpinBox]).c_str(), 0., &bOk));

	for(unsigned int iCheck=0; iCheck<m_vecCheckBoxes.size(); ++iCheck)
	{
		int bChecked = xml.Query<int>((strXmlRoot+m_vecCheckNames[iCheck]).c_str(), 0, &bOk);
		m_vecCheckBoxes[iCheck]->setChecked(bChecked);
	}

	for(unsigned int iRadio=0; iRadio<m_vecRadioPlus.size(); ++iRadio)
	{
		int bChecked = xml.Query<int>((strXmlRoot+m_vecRadioNames[iRadio]).c_str(), 0, &bOk);
		if(bChecked)
			m_vecRadioPlus[iRadio]->setChecked(1);
		else
			m_vecRadioMinus[iRadio]->setChecked(1);;
	}

	groupGuide->setChecked(xml.Query<int>((strXmlRoot+"reso/use_guide").c_str(), 0, &bOk));

	m_bDontCalc = bOldDontCalc;
	Calc();
}


void ResoDlg::ButtonBoxClicked(QAbstractButton* pBtn)
{
	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::ApplyRole ||
	   buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		WriteLastConfig();
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

void ResoDlg::hideEvent(QHideEvent *event)
{
	m_pSettings->setValue("reso/wnd_geo", saveGeometry());
}

void ResoDlg::showEvent(QShowEvent *event)
{
	restoreGeometry(m_pSettings->value("reso/wnd_geo").toByteArray());
}

void ResoDlg::ResoParamsChanged(const ResoParams& params)
{
	bool bOldDontCalc = m_bDontCalc;
	m_bDontCalc = 1;

	if(params.bSensesChanged[0]) params.bScatterSenses[0] ? radioMonoScatterPlus->setChecked(1) : radioMonoScatterMinus->setChecked(1);
	if(params.bSensesChanged[1]) params.bScatterSenses[1] ? radioSampleScatterPlus->setChecked(1) : radioSampleScatterMinus->setChecked(1);
	if(params.bSensesChanged[2]) params.bScatterSenses[2] ? radioAnaScatterPlus->setChecked(1) : radioAnaScatterMinus->setChecked(1);

	if(params.bMonoDChanged) spinMonod->setValue(params.dMonoD);
	if(params.bAnaDChanged) spinAnad->setValue(params.dAnaD);

	m_bDontCalc = bOldDontCalc;
	Calc();
}

void ResoDlg::RecipParamsChanged(const RecipParams& parms)
{
	bool bOldDontCalc = m_bDontCalc;
	m_bDontCalc = 1;

	double dQ = parms.dQ;
	if(parms.d2Theta < 0.)
		dQ = -dQ;

	spinQ->setValue(dQ);
	spinE->setValue(parms.dE);
	spinkfix->setValue(radioFixedki->isChecked() ? parms.dki : parms.dkf);

	//m_pop.Q_vec = ::make_vec({parms.Q[0], parms.Q[1], parms.Q[2]});
	m_bDontCalc = bOldDontCalc;
	Calc();
}

void ResoDlg::RealParamsChanged(const RealParams& parms)
{
	// TODO
}

// TODO: also send locally changed params back to taz


// --------------------------------------------------------------------------------
// Monte-Carlo stuff

void ResoDlg::checkAutoCalcElli4dChanged()
{
	if(checkElli4dAutoCalc->isChecked() && !m_bEll4dCurrent)
		CalcElli4d();
}

void ResoDlg::CalcElli4d()
{
	m_ell4d = calc_res_ellipsoid4d(m_res.reso, m_res.Q_avg);

	std::ostringstream ostrElli;
	ostrElli << "Ellipsoid volume: " << m_ell4d.vol << "\n\n";
	ostrElli << "Ellipsoid offsets:\n"
			<< "\tQx = " << m_ell4d.x_offs << "\n"
			<< "\tQy = " << m_ell4d.y_offs << "\n"
			<< "\tQz = " << m_ell4d.z_offs << "\n"
			<< "\tE = " << m_ell4d.w_offs << "\n\n";
	ostrElli << "Ellipsoid HWHMs (unsorted):\n"
			<< "\t" << m_ell4d.x_hwhm << "\n"
			<< "\t" << m_ell4d.y_hwhm << "\n"
			<< "\t" << m_ell4d.z_hwhm << "\n"
			<< "\t" << m_ell4d.w_hwhm << "\n\n";

	labelElli->setText(QString::fromUtf8(ostrElli.str().c_str()));
}

void ResoDlg::MCGenerate()
{
	if(!m_bEll4dCurrent)
		CalcElli4d();

	QString strLastDir = m_pSettings ? m_pSettings->value("reso/mc_dir", ".").toString() : ".";
	QString _strFile = QFileDialog::getSaveFileName(this, "Save MC neutron data...", strLastDir, "DAT files (*.dat);;All files (*.*)");
	if(_strFile == "")
		return;

	std::string strFile = _strFile.toStdString();

	const int iNeutrons = spinMCNeutrons->value();
	const bool bCenter = checkMCCenter->isChecked();

	std::ofstream ofstr(strFile);
	if(!ofstr.is_open())
	{
		QMessageBox::critical(this, "Error", "Cannot open file.");
		return;
	}

	std::vector<ublas::vector<double>> vecNeutrons;
	mc_neutrons(m_ell4d, iNeutrons, bCenter, vecNeutrons);

	ofstr.precision(16);

	ofstr << "#" << std::setw(23) << m_ell4d.x_lab
			<< std::setw(24) << m_ell4d.y_lab
			<< std::setw(24) << m_ell4d.z_lab
			<< std::setw(24) << m_ell4d.w_lab << "\n";

	for(const ublas::vector<double>& vecNeutron : vecNeutrons)
	{
		for(unsigned i=0; i<4; ++i)
			ofstr << std::setw(24) << vecNeutron[i];
		ofstr << "\n";
	}

	if(m_pSettings)
		m_pSettings->setValue("reso/mc_dir", QString(::get_dir(strFile).c_str()));
}
// --------------------------------------------------------------------------------


#include "ResoDlg.moc"
