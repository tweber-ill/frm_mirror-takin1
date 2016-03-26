/*
 * monte carlo convolution tool
 * @author tweber
 * @date aug-2015
 * @license GPLv2
 */

#include "ConvoDlg.h"
#include "tlibs/string/string.h"
#include "tlibs/math/math.h"
#include "tlibs/helper/thread.h"

#ifndef NO_PY
	#include "sqw_py.h"
#endif
#include "TASReso.h"
#include "helper/globals.h"
#include "helper/qthelper.h"

#include <iostream>
#include <fstream>
#include <QFileDialog>
#include <QMessageBox>

using t_real = t_real_reso;


ConvoDlg::ConvoDlg(QWidget* pParent, QSettings* pSett)
	: QDialog(pParent, Qt::WindowTitleHint|Qt::WindowCloseButtonHint|Qt::WindowMinMaxButtonsHint),
		m_pSett(pSett)
{
	setupUi(this);
	if(m_pSett)
	{
		QFont font;
		if(m_pSett->contains("main/font_gen") && font.fromString(m_pSett->value("main/font_gen", "").toString()))
			setFont(font);
	}

	btnStart->setIcon(load_icon("res/media-playback-start.svg"));
	btnStop->setIcon(load_icon("res/media-playback-stop.svg"));
	btnSaveResult->setIcon(load_icon("res/document-save-as.svg"));


	m_plotwrap.reset(new QwtPlotWrapper(plot, 2, true));
	m_plotwrap->GetPlot()->setAxisTitle(QwtPlot::xBottom, "");
	m_plotwrap->GetPlot()->setAxisTitle(QwtPlot::yLeft, "S (a.u.)");

	QPen penCurve;
	penCurve.setColor(QColor(0,0,0x99));
	penCurve.setWidth(2);
	m_plotwrap->GetCurve(0)->setPen(penCurve);
	m_plotwrap->GetCurve(0)->setStyle(QwtPlotCurve::CurveStyle::Lines);
	m_plotwrap->GetCurve(0)->setTitle("S(Q,w)");

	QPen penPoints;
	penPoints.setColor(QColor(0xff,0,0));
	penPoints.setWidth(4);
	m_plotwrap->GetCurve(1)->setPen(penPoints);
	m_plotwrap->GetCurve(1)->setStyle(QwtPlotCurve::CurveStyle::Dots);
	m_plotwrap->GetCurve(1)->setTitle("S(Q,w)");


	m_pSqwParamDlg = new SqwParamDlg(this, m_pSett);
	QObject::connect(this, SIGNAL(SqwLoaded(const std::vector<SqwBase::t_var>&)),
		m_pSqwParamDlg, SLOT(SqwLoaded(const std::vector<SqwBase::t_var>&)));
	QObject::connect(m_pSqwParamDlg, SIGNAL(SqwParamsChanged(const std::vector<SqwBase::t_var>&)),
		this, SLOT(SqwParamsChanged(const std::vector<SqwBase::t_var>&)));

	QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(ButtonBoxClicked(QAbstractButton*)));

	QObject::connect(btnBrowseCrys, SIGNAL(clicked()), this, SLOT(browseCrysFiles()));
	QObject::connect(btnBrowseRes, SIGNAL(clicked()), this, SLOT(browseResoFiles()));
	QObject::connect(btnBrowseSqw, SIGNAL(clicked()), this, SLOT(browseSqwFiles()));
	QObject::connect(btnSqwParams, SIGNAL(clicked()), this, SLOT(showSqwParamDlg()));
	QObject::connect(btnSaveResult, SIGNAL(clicked()), this, SLOT(SaveResult()));

	QObject::connect(comboSqw, SIGNAL(currentIndexChanged(int)), this, SLOT(SqwModelChanged(int)));
	QObject::connect(editSqw, SIGNAL(textChanged(const QString&)), this, SLOT(createSqwModel(const QString&)));

	QObject::connect(btnStart, SIGNAL(clicked()), this, SLOT(Start()));
	QObject::connect(btnStop, SIGNAL(clicked()), this, SLOT(Stop()));

	LoadSettings();
}


ConvoDlg::~ConvoDlg()
{
	if(m_pth)
	{
		if(m_pth->joinable())
			m_pth->join();
		delete m_pth;
		m_pth = nullptr;
	}

	if(m_pSqw)
	{
		delete m_pSqw;
		m_pSqw = nullptr;
	}

	if(m_pSqwParamDlg)
	{
		delete m_pSqwParamDlg;
		m_pSqwParamDlg = nullptr;
	}
}


