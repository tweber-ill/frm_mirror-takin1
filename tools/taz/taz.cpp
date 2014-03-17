/*
 * Scattering Triangle Tool
 * @author tweber
 * @date feb-2014
 */

#include "taz.h"

#include <iostream>
#include <algorithm>
#include <vector>

#include <QtGui/QApplication>
#include <QtGui/QHBoxLayout>
#include <QtGui/QMenuBar>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

#include "helper/lattice.h"
#include "helper/spec_char.h"
#include "helper/string.h"
#include "helper/xml.h"

const std::string TazDlg::s_strTitle = "TAZ - Triple-Axis Tool";

static QString dtoqstr(double dVal, unsigned int iPrec=3)
{
	std::ostringstream ostr;
	ostr.precision(iPrec);
	ostr << dVal;
	return QString(ostr.str().c_str());
}

TazDlg::TazDlg(QWidget* pParent)
		: QDialog(pParent),
		  m_settings("tobis_stuff", "taz")
{
	const bool bSmallqVisible = 0;

	this->setupUi(this);
	this->setWindowTitle(s_strTitle.c_str());

	m_vecEdits_real =
	{
		editA, editB, editC,
		editAlpha, editBeta, editGamma
	};
	m_vecEdits_recip =
	{
		editARecip, editBRecip, editCRecip,
		editAlphaRecip, editBetaRecip, editGammaRecip
	};

	m_vecEdits_plane =
	{
		editScatX0, editScatX1, editScatX2,
		editScatY0, editScatY1, editScatY2,
		editPlaneTolerance
	};
	m_vecEdits_monoana =
	{
		editMonoD, editAnaD
	};

	m_vecSpinBoxesSample = { spinRotPhi, spinRotTheta, spinRotPsi };
	m_vecCheckBoxesSenses = { checkSenseM, checkSenseS, checkSenseA };


	m_vecEditNames_real =
	{
		"sample/a", "sample/b", "sample/c",
		"sample/alpha", "sample/beta", "sample/gamma"
	};
	m_vecEditNames_recip =
	{
		"sample/a_recip", "sample/b_recip", "sample/c_recip",
		"sample/alpha_recip", "sample/beta_recip", "sample/gamma_recip"
	};
	m_vecEditNames_plane =
	{
		"plane/x0", "plane/x1", "plane/x2",
		"plane/y0", "plane/y1", "plane/y2",
		"plane/delta"
	};
	m_vecEditNames_monoana =
	{
		"tas/mono_d", "tas/ana_d"
	};

	 m_vecSpinBoxNamesSample = {"sample/phi", "sample/theta", "sample/psi"};
	 m_vecCheckBoxNamesSenses = {"tas/sense_m", "tas/sense_s", "tas/sense_a"};


	QHBoxLayout *pLayoutRecip = new QHBoxLayout(groupRecip);
	m_pviewRecip = new ScatteringTriangleView(groupRecip);
	pLayoutRecip->addWidget(m_pviewRecip);

	QHBoxLayout *pLayoutReal = new QHBoxLayout(groupReal);
	m_pviewReal = new TasLayoutView(groupReal);
	pLayoutReal->addWidget(m_pviewReal);


	m_pviewRecip->setScene(&m_sceneRecip);
	m_pviewReal->setScene(&m_sceneReal);


	QObject::connect(&m_sceneRecip, SIGNAL(triangleChanged(const TriangleOptions&)),
					&m_sceneReal, SLOT(triangleChanged(const TriangleOptions&)));
	QObject::connect(&m_sceneReal, SIGNAL(tasChanged(const TriangleOptions&)),
					&m_sceneRecip, SLOT(tasChanged(const TriangleOptions&)));

	QObject::connect(m_pviewRecip, SIGNAL(scaleChanged(double)),
					&m_sceneRecip, SLOT(scaleChanged(double)));
	QObject::connect(m_pviewReal, SIGNAL(scaleChanged(double)),
					&m_sceneReal, SLOT(scaleChanged(double)));


	for(QLineEdit* pEdit : m_vecEdits_monoana)
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(UpdateDs()));

	for(QLineEdit* pEdit : m_vecEdits_real)
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(CalcPeaks()));

	for(QLineEdit* pEdit : m_vecEdits_plane)
	{
		if(pEdit != editPlaneTolerance)
			QObject::connect(pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(CalcPeaks()));
	}
	QObject::connect(editPlaneTolerance, SIGNAL(textEdited(const QString&)), this, SLOT(ChangedTolerance()));

	for(QDoubleSpinBox* pSpin : m_vecSpinBoxesSample)
		QObject::connect(pSpin, SIGNAL(valueChanged(double)), this, SLOT(CalcPeaks()));

	for(QLineEdit* pEdit : m_vecEdits_recip)
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(CalcPeaksRecip()));

	QObject::connect(checkSenseM, SIGNAL(stateChanged(int)), this, SLOT(UpdateMonoSense()));
	QObject::connect(checkSenseS, SIGNAL(stateChanged(int)), this, SLOT(UpdateSampleSense()));
	QObject::connect(checkSenseA, SIGNAL(stateChanged(int)), this, SLOT(UpdateAnaSense()));

	QObject::connect(btn3D, SIGNAL(clicked()), this, SLOT(Show3D()));


	// --------------------------------------------------------------------------------
	// menu
	QMenu *pMenuFile = new QMenu(this);
	pMenuFile->setTitle("File");

	/*QAction *pNew = new QAction(this);
	pNew->setText("New");
	pNew->setIcon(QIcon::fromTheme("document-new"));
	pMenuFile->addAction(pNew);

	pMenuFile->addSeparator();*/

	QAction *pLoad = new QAction(this);
	pLoad->setText("Load...");
	pLoad->setIcon(QIcon::fromTheme("document-open"));
	pMenuFile->addAction(pLoad);

	QAction *pSave = new QAction(this);
	pSave->setText("Save");
	pSave->setIcon(QIcon::fromTheme("document-save"));
	pMenuFile->addAction(pSave);

	QAction *pSaveAs = new QAction(this);
	pSaveAs->setText("Save as...");
	pSaveAs->setIcon(QIcon::fromTheme("document-save-as"));
	pMenuFile->addAction(pSaveAs);

	pMenuFile->addSeparator();

    QAction *pExit = new QAction(this);
    pExit->setText("Exit");
    pExit->setIcon(QIcon::fromTheme("application-exit"));
    pMenuFile->addAction(pExit);


	QMenu *pMenuView = new QMenu(this);
	pMenuView->setTitle("View");

    m_pSmallq = new QAction(this);
    m_pSmallq->setText("Enable Reduced Scattering Vector q");
    m_pSmallq->setCheckable(1);
    m_pSmallq->setChecked(bSmallqVisible);
    pMenuView->addAction(m_pSmallq);


	QMenu *pMenuHelp = new QMenu(this);
	pMenuHelp->setTitle("Help");

	QAction *pAboutQt = new QAction(this);
	pAboutQt->setText("About Qt...");
	//pAboutQt->setIcon(QIcon::fromTheme("help-about"));
	pMenuHelp->addAction(pAboutQt);

	pMenuHelp->addSeparator();

	QAction *pAbout = new QAction(this);
	pAbout->setText("About...");
	pAbout->setIcon(QIcon::fromTheme("help-about"));
	pMenuHelp->addAction(pAbout);


	QMenuBar *pMenuBar = new QMenuBar(this);
	pMenuBar->addMenu(pMenuFile);
	pMenuBar->addMenu(pMenuView);
	pMenuBar->addMenu(pMenuHelp);
	this->layout()->setMenuBar(pMenuBar);

	QObject::connect(pLoad, SIGNAL(triggered()), this, SLOT(Load()));
	QObject::connect(pSave, SIGNAL(triggered()), this, SLOT(Save()));
	QObject::connect(pSaveAs, SIGNAL(triggered()), this, SLOT(SaveAs()));
	QObject::connect(m_pSmallq, SIGNAL(toggled(bool)), this, SLOT(EnableSmallq(bool)));
	QObject::connect(pExit, SIGNAL(triggered()), this, SLOT(close()));
	QObject::connect(pAbout, SIGNAL(triggered()), this, SLOT(ShowAbout()));
	QObject::connect(pAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	// --------------------------------------------------------------------------------



	UpdateDs();
	CalcPeaks();

	m_sceneRecip.GetTriangle()->SetqVisible(bSmallqVisible);
	m_sceneRecip.emitUpdate();
}

