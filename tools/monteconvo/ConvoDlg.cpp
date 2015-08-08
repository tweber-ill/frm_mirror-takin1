/*
 * monte carlo convolution tool
 * @author tweber
 * @date aug-2015
 * @license GPLv2
 */

#include "ConvoDlg.h"
#include "tlibs/string/string.h"
#include "tlibs/math/math.h"
#include "sqw.h"
#include "TASReso.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <future>

#include <QFileDialog>
#include <QMessageBox>

#include <qwt_picker_machine.h>


ConvoDlg::ConvoDlg(QWidget* pParent, QSettings* pSett)
	: QDialog(pParent), m_pSett(pSett)
{
	setupUi(this);

	m_pGrid = new QwtPlotGrid();
	QPen penGrid;
	penGrid.setColor(QColor(0x99,0x99,0x99));
	penGrid.setStyle(Qt::DashLine);
	m_pGrid->setPen(penGrid);
	m_pGrid->attach(plot);

        QPen penCurve;
        penCurve.setColor(QColor(0,0,0x99));
        penCurve.setWidth(2);
	m_pCurve = new QwtPlotCurve("S(Q,w)");
        m_pCurve->setPen(penCurve);
        m_pCurve->setStyle(QwtPlotCurve::CurveStyle::Lines);
        m_pCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
        m_pCurve->attach(plot);

        QPen penPoints;
        penPoints.setColor(QColor(0xff,0,0));
        penPoints.setWidth(4);
	m_pPoints = new QwtPlotCurve("S(Q,w)");
        m_pPoints->setPen(penPoints);
        m_pPoints->setStyle(QwtPlotCurve::CurveStyle::Dots);
        m_pPoints->setRenderHint(QwtPlotItem::RenderAntialiased, true);
        m_pPoints->attach(plot);

	plot->canvas()->setMouseTracking(1);
	m_pPicker = new QwtPlotPicker(plot->xBottom, plot->yLeft,
#if QWT_VER<6
		QwtPlotPicker::PointSelection,
#endif
		QwtPlotPicker::NoRubberBand, QwtPlotPicker::AlwaysOn, plot->canvas());

	m_pPicker->setEnabled(1);

	plot->setAxisTitle(QwtPlot::xBottom, "");
	plot->setAxisTitle(QwtPlot::yLeft, "S (a.u.)");


	QObject::connect(btnBrowseCrys, SIGNAL(clicked()), this, SLOT(browseCrysFiles()));
	QObject::connect(btnBrowseRes, SIGNAL(clicked()), this, SLOT(browseResoFiles()));
	QObject::connect(btnBrowseSqw, SIGNAL(clicked()), this, SLOT(browseSqwFiles()));

	QObject::connect(btnStart, SIGNAL(clicked()), this, SLOT(Start()));
}

ConvoDlg::~ConvoDlg()
{
}