void ConvoDlg::SaveResult()
{
	QFileDialog::Option fileopt = QFileDialog::Option(0);
	if(m_pSett && !m_pSett->value("main/native_dialogs", 1).toBool())
		fileopt = QFileDialog::DontUseNativeDialog;

	QString strDirLast = ".";
	if(m_pSett)
		strDirLast = m_pSett->value("convo/last_dir_result", ".").toString();

	QString strFile = QFileDialog::getSaveFileName(this,
		"Save Scan", strDirLast, "Data Files (*.dat *.DAT)", nullptr, fileopt);

	if(strFile == "")
		return;

	std::string strFile1 = strFile.toStdString();
	std::string strDir = tl::get_dir(strFile1);

	std::ofstream ofstr(strFile1);
	if(!ofstr)
	{
		QMessageBox::critical(this, "Error", "Could not open file.");
		return;
	}

	std::string strResult = textResult->toPlainText().toStdString();
	ofstr.write(strResult.c_str(), strResult.size());

	if(m_pSett)
		m_pSett->setValue("convo/last_dir_result", QString(strDir.c_str()));
}


void ConvoDlg::SqwModelChanged(int)
{
	editSqw->clear();
	emit SqwLoaded(std::vector<SqwBase::t_var>{});
}

void ConvoDlg::createSqwModel(const QString& qstrFile)
{
	if(m_pSqw)
	{
		delete m_pSqw;
		m_pSqw = nullptr;

		emit SqwLoaded(std::vector<SqwBase::t_var>{});
	}


	const int iSqwModel = comboSqw->currentIndex();
	std::string strSqwFile = qstrFile.toStdString();
	tl::trim(strSqwFile);

	if(iSqwModel!=3 && strSqwFile == "")
	{
		QMessageBox::critical(this, "Error", "Could not load S(q,w) config file.");
		return;
	}

	switch(iSqwModel)
	{
		case 0:
			m_pSqw = new SqwKdTree(strSqwFile.c_str());
			break;
		case 1:
#ifdef NO_PY
			QMessageBox::critical(this, "Error", "Compiled without python support.");
			return;
#else
			m_pSqw = new SqwPy(strSqwFile.c_str());
			break;
#endif
		case 2:
			/*m_pSqw = new SqwPhonon(tl::make_vec({4.,4.,0}),
					tl::make_vec({0.,0.,1.}), tl::make_vec({1.,-1.,0.}),
					55.5/(M_PI/2.)/std::sqrt(2.), M_PI/2., 0.1, 0.1,
					22.5/(M_PI/2.), M_PI/2., 0.1, 0.1,
					28.5/(M_PI/2.)/std::sqrt(2.), M_PI/2., 0.1, 0.1);*/
			m_pSqw = new SqwPhonon(strSqwFile.c_str());
			break;
		case 3:
			if(strSqwFile.length())
				m_pSqw = new SqwElast(strSqwFile.c_str());
			else
				m_pSqw = new SqwElast();
			break;
		default:
		{
			QMessageBox::critical(this, "Error", "Unknown S(q,w) model selected.");
			return;
		}
	}

	if(m_pSqw && m_pSqw->IsOk())
		emit SqwLoaded(m_pSqw->GetVars());
	else
	{
		QMessageBox::critical(this, "Error", "Could not create S(q,w).");
		return;
	}
}


void ConvoDlg::SqwParamsChanged(const std::vector<SqwBase::t_var>& vecVars)
{
	if(!m_pSqw)
		return;
	m_pSqw->SetVars(vecVars);

#ifndef NDEBUG
	// check: read parameters back in
	emit SqwLoaded(m_pSqw->GetVars());
#endif
}