TazDlg::~TazDlg()
{
	if(m_pRecip3d)
	{
		delete m_pRecip3d;
		m_pRecip3d = 0;
	}

	if(m_pviewRecip)
	{
		delete m_pviewRecip;
		m_pviewRecip = 0;
	}
}


void TazDlg::UpdateDs()
{
	double dMonoD = editMonoD->text().toDouble();
	double dAnaD = editAnaD->text().toDouble();

	m_sceneRecip.SetDs(dMonoD, dAnaD);
}

std::ostream& operator<<(std::ostream& ostr, const Lattice& lat)
{
	ostr << "a = " << lat.GetA();
	ostr << ", b = " << lat.GetB();
	ostr << ", c = " << lat.GetC();
	ostr << ", alpha = " << lat.GetAlpha();
	ostr << ", beta = " << lat.GetBeta();
	ostr << ", gamma = " << lat.GetGamma();
	return ostr;
}

void TazDlg::ChangedTolerance()
{
	if(!m_sceneRecip.GetTriangle())
		return;

	double dTol = editPlaneTolerance->text().toDouble();
	m_sceneRecip.GetTriangle()->SetPlaneDistTolerance(dTol);
	if(m_pRecip3d)
		m_pRecip3d->SetPlaneDistTolerance(dTol);

	CalcPeaks();
}