void ConvoDlg::Start()
{
	// TODO: use thread
	//std::thread* pth = new std::thread([this]
	{
		const unsigned int iNumNeutrons = spinNeutrons->value();

		const unsigned int iNumSteps = spinStepCnt->value();
		std::vector<double> vecH = tl::linspace<double,double>(spinStartH->value(), spinStopH->value(), iNumSteps);
		std::vector<double> vecK = tl::linspace<double,double>(spinStartK->value(), spinStopK->value(), iNumSteps);
		std::vector<double> vecL = tl::linspace<double,double>(spinStartL->value(), spinStopL->value(), iNumSteps);
		std::vector<double> vecE = tl::linspace<double,double>(spinStartE->value(), spinStopE->value(), iNumSteps);

		std::string strScanVar = "";
		std::vector<double> *pVecScanX = nullptr;
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
			QMessageBox::critical(this, "Error", "No scan variable found.");
			return;
		}


		plot->setAxisTitle(QwtPlot::xBottom, strScanVar.c_str());


		TASReso reso;
		if(!reso.LoadRes(editRes->text().toStdString().c_str()))
		{
			QMessageBox::critical(this, "Error", "Could not load resolution file.");
			return;
		}

		if(!reso.LoadLattice(editCrys->text().toStdString().c_str()))
		{
			QMessageBox::critical(this, "Error", "Could not load crystal file.");
			return;
		}

		reso.SetAlgo(ResoAlgo(comboAlgo->currentIndex()));
		reso.SetKiFix(comboFixedK->currentIndex()==0);
		reso.SetKFix(spinKfix->value());


		std::shared_ptr<SqwBase> ptrSqw;

	/*	if(pcSqw)
		{
			ptrSqw.reset(new SqwKdTree(pcSqw));
		}
		else*/
		{
			ptrSqw.reset(new SqwPhonon(tl::make_vec({4.,4.,0}),
					tl::make_vec({0.,0.,1.}), tl::make_vec({1.,-1.,0.}),
					40., M_PI/2., 0.1, 0.1,
					12., M_PI/2., 0.1, 0.1,
					18., M_PI/2., 0.1, 0.1));
		}


		SqwBase *psqw = ptrSqw.get();

		if(!psqw->IsOk())
		{
			QMessageBox::critical(this, "Error", "Could not create S(q,w).");
			return;
		}


		std::ostringstream ostrOut;
		ostrOut << "#\n";
		ostrOut << "# Format: h k l E S\n";
		ostrOut << "# Neutrons: " << iNumNeutrons << "\n";
		ostrOut << "#\n";

		progress->setMaximum(iNumSteps);
		progress->setValue(0);
		tabWidget->setCurrentWidget(tabPlot);


		m_vecQ.clear();
		m_vecS.clear();

		m_vecQ.reserve(iNumSteps);
		m_vecS.reserve(iNumSteps);

		std::vector<std::future<double>> vecFuts;
		vecFuts.reserve(iNumSteps);

		for(unsigned int iStep=0; iStep<iNumSteps; ++iStep)
		{
			std::future<double> fut = std::async(std::launch::deferred | std::launch::async,
			[&reso, iStep, iNumNeutrons, &vecH, &vecK, &vecL, &vecE, this, psqw]() -> double
			{
				TASReso localreso = reso;
				std::vector<ublas::vector<double>> vecNeutrons;

				try
				{
					if(!localreso.SetHKLE(vecH[iStep], vecK[iStep], vecL[iStep], vecE[iStep]))
					{
						std::ostringstream ostrErr;
						ostrErr << "Invalid crystal position: (" <<
							vecH[iStep] << " " << vecK[iStep] << " " << vecL[iStep] << ") rlu, "
							<< vecE[iStep] << " meV.";
						throw tl::Err(ostrErr.str().c_str());
					}
				}
				catch(const std::exception& ex)
				{
					QMessageBox::critical(this, "Error", ex.what());
					return 0.;
				}

				Ellipsoid4d elli = localreso.GenerateMC(iNumNeutrons, vecNeutrons);

				double dS = 0.;
				double dhklE_mean[4] = {0., 0., 0., 0.};

				for(const ublas::vector<double>& vecHKLE : vecNeutrons)
				{
					dS += (*psqw)(vecHKLE[0], vecHKLE[1], vecHKLE[2], vecHKLE[3]);

					for(int i=0; i<4; ++i)
						dhklE_mean[i] += vecHKLE[i];
				}

				dS /= double(iNumNeutrons);
				for(int i=0; i<4; ++i)
					dhklE_mean[i] /= double(iNumNeutrons);
				return dS;
			});

			vecFuts.push_back(std::move(fut));
		}

		for(unsigned int iStep=0; iStep<iNumSteps; ++iStep)
		{
			double dS = vecFuts[iStep].get();

			ostrOut.precision(16);
			ostrOut << std::left << std::setw(20) << vecH[iStep] << " "
				<< std::left << std::setw(20) << vecK[iStep] << " "
				<< std::left << std::setw(20) << vecL[iStep] << " "
				<< std::left << std::setw(20) << vecE[iStep] << " "
				<< std::left << std::setw(20) << dS << "\n";


			m_vecQ.push_back((*pVecScanX)[iStep]);
			m_vecS.push_back(dS);


	#if QWT_VER>=6
			m_pCurve->setRawSamples(m_vecQ.data(), m_vecS.data(), m_vecQ.size());
			m_pPoints->setRawSamples(m_vecQ.data(), m_vecS.data(), m_vecQ.size());
	#else
			m_pCurve->setRawData(m_vecQ.data(), m_vecS.data(), m_vecQ.size());
			m_pPoints->setRawData(m_vecQ.data(), m_vecS.data(), m_vecQ.size());
	#endif
			plot->replot();


			textResult->clear();
			textResult->setPlainText(ostrOut.str().c_str());

			progress->setValue(iStep+1);
		}

		ostrOut << "# ---------------- EOF ----------------\n";
		textResult->clear();
		textResult->setPlainText(ostrOut.str().c_str());
	}//);
}


void ConvoDlg::browseCrysFiles()
{
	QString strDirLast = ".";
	if(m_pSett)
		strDirLast = m_pSett->value("convo/last_dir_crys", ".").toString();
	QString strFile = QFileDialog::getOpenFileName(this,
							"Open Crystal File...",
							strDirLast,
							"Takin files (*.taz *.TAZ)");
	if(strFile == "")
		return;

	editCrys->setText(strFile);

	std::string strDir = tl::get_dir(strFile.toStdString());
	if(m_pSett)
		m_pSett->setValue("convo/last_dir_crys", QString(strDir.c_str()));
}

void ConvoDlg::browseResoFiles()
{
	QString strDirLast = ".";
	if(m_pSett)
		strDirLast = m_pSett->value("convo/last_dir_reso", ".").toString();
	QString strFile = QFileDialog::getOpenFileName(this,
							"Open Resolution File...",
							strDirLast,
							"Takin files (*.taz *.TAZ)");
	if(strFile == "")
		return;

	editRes->setText(strFile);

	std::string strDir = tl::get_dir(strFile.toStdString());
	if(m_pSett)
		m_pSett->setValue("convo/last_dir_reso", QString(strDir.c_str()));
}

void ConvoDlg::browseSqwFiles()
{
	QString strDirLast = ".";
	if(m_pSett)
		strDirLast = m_pSett->value("convo/last_dir_sqw", ".").toString();
	QString strFile = QFileDialog::getOpenFileName(this,
							"Open S(q,w) File...",
							strDirLast,
							"All S(q,w) files (*.dat *.DAT *.py *.PY)");
	if(strFile == "")
		return;

	editSqw->setText(strFile);

	std::string strDir = tl::get_dir(strFile.toStdString());
	if(m_pSett)
		m_pSett->setValue("convo/last_dir_sqw", QString(strDir.c_str()));
}



#include "ConvoDlg.moc"