void ConvoDlg::Start()
{
	m_atStop.store(false);

	btnStart->setEnabled(false);
	tabSettings->setEnabled(false);
	btnStop->setEnabled(true);
	tabWidget->setCurrentWidget(tabPlot);

	bool bForceDeferred = false;
	const int iSqwModel = comboSqw->currentIndex();
	if(iSqwModel == 1)
		bForceDeferred = true;
	Qt::ConnectionType connty = bForceDeferred ? Qt::ConnectionType::DirectConnection
			: Qt::ConnectionType::BlockingQueuedConnection;

	std::function<void()> fkt = [this, connty, bForceDeferred]
	{
		std::function<void()> fktEnableButtons = [this]
		{
			QMetaObject::invokeMethod(btnStop, "setEnabled", Q_ARG(bool, false));
			QMetaObject::invokeMethod(tabSettings, "setEnabled", Q_ARG(bool, true));
			QMetaObject::invokeMethod(btnStart, "setEnabled", Q_ARG(bool, true));
		};

		const bool bUseR0 = true;
		const unsigned int iNumNeutrons = spinNeutrons->value();

		const unsigned int iNumSteps = spinStepCnt->value();
		std::vector<t_real> vecH = tl::linspace<t_real,t_real>(spinStartH->value(), spinStopH->value(), iNumSteps);
		std::vector<t_real> vecK = tl::linspace<t_real,t_real>(spinStartK->value(), spinStopK->value(), iNumSteps);
		std::vector<t_real> vecL = tl::linspace<t_real,t_real>(spinStartL->value(), spinStopL->value(), iNumSteps);
		std::vector<t_real> vecE = tl::linspace<t_real,t_real>(spinStartE->value(), spinStopE->value(), iNumSteps);

		std::string strScanVar = "";
		std::vector<t_real> *pVecScanX = nullptr;
		if(!tl::float_equal(spinStartH->value(), spinStopH->value(), 0.0001))
		{
			pVecScanX = &vecH;
			strScanVar = "h (rlu)";
		}
		else if(!tl::float_equal(spinStartK->value(), spinStopK->value(), 0.0001))
		{
			pVecScanX = &vecK;
			strScanVar = "k (rlu)";
		}
		else if(!tl::float_equal(spinStartL->value(), spinStopL->value(), 0.0001))
		{
			pVecScanX = &vecL;
			strScanVar = "l (rlu)";
		}
		else if(!tl::float_equal(spinStartE->value(), spinStopE->value(), 0.0001))
		{
			pVecScanX = &vecE;
			strScanVar = "E (meV)";
		}
		else
		{
			//QMessageBox::critical(this, "Error", "No scan variable found.");
			fktEnableButtons();
			return;
		}

		//QMetaObject::invokeMethod(plot, "setAxisTitle",
		//	Q_ARG(int, QwtPlot::xBottom),
		//	Q_ARG(const QString&, QString(strScanVar.c_str())));
		//plot->setAxisTitle(QwtPlot::xBottom, strScanVar.c_str());


		TASReso reso;
		if(!reso.LoadRes(editRes->text().toStdString().c_str()))
		{
			//QMessageBox::critical(this, "Error", "Could not load resolution file.");
			fktEnableButtons();
			return;
		}

		if(!reso.LoadLattice(editCrys->text().toStdString().c_str()))
		{
			//QMessageBox::critical(this, "Error", "Could not load crystal file.");
			fktEnableButtons();
			return;
		}

		reso.SetAlgo(ResoAlgo(comboAlgo->currentIndex()));
		reso.SetKiFix(comboFixedK->currentIndex()==0);
		reso.SetKFix(spinKfix->value());

		const int iFocMono = comboFocMono->currentIndex();
		const int iFocAna = comboFocAna->currentIndex();

		unsigned ifocMode = unsigned(ResoFocus::FOC_NONE);
		if(iFocMono == 1) ifocMode |= unsigned(ResoFocus::FOC_MONO_H);	// horizontal
		else if(iFocMono == 2) ifocMode |= unsigned(ResoFocus::FOC_MONO_V);	// vertical
		else if(iFocMono == 3) ifocMode |= unsigned(ResoFocus::FOC_MONO_V)|unsigned(ResoFocus::FOC_MONO_H);		// both
		if(iFocAna == 1) ifocMode |= unsigned(ResoFocus::FOC_ANA_H);	// horizontal
		else if(iFocAna == 2) ifocMode |= unsigned(ResoFocus::FOC_ANA_V);	// vertical
		else if(iFocAna == 3) ifocMode |= unsigned(ResoFocus::FOC_ANA_V)|unsigned(ResoFocus::FOC_ANA_H);		// both

		ResoFocus focMode = ResoFocus(ifocMode);
		reso.SetOptimalFocus(focMode);


		if(m_pSqw == nullptr || !m_pSqw->IsOk())
		{
			//QMessageBox::critical(this, "Error", "No valid S(q,w) model loaded.");
			fktEnableButtons();
			return;
		}



		std::ostringstream ostrOut;
		ostrOut << "#\n";
		ostrOut << "# Format: h k l E S\n";
		ostrOut << "# Neutrons: " << iNumNeutrons << "\n";
		ostrOut << "#\n";

		QMetaObject::invokeMethod(progress, "setMaximum", Q_ARG(int, iNumSteps));
		QMetaObject::invokeMethod(progress, "setValue", Q_ARG(int, 0));

		QMetaObject::invokeMethod(textResult, "clear", connty);


		m_vecQ.clear();
		m_vecS.clear();

		m_vecQ.reserve(iNumSteps);
		m_vecS.reserve(iNumSteps);

		unsigned int iNumThreads = bForceDeferred ? 0 : std::thread::hardware_concurrency();

		tl::ThreadPool<std::pair<bool, t_real>()> tp(iNumThreads);
		auto& lstFuts = tp.GetFutures();

		for(unsigned int iStep=0; iStep<iNumSteps; ++iStep)
		{
			t_real dCurH = vecH[iStep];
			t_real dCurK = vecK[iStep];
			t_real dCurL = vecL[iStep];
			t_real dCurE = vecE[iStep];

			tp.AddTask(
			[&reso, dCurH, dCurK, dCurL, dCurE, iNumNeutrons, this]() -> std::pair<bool, t_real>
			{
				if(m_atStop.load()) return std::pair<bool, t_real>(false, 0.);

				TASReso localreso = reso;
				std::vector<ublas::vector<t_real>> vecNeutrons;

				try
				{
					if(!localreso.SetHKLE(dCurH, dCurK, dCurL, dCurE))
					{
						std::ostringstream ostrErr;
						ostrErr << "Invalid crystal position: (" <<
							dCurH << " " << dCurK << " " << dCurL << ") rlu, "
							<< dCurE << " meV.";
						throw tl::Err(ostrErr.str().c_str());
					}
				}
				catch(const std::exception& ex)
				{
					//QMessageBox::critical(this, "Error", ex.what());
					tl::log_err(ex.what());
					return std::pair<bool, t_real>(false, 0.);
				}

				Ellipsoid4d elli = localreso.GenerateMC(iNumNeutrons, vecNeutrons);

				t_real dS = 0.;
				t_real dhklE_mean[4] = {0., 0., 0., 0.};

				for(const ublas::vector<t_real>& vecHKLE : vecNeutrons)
				{
					if(m_atStop.load()) return std::pair<bool, t_real>(false, 0.);

					dS += (*m_pSqw)(vecHKLE[0], vecHKLE[1], vecHKLE[2], vecHKLE[3]);

					for(int i=0; i<4; ++i)
						dhklE_mean[i] += vecHKLE[i];
				}

				dS /= t_real(iNumNeutrons);
				for(int i=0; i<4; ++i)
					dhklE_mean[i] /= t_real(iNumNeutrons);

				if(bUseR0)
					dS *= localreso.GetResoResults().dResVol;
				if(bUseR0 && localreso.GetResoParams().bCalcR0)
					dS *= localreso.GetResoResults().dR0;

				return std::pair<bool, t_real>(true, dS);
			});
		}

		tp.StartTasks();

		auto iterTask = tp.GetTasks().begin();
		unsigned int iStep = 0;
		for(auto &fut : lstFuts)
		{
			if(m_atStop.load()) break;

			// deferred (in main thread), eval this task manually
			if(iNumThreads == 0)
			{
				(*iterTask)();
				++iterTask;
			}

			std::pair<bool, t_real> pairS = fut.get();
			if(!pairS.first)
				break;
			t_real dS = pairS.second;

			ostrOut.precision(16);
			ostrOut << std::left << std::setw(20) << vecH[iStep] << " "
				<< std::left << std::setw(20) << vecK[iStep] << " "
				<< std::left << std::setw(20) << vecL[iStep] << " "
				<< std::left << std::setw(20) << vecE[iStep] << " "
				<< std::left << std::setw(20) << dS << "\n";


			m_vecQ.push_back((*pVecScanX)[iStep]);
			m_vecS.push_back(dS);

			set_qwt_data<t_real_reso>()(*m_plotwrap, m_vecQ, m_vecS, 0, false);
			set_qwt_data<t_real_reso>()(*m_plotwrap, m_vecQ, m_vecS, 1, false);
			//m_plotwrap->SetData(m_vecQ, m_vecS, 0, false);
			//m_plotwrap->SetData(m_vecQ, m_vecS, 1, false);

			//set_zoomer_base(m_plotwrap->GetZoomer(), m_vecQ, m_vecS, true);
			QMetaObject::invokeMethod(m_plotwrap->GetPlot(), "replot", connty);

			QMetaObject::invokeMethod(textResult, "setPlainText", connty,
				Q_ARG(const QString&, QString(ostrOut.str().c_str())));

			QMetaObject::invokeMethod(progress, "setValue", Q_ARG(int, iStep+1));
			++iStep;
		}

		ostrOut << "# ---------------- EOF ----------------\n";

		QMetaObject::invokeMethod(textResult, "setPlainText", connty,
			Q_ARG(const QString&, QString(ostrOut.str().c_str())));

		fktEnableButtons();
	};


	if(bForceDeferred)
		fkt();
	else
	{
		if(m_pth) { if(m_pth->joinable()) m_pth->join(); delete m_pth; }
		m_pth = new std::thread(std::move(fkt));
	}
}