void TazDlg::CalcPeaksRecip()
{
	double a = editARecip->text().toDouble();
	double b = editBRecip->text().toDouble();
	double c = editCRecip->text().toDouble();

	double alpha = editAlphaRecip->text().toDouble()/180.*M_PI;
	double beta = editBetaRecip->text().toDouble()/180.*M_PI;
	double gamma = editGammaRecip->text().toDouble()/180.*M_PI;

	Lattice lattice(a,b,c, alpha,beta,gamma);
	Lattice recip = lattice.GetRecip();

	editA->setText(dtoqstr(recip.GetA()));
	editB->setText(dtoqstr(recip.GetB()));
	editC->setText(dtoqstr(recip.GetC()));
	editAlpha->setText(dtoqstr(recip.GetAlpha()/M_PI*180.));
	editBeta->setText(dtoqstr(recip.GetBeta()/M_PI*180.));
	editGamma->setText(dtoqstr(recip.GetGamma()/M_PI*180.));

	m_bUpdateRecipEdits = 0;
	CalcPeaks();
	m_bUpdateRecipEdits = 1;
}

void TazDlg::CalcPeaks()
{
	if(!m_sceneRecip.GetTriangle())
		return;

	double a = editA->text().toDouble();
	double b = editB->text().toDouble();
	double c = editC->text().toDouble();

	double alpha = editAlpha->text().toDouble()/180.*M_PI;
	double beta = editBeta->text().toDouble()/180.*M_PI;
	double gamma = editGamma->text().toDouble()/180.*M_PI;

	Lattice lattice(a,b,c, alpha,beta,gamma);
	Lattice recip_unrot = lattice.GetRecip();

	double dPhi = spinRotPhi->value() / 180. * M_PI;
	double dTheta = spinRotTheta->value() / 180. * M_PI;
	double dPsi = spinRotPsi->value() / 180. * M_PI;
	lattice.RotateEuler(dPhi, dTheta, dPsi);

	Lattice recip = lattice.GetRecip();


	double dX0 = editScatX0->text().toDouble();
	double dX1 = editScatX1->text().toDouble();
	double dX2 = editScatX2->text().toDouble();
	ublas::vector<double> vecPlaneX = dX0*recip_unrot.GetVec(0) +
									dX1*recip_unrot.GetVec(1) +
									dX2*recip_unrot.GetVec(2);

	double dY0 = editScatY0->text().toDouble();
	double dY1 = editScatY1->text().toDouble();
	double dY2 = editScatY2->text().toDouble();
	ublas::vector<double> vecPlaneY = dY0*recip_unrot.GetVec(0) +
									dY1*recip_unrot.GetVec(1) +
									dY2*recip_unrot.GetVec(2);


	ublas::vector<double> vecX0 = ublas::zero_vector<double>(3);
	Plane<double> plane(vecX0, vecPlaneX, vecPlaneY);


	if(m_bUpdateRecipEdits)
	{
		editARecip->setText(dtoqstr(recip.GetA()));
		editBRecip->setText(dtoqstr(recip.GetB()));
		editCRecip->setText(dtoqstr(recip.GetC()));
		editAlphaRecip->setText(dtoqstr(recip.GetAlpha()/M_PI*180.));
		editBetaRecip->setText(dtoqstr(recip.GetBeta()/M_PI*180.));
		editGammaRecip->setText(dtoqstr(recip.GetGamma()/M_PI*180.));
	}

	const std::wstring& strAA = ::get_spec_char_utf16("AA");
	const std::wstring& strMinus = ::get_spec_char_utf16("sup-");
	const std::wstring& strThree = ::get_spec_char_utf16("sup3");

	double dVol = lattice.GetVol();
	std::wostringstream ostrSample;
	ostrSample.precision(4);
	ostrSample << "Sample - ";
	ostrSample << "Unit Cell Volume: ";
	ostrSample << "Real: " << dVol << " " << strAA << strThree;
	ostrSample << ", Reciprocal: " << (1./dVol) << " " << strAA << strMinus << strThree;
	groupSample->setTitle(QString::fromWCharArray(ostrSample.str().c_str()));

	m_sceneRecip.GetTriangle()->CalcPeaks(lattice, recip, recip_unrot, plane);
	m_sceneRecip.SnapToNearestPeak(m_sceneRecip.GetTriangle()->GetNodeGq());
	m_sceneRecip.emitUpdate();

	if(m_pRecip3d)
		m_pRecip3d->CalcPeaks(lattice, recip, recip_unrot, plane);
}

void TazDlg::UpdateSampleSense()
{
	m_sceneRecip.SetSampleSense(checkSenseS->isChecked());
}

void TazDlg::UpdateMonoSense()
{
	m_sceneRecip.SetMonoSense(checkSenseM->isChecked());
}

void TazDlg::UpdateAnaSense()
{
	m_sceneRecip.SetAnaSense(checkSenseA->isChecked());
}

void TazDlg::Show3D()
{
	if(!m_pRecip3d)
	{
		m_pRecip3d = new Recip3DDlg(this);

		double dTol = editPlaneTolerance->text().toDouble();
		m_pRecip3d->SetPlaneDistTolerance(dTol);
	}

	if(!m_pRecip3d->isVisible())
		m_pRecip3d->show();
	m_pRecip3d->activateWindow();

	CalcPeaks();
}

void TazDlg::EnableSmallq(bool bEnable)
{
	m_sceneRecip.GetTriangle()->SetqVisible(bEnable);
}

bool TazDlg::Load(const char* pcFile)
{
	m_strCurFile = pcFile;

	const std::string strXmlRoot("taz/");

	std::string strFile1 = pcFile;
	std::string strDir = get_dir(strFile1).c_str();


	Xml xml;
	if(!xml.Load(strFile1.c_str()))
	{
		QMessageBox::critical(this, "Error", "Could not load configuration file.");
		return false;
	}

	m_settings.setValue("main/last_dir", QString(strDir.c_str()));


	bool bOk = 0;

	// edit boxes
	std::vector<std::vector<QLineEdit*>*> vecEdits
			= {&m_vecEdits_real, &m_vecEdits_recip,
				&m_vecEdits_plane, &m_vecEdits_monoana};
	std::vector<std::vector<std::string>*> vecEditNames
			= {&m_vecEditNames_real, &m_vecEditNames_recip,
				&m_vecEditNames_plane, &m_vecEditNames_monoana};
	unsigned int iIdxEdit = 0;
	for(const std::vector<QLineEdit*>* pVec : vecEdits)
	{
		const std::vector<std::string>* pvecName = vecEditNames[iIdxEdit];

		for(unsigned int iEditBox=0; iEditBox<pVec->size(); ++iEditBox)
		{
			std::string str = xml.QueryString((strXmlRoot+(*pvecName)[iEditBox]).c_str(), "0", &bOk);
			trim(str);
			if(bOk)
				(*pVec)[iEditBox]->setText(str.c_str());
		}

		++iIdxEdit;
	}


	// spin boxes
	for(unsigned int iSpinBox=0; iSpinBox<m_vecSpinBoxesSample.size(); ++iSpinBox)
	{
		double dVal = xml.Query<double>((strXmlRoot+m_vecSpinBoxNamesSample[iSpinBox]).c_str(), 0., &bOk);
		if(bOk)
			m_vecSpinBoxesSample[iSpinBox]->setValue(dVal);
	}


	// check boxes
	for(unsigned int iCheckBox=0; iCheckBox<m_vecCheckBoxesSenses.size(); ++iCheckBox)
	{
		int iVal = xml.Query<int>((strXmlRoot+m_vecCheckBoxNamesSenses[iCheckBox]).c_str(), 0, &bOk);
		if(bOk)
			m_vecCheckBoxesSenses[iCheckBox]->setChecked(iVal != 0);
	}


	// TAS Layout
	double dRealScale = xml.Query<double>((strXmlRoot + "real/pixels_per_cm").c_str(), 0., &bOk);
	if(bOk)
		m_sceneReal.GetTasLayout()->SetScaleFactor(dRealScale);

	unsigned int iNodeReal = 0;
	for(TasLayoutNode *pNode : m_sceneReal.GetTasLayout()->GetNodes())
	{
		std::string strNode = m_sceneReal.GetTasLayout()->GetNodeNames()[iNodeReal];

		bool bOkX=0, bOkY=0;
		double dValX = xml.Query<double>((strXmlRoot + "real/" + strNode + "_x").c_str(), 0., &bOkX);
		double dValY = xml.Query<double>((strXmlRoot + "real/" + strNode + "_y").c_str(), 0., &bOkY);

		pNode->setPos(dValX, dValY);
		++iNodeReal;
	}


	// scattering triangle
	double dRecipScale = xml.Query<double>((strXmlRoot + "recip/pixels_per_A-1").c_str(), 0., &bOk);
	if(bOk)
		m_sceneRecip.GetTriangle()->SetScaleFactor(dRecipScale);

	unsigned int iNodeRecip = 0;
	for(ScatteringTriangleNode *pNode : m_sceneRecip.GetTriangle()->GetNodes())
	{
		std::string strNode = m_sceneRecip.GetTriangle()->GetNodeNames()[iNodeRecip];

		bool bOkX=0, bOkY=0;
		double dValX = xml.Query<double>((strXmlRoot + "recip/" + strNode + "_x").c_str(), 0., &bOkX);
		double dValY = xml.Query<double>((strXmlRoot + "recip/" + strNode + "_y").c_str(), 0., &bOkY);

		pNode->setPos(dValX, dValY);
		++iNodeRecip;
	}


	int bSmallqEnabled = xml.Query<int>((strXmlRoot + "recip/enable_q").c_str(), 0, &bOk);
	if(bOk)
		m_pSmallq->setChecked(bSmallqEnabled!=0);


	m_strCurFile = strFile1;
	setWindowTitle((s_strTitle + " - " + m_strCurFile).c_str());

	CalcPeaks();
	return true;
}