void ConvoDlg::Stop()
{
	m_atStop.store(true);
}


void ConvoDlg::browseCrysFiles()
{
	QFileDialog::Option fileopt = QFileDialog::Option(0);
	if(m_pSett && !m_pSett->value("main/native_dialogs", 1).toBool())
		fileopt = QFileDialog::DontUseNativeDialog;

	QString strDirLast = ".";
	if(m_pSett)
		strDirLast = m_pSett->value("convo/last_dir_crys", ".").toString();
	QString strFile = QFileDialog::getOpenFileName(this,
		"Open Crystal File...", strDirLast, "Takin files (*.taz *.TAZ)",
		nullptr, fileopt);
	if(strFile == "")
		return;

	editCrys->setText(strFile);

	std::string strDir = tl::get_dir(strFile.toStdString());
	if(m_pSett)
		m_pSett->setValue("convo/last_dir_crys", QString(strDir.c_str()));
}

void ConvoDlg::browseResoFiles()
{
	QFileDialog::Option fileopt = QFileDialog::Option(0);
	if(m_pSett && !m_pSett->value("main/native_dialogs", 1).toBool())
		fileopt = QFileDialog::DontUseNativeDialog;

	QString strDirLast = ".";
	if(m_pSett)
		strDirLast = m_pSett->value("convo/last_dir_reso", ".").toString();
	QString strFile = QFileDialog::getOpenFileName(this,
		"Open Resolution File...", strDirLast, "Takin files (*.taz *.TAZ)",
		nullptr, fileopt);
	if(strFile == "")
		return;

	editRes->setText(strFile);

	std::string strDir = tl::get_dir(strFile.toStdString());
	if(m_pSett)
		m_pSett->setValue("convo/last_dir_reso", QString(strDir.c_str()));
}