bool TazDlg::Load()
{
	QString strDirLast = m_settings.value("main/last_dir", ".").toString();
	QString strFile = QFileDialog::getOpenFileName(this,
							"Open TAS configuration...",
							strDirLast,
							"TAZ files (*.taz *.TAZ)");
	if(strFile == "")
		return false;

	return Load(strFile.toStdString().c_str());
}

bool TazDlg::Save()
{
	if(m_strCurFile == "")
		return SaveAs();

	const std::string strXmlRoot("taz/");
	typedef std::map<std::string, std::string> tmap;
	tmap mapConf;


	// edit boxes
	std::vector<const std::vector<QLineEdit*>*> vecEdits
			= {&m_vecEdits_real, &m_vecEdits_recip,
				&m_vecEdits_plane, &m_vecEdits_monoana};
	std::vector<const std::vector<std::string>*> vecEditNames
			= {&m_vecEditNames_real, &m_vecEditNames_recip,
				&m_vecEditNames_plane, &m_vecEditNames_monoana};
	unsigned int iIdxEdit = 0;
	for(const std::vector<QLineEdit*>* pVec : vecEdits)
	{
		const std::vector<std::string>* pvecName = vecEditNames[iIdxEdit];

		for(unsigned int iEditBox=0; iEditBox<pVec->size(); ++iEditBox)
			mapConf[strXmlRoot+(*pvecName)[iEditBox]]
			        = (*pVec)[iEditBox]->text().toStdString();

		++iIdxEdit;
	}


	// spin boxes
	for(unsigned int iSpinBox=0; iSpinBox<m_vecSpinBoxesSample.size(); ++iSpinBox)
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << m_vecSpinBoxesSample[iSpinBox]->value();

		mapConf[strXmlRoot + m_vecSpinBoxNamesSample[iSpinBox]] = ostrVal.str();
	}


	// check boxes
	for(unsigned int iCheckBox=0; iCheckBox<m_vecCheckBoxesSenses.size(); ++iCheckBox)
		mapConf[strXmlRoot+m_vecCheckBoxNamesSenses[iCheckBox]]
		        		= (m_vecCheckBoxesSenses[iCheckBox]->isChecked() ? "1" : "0");


	// TAS layout
	unsigned int iNodeReal = 0;
	for(const TasLayoutNode *pNode : m_sceneReal.GetTasLayout()->GetNodes())
	{
		std::string strNode = m_sceneReal.GetTasLayout()->GetNodeNames()[iNodeReal];
		std::string strValX = var_to_str(pNode->pos().x());
		std::string strValY = var_to_str(pNode->pos().y());

		mapConf[strXmlRoot + "real/" + strNode + "_x"] = strValX;
		mapConf[strXmlRoot + "real/" + strNode + "_y"] = strValY;

		++iNodeReal;
	}
	double dRealScale = m_sceneReal.GetTasLayout()->GetScaleFactor();
	mapConf[strXmlRoot + "real/pixels_per_cm"] = var_to_str(dRealScale);


	// scattering triangle
	unsigned int iNodeRecip = 0;
	for(const ScatteringTriangleNode *pNode : m_sceneRecip.GetTriangle()->GetNodes())
	{
		std::string strNode = m_sceneRecip.GetTriangle()->GetNodeNames()[iNodeRecip];
		std::string strValX = var_to_str(pNode->pos().x());
		std::string strValY = var_to_str(pNode->pos().y());

		mapConf[strXmlRoot + "recip/" + strNode + "_x"] = strValX;
		mapConf[strXmlRoot + "recip/" + strNode + "_y"] = strValY;

		++iNodeRecip;
	}
	double dRecipScale = m_sceneRecip.GetTriangle()->GetScaleFactor();
	mapConf[strXmlRoot + "recip/pixels_per_A-1"] = var_to_str(dRecipScale);


	bool bSmallqEnabled = m_pSmallq->isChecked();
	mapConf[strXmlRoot + "recip/enable_q"] = (bSmallqEnabled ? "1" : "0");


	if(!Xml::SaveMap(m_strCurFile.c_str(), mapConf))
	{
		QMessageBox::critical(this, "Error", "Could not save configuration file.");
		return false;
	}

	return true;
}

bool TazDlg::SaveAs()
{
	QString strDirLast = m_settings.value("main/last_dir", ".").toString();
	QString strFile = QFileDialog::getSaveFileName(this,
								"Save TAS configuration",
								strDirLast,
								"TAZ files (*.taz *.TAZ)");

	if(strFile != "")
	{
		std::string strFile1 = strFile.toStdString();
		std::string strDir = get_dir(strFile1).c_str();

		m_strCurFile = strFile1;
		setWindowTitle((s_strTitle + " - " + m_strCurFile).c_str());
		bool bOk = Save();

		if(bOk)
			m_settings.setValue("main/last_dir", QString(strDir.c_str()));

		return bOk;
	}

	return false;
}



#include <boost/version.hpp>

void TazDlg::ShowAbout()
{
	QString strAbout;
	strAbout += "TAZ version 0.5\n";
	strAbout += "Written by Tobias Weber, 2014";
	strAbout += "\n\n";

	strAbout += "Built with CC version ";
	strAbout += QString(__VERSION__);
	strAbout += "\n";

	strAbout += "Build date: ";
	strAbout += QString(__DATE__);
	strAbout += ", ";
	strAbout += QString(__TIME__);
	strAbout += "\n\n";

	strAbout += "Uses Qt version ";
	strAbout += QString(QT_VERSION_STR);
	strAbout += "\n\t " + QString::fromWCharArray(get_spec_char_utf16("rightarrow").c_str())
						+ " http://qt-project.org\n";
	strAbout += "Uses Boost version ";
	std::string strBoost = BOOST_LIB_VERSION;
	find_all_and_replace<std::string>(strBoost, "_", ".");
	strAbout += strBoost.c_str();
	strAbout += "\n\t " + QString::fromWCharArray(get_spec_char_utf16("rightarrow").c_str())
						+ " http://www.boost.org\n";

	QMessageBox::information(this, "About TAZ", strAbout);
}

#include "taz.moc"