void ConvoDlg::browseSqwFiles()
{
	QFileDialog::Option fileopt = QFileDialog::Option(0);
	if(m_pSett && !m_pSett->value("main/native_dialogs", 1).toBool())
		fileopt = QFileDialog::DontUseNativeDialog;

	QString strDirLast = ".";
	if(m_pSett)
		strDirLast = m_pSett->value("convo/last_dir_sqw", ".").toString();
	QString strFile = QFileDialog::getOpenFileName(this,
		"Open S(q,w) File...", strDirLast, "All S(q,w) files (*.dat *.DAT *.py *.PY)",
		nullptr, fileopt);
	if(strFile == "")
		return;

	editSqw->setText(strFile);

	std::string strDir = tl::get_dir(strFile.toStdString());
	if(m_pSett)
		m_pSett->setValue("convo/last_dir_sqw", QString(strDir.c_str()));
}

void ConvoDlg::showSqwParamDlg()
{
	m_pSqwParamDlg->show();
	m_pSqwParamDlg->activateWindow();
}

void ConvoDlg::LoadSettings()
{
	if(m_pSett)
	{
		if(m_pSett->contains("monteconvo/geo"))
			restoreGeometry(m_pSett->value("monteconvo/geo").toByteArray());

		if(m_pSett->contains("monteconvo/algo"))
			comboAlgo->setCurrentIndex(m_pSett->value("monteconvo/algo").toInt());
		if(m_pSett->contains("monteconvo/fixedk"))
			comboFixedK->setCurrentIndex(m_pSett->value("monteconvo/fixedk").toInt());
		if(m_pSett->contains("monteconvo/mono_foc"))
			comboFocMono->setCurrentIndex(m_pSett->value("monteconvo/mono_foc").toInt());
		if(m_pSett->contains("monteconvo/ana_foc"))
			comboFocAna->setCurrentIndex(m_pSett->value("monteconvo/ana_foc").toInt());
		if(m_pSett->contains("monteconvo/sqw"))
			comboSqw->setCurrentIndex(m_pSett->value("monteconvo/sqw").toInt());

		if(m_pSett->contains("monteconvo/crys"))
			editCrys->setText(m_pSett->value("monteconvo/crys").toString());
		if(m_pSett->contains("monteconvo/instr"))
			editRes->setText(m_pSett->value("monteconvo/instr").toString());
		if(m_pSett->contains("monteconvo/sqw_conf"))
			editSqw->setText(m_pSett->value("monteconvo/sqw_conf").toString());

		if(m_pSett->contains("monteconvo/h_from"))
			spinStartH->setValue(m_pSett->value("monteconvo/h_from").toDouble());
		if(m_pSett->contains("monteconvo/k_from"))
			spinStartK->setValue(m_pSett->value("monteconvo/k_from").toDouble());
		if(m_pSett->contains("monteconvo/l_from"))
			spinStartL->setValue(m_pSett->value("monteconvo/l_from").toDouble());
		if(m_pSett->contains("monteconvo/E_from"))
			spinStartE->setValue(m_pSett->value("monteconvo/E_from").toDouble());
		if(m_pSett->contains("monteconvo/h_to"))
			spinStopH->setValue(m_pSett->value("monteconvo/h_to").toDouble());
		if(m_pSett->contains("monteconvo/k_to"))
			spinStopK->setValue(m_pSett->value("monteconvo/k_to").toDouble());
		if(m_pSett->contains("monteconvo/l_to"))
			spinStopL->setValue(m_pSett->value("monteconvo/l_to").toDouble());
		if(m_pSett->contains("monteconvo/E_to"))
			spinStopE->setValue(m_pSett->value("monteconvo/E_to").toDouble());

		if(m_pSett->contains("monteconvo/kfix"))
			spinKfix->setValue(m_pSett->value("monteconvo/kfix").toDouble());
		if(m_pSett->contains("monteconvo/neutron_count"))
			spinNeutrons->setValue(m_pSett->value("monteconvo/neutron_count").toDouble());
		if(m_pSett->contains("monteconvo/step_count"))
			spinStepCnt->setValue(m_pSett->value("monteconvo/step_count").toDouble());
	}
}

void ConvoDlg::showEvent(QShowEvent *pEvt)
{
	//LoadSettings();
	QDialog::showEvent(pEvt);
}

void ConvoDlg::ButtonBoxClicked(QAbstractButton *pBtn)
{
	if(pBtn == buttonBox->button(QDialogButtonBox::Close))
	{
		if(m_pSett)
		{
			m_pSett->setValue("monteconvo/geo", saveGeometry());

			m_pSett->setValue("monteconvo/algo", comboAlgo->currentIndex());
			m_pSett->setValue("monteconvo/fixedk", comboFixedK->currentIndex());
			m_pSett->setValue("monteconvo/mono_foc", comboFocMono->currentIndex());
			m_pSett->setValue("monteconvo/ana_foc", comboFocAna->currentIndex());
			m_pSett->setValue("monteconvo/sqw", comboSqw->currentIndex());

			m_pSett->setValue("monteconvo/crys", editCrys->text());
			m_pSett->setValue("monteconvo/instr", editRes->text());
			m_pSett->setValue("monteconvo/sqw_conf", editSqw->text());

			m_pSett->setValue("monteconvo/h_from", spinStartH->value());
			m_pSett->setValue("monteconvo/k_from", spinStartK->value());
			m_pSett->setValue("monteconvo/l_from", spinStartL->value());
			m_pSett->setValue("monteconvo/E_from", spinStartE->value());
			m_pSett->setValue("monteconvo/h_to", spinStopH->value());
			m_pSett->setValue("monteconvo/k_to", spinStopK->value());
			m_pSett->setValue("monteconvo/l_to", spinStopL->value());
			m_pSett->setValue("monteconvo/E_to", spinStopE->value());

			m_pSett->setValue("monteconvo/kfix", spinKfix->value());
			m_pSett->setValue("monteconvo/neutron_count", spinNeutrons->value());
			m_pSett->setValue("monteconvo/step_count", spinStepCnt->value());
		}

		QDialog::accept();
	}
}

#include "ConvoDlg.moc"
