/*
 * TAS tool
 * @author tweber
 * @date feb-2014
 */

#include "taz.h"

#include <iostream>
#include <algorithm>
#include <vector>
#include <boost/algorithm/string.hpp>

#include <QtGui/QApplication>
#include <QtGui/QHBoxLayout>
#include <QtGui/QMenuBar>
#include <QtGui/QToolBar>
#include <QtGui/QStatusBar>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

#include "helper/lattice.h"
#include "helper/spec_char.h"
#include "helper/string.h"
#include "helper/xml.h"
#include "helper/log.h"

#define DEFAULT_MSG_TIMEOUT 4000
const std::string TazDlg::s_strTitle = "Takin";

static QString dtoqstr(double dVal, unsigned int iPrec=3)
{
	std::ostringstream ostr;
	ostr.precision(iPrec);
	ostr << dVal;
	return QString(ostr.str().c_str());
}

TazDlg::TazDlg(QWidget* pParent)
		: QMainWindow(pParent),
		  m_settings("tobis_stuff", "takin"),
		  m_pmapSpaceGroups(get_space_groups()),
		  m_dlgRecipParam(this, &m_settings),
		  m_dlgRealParam(this, &m_settings),
		  m_pStatusMsg(new QLabel(this)),
		  m_pCoordStatusMsg(new QLabel(this)),
		  m_pGotoDlg(new GotoDlg(this, &m_settings))
{
	//log_debug("In ", __func__, ".");

	const bool bSmallqVisible = 1;
	const bool bBZVisible = 0;

	this->setupUi(this);
	this->setWindowTitle(s_strTitle.c_str());

	if(m_settings.contains("main/geo"))
	{
		QByteArray geo = m_settings.value("main/geo").toByteArray();
		restoreGeometry(geo);
	}
/*
	if(m_settings.contains("main/width") && m_settings.contains("main/height"))
	{
		int iW = m_settings.value("main/width").toInt();
		int iH = m_settings.value("main/height").toInt();
		resize(iW, iH);
	}*/

	m_pStatusMsg->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	m_pCoordStatusMsg->setFrameStyle(QFrame::Panel | QFrame::Sunken);

	QStatusBar *pStatusBar = new QStatusBar(this);
	pStatusBar->addWidget(m_pStatusMsg, 1);
	pStatusBar->addPermanentWidget(m_pCoordStatusMsg, 0);
	m_pCoordStatusMsg->setMinimumWidth(200);
	m_pCoordStatusMsg->setAlignment(Qt::AlignCenter);
	this->setStatusBar(pStatusBar);

	// --------------------------------------------------------------------------------

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
	};
	m_vecEdits_monoana =
	{
		editMonoD, editAnaD
	};

	//m_vecSpinBoxesSample = { spinRotPhi, spinRotTheta, spinRotPsi };
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
	QObject::connect(&m_sceneRecip, SIGNAL(paramsChanged(const RecipParams&)),
					&m_sceneReal, SLOT(recipParamsChanged(const RecipParams&)));

	QObject::connect(m_pviewRecip, SIGNAL(scaleChanged(double)),
					&m_sceneRecip, SLOT(scaleChanged(double)));
	QObject::connect(m_pviewReal, SIGNAL(scaleChanged(double)),
					&m_sceneReal, SLOT(scaleChanged(double)));

	QObject::connect(&m_sceneRecip, SIGNAL(paramsChanged(const RecipParams&)),
					&m_dlgRecipParam, SLOT(paramsChanged(const RecipParams&)));
	QObject::connect(&m_sceneReal, SIGNAL(paramsChanged(const RealParams&)),
					&m_dlgRealParam, SLOT(paramsChanged(const RealParams&)));

	QObject::connect(&m_sceneRecip, SIGNAL(coordsChanged(double, double, double)),
					this, SLOT(RecipCoordsChanged(double, double, double)));

	QObject::connect(&m_sceneRecip, SIGNAL(spurionInfo(const ElasticSpurion&, const std::vector<InelasticSpurion>&, const std::vector<InelasticSpurion>&)),
					this, SLOT(spurionInfo(const ElasticSpurion&, const std::vector<InelasticSpurion>&, const std::vector<InelasticSpurion>&)));

	QObject::connect(m_pGotoDlg, SIGNAL(vars_changed(const CrystalOptions&, const TriangleOptions&)),
					this, SLOT(VarsChanged(const CrystalOptions&, const TriangleOptions&)));
	QObject::connect(&m_sceneRecip, SIGNAL(paramsChanged(const RecipParams&)),
					m_pGotoDlg, SLOT(RecipParamsChanged(const RecipParams&)));


	for(QLineEdit* pEdit : m_vecEdits_monoana)
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(UpdateDs()));

	for(QLineEdit* pEdit : m_vecEdits_real)
	{
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(CheckCrystalType()));
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(CalcPeaks()));
	}

	for(QLineEdit* pEdit : m_vecEdits_plane)
	{
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(CalcPeaks()));
	}

	//for(QDoubleSpinBox* pSpin : m_vecSpinBoxesSample)
	//	QObject::connect(pSpin, SIGNAL(valueChanged(double)), this, SLOT(CalcPeaks()));

	for(QLineEdit* pEdit : m_vecEdits_recip)
	{
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(CheckCrystalType()));
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(CalcPeaksRecip()));
	}

	QObject::connect(checkSenseM, SIGNAL(stateChanged(int)), this, SLOT(UpdateMonoSense()));
	QObject::connect(checkSenseS, SIGNAL(stateChanged(int)), this, SLOT(UpdateSampleSense()));
	QObject::connect(checkSenseA, SIGNAL(stateChanged(int)), this, SLOT(UpdateAnaSense()));

	QObject::connect(editSpaceGroupsFilter, SIGNAL(textChanged(const QString&)), this, SLOT(RepopulateSpaceGroups()));
	QObject::connect(comboSpaceGroups, SIGNAL(currentIndexChanged(int)), this, SLOT(SetCrystalType()));
	QObject::connect(comboSpaceGroups, SIGNAL(currentIndexChanged(int)), this, SLOT(CalcPeaks()));



	// --------------------------------------------------------------------------------
	// file menu
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


	// --------------------------------------------------------------------------------
	// recip menu
	m_pMenuViewRecip = new QMenu(this);
	m_pMenuViewRecip->setTitle("Reciprocal Space");

	m_pGoto = new QAction(this);
	m_pGoto->setText("Go to Position...");
	m_pGoto->setIcon(QIcon("res/goto.svg"));
	m_pMenuViewRecip->addAction(m_pGoto);

	QAction *pRecipParams = new QAction(this);
	pRecipParams->setText("Parameters...");
	m_pMenuViewRecip->addAction(pRecipParams);
	m_pMenuViewRecip->addSeparator();

	m_pSmallq = new QAction(this);
	m_pSmallq->setText("Show Reduced Scattering Vector q");
	m_pSmallq->setIcon(QIcon("res/q.svg"));
	m_pSmallq->setCheckable(1);
	m_pSmallq->setChecked(bSmallqVisible);
	m_pMenuViewRecip->addAction(m_pSmallq);

	m_pSnapSmallq = new QAction(this);
	m_pSnapSmallq->setText("Snap G to Bragg Peak");
	m_pSnapSmallq->setCheckable(1);
	m_pSnapSmallq->setChecked(m_sceneRecip.getSnapq());
	m_pMenuViewRecip->addAction(m_pSnapSmallq);

	m_pBZ = new QAction(this);
	m_pBZ->setText("Show First Brillouin Zone");
	m_pBZ->setIcon(QIcon("res/brillouin.svg"));
	m_pBZ->setCheckable(1);
	m_pBZ->setChecked(bBZVisible);
	m_pMenuViewRecip->addAction(m_pBZ);

	m_pMenuViewRecip->addSeparator();

	QAction *pView3D = new QAction(this);
	pView3D->setText("3D View...");
	pView3D->setIcon(QIcon::fromTheme("applications-graphics"));
	m_pMenuViewRecip->addAction(pView3D);

	m_pMenuViewRecip->addSeparator();

	QAction *pRecipExport = new QAction(this);
	pRecipExport->setText("Export Image...");
	pRecipExport->setIcon(QIcon::fromTheme("image-x-generic"));
	m_pMenuViewRecip->addAction(pRecipExport);



	// --------------------------------------------------------------------------------
	// real menu
	m_pMenuViewReal = new QMenu(this);
	m_pMenuViewReal->setTitle("Real Space");

	m_pMenuViewReal->addAction(m_pGoto);

	QAction *pRealParams = new QAction(this);
	pRealParams->setText("Parameters...");
	m_pMenuViewReal->addAction(pRealParams);

	m_pMenuViewReal->addSeparator();

	m_pShowRealQDir = new QAction(this);
	m_pShowRealQDir->setText("Show Q Direction");
	m_pShowRealQDir->setCheckable(1);
	m_pShowRealQDir->setChecked(m_sceneReal.GetTasLayout()->GetRealQVisible());
	m_pMenuViewReal->addAction(m_pShowRealQDir);

	m_pMenuViewReal->addSeparator();

	QAction *pRealExport = new QAction(this);
	pRealExport->setText("Export Image...");
	pRealExport->setIcon(QIcon::fromTheme("image-x-generic"));
	m_pMenuViewReal->addAction(pRealExport);



	// --------------------------------------------------------------------------------
	// resolution menu
	QMenu *pMenuReso = new QMenu(this);
	pMenuReso->setTitle("Resolution");

	QAction *pResoParams = new QAction(this);
	pResoParams->setText("Parameters...");
	pResoParams->setIcon(QIcon::fromTheme("accessories-calculator"));
	pMenuReso->addAction(pResoParams);

	pMenuReso->addSeparator();

	QAction *pResoEllipses = new QAction(this);
	pResoEllipses->setText("Ellipses...");
	pResoEllipses->setIcon(QIcon("res/ellipses.svg"));
	pMenuReso->addAction(pResoEllipses);

	QAction *pResoEllipses3D = new QAction(this);
	pResoEllipses3D->setText("3D Ellipsoids...");
	pMenuReso->addAction(pResoEllipses3D);


	// --------------------------------------------------------------------------------
	// calc menu
	QMenu *pMenuCalc = new QMenu(this);
	pMenuCalc->setTitle("Calculation");

	QAction *pNeutronProps = new QAction(this);
	pNeutronProps->setText("Neutron Properties...");
	pMenuCalc->addAction(pNeutronProps);

	QAction *pSpuri = new QAction(this);
	pSpuri->setText("Spurious Scattering...");
	pMenuCalc->addAction(pSpuri);



	// --------------------------------------------------------------------------------
	// network menu
	QMenu *pMenuNet = new QMenu(this);
	pMenuNet->setTitle("Network");

	QAction *pConn = new QAction(this);
	pConn->setText("Connect to NICOS...");
	pConn->setIcon(QIcon::fromTheme("network-wireless"));
	pMenuNet->addAction(pConn);

	QAction *pDisconn = new QAction(this);
	pDisconn->setText("Disconnect");
	pDisconn->setIcon(QIcon::fromTheme("network-offline"));
	pMenuNet->addAction(pDisconn);
	
	QAction *pNetCache = new QAction(this);
	pNetCache->setText("Show Network Cache...");
	pMenuNet->addSeparator();
	pMenuNet->addAction(pNetCache);

	QAction *pNetRefresh = new QAction(this);
	pNetRefresh->setText("Refresh");
	pNetRefresh->setIcon(QIcon::fromTheme("view-refresh"));
	pMenuNet->addSeparator();
	pMenuNet->addAction(pNetRefresh);



	// --------------------------------------------------------------------------------
	// help menu
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



	// --------------------------------------------------------------------------------
	QMenuBar *pMenuBar = new QMenuBar(this);
	pMenuBar->addMenu(pMenuFile);
	pMenuBar->addMenu(m_pMenuViewRecip);
	pMenuBar->addMenu(m_pMenuViewReal);
	pMenuBar->addMenu(pMenuReso);
	pMenuBar->addMenu(pMenuCalc);
	pMenuBar->addMenu(pMenuNet);
	pMenuBar->addMenu(pMenuHelp);



	QObject::connect(pLoad, SIGNAL(triggered()), this, SLOT(Load()));
	QObject::connect(pSave, SIGNAL(triggered()), this, SLOT(Save()));
	QObject::connect(pSaveAs, SIGNAL(triggered()), this, SLOT(SaveAs()));
	QObject::connect(pExit, SIGNAL(triggered()), this, SLOT(close()));

	QObject::connect(m_pSmallq, SIGNAL(toggled(bool)), this, SLOT(EnableSmallq(bool)));
	QObject::connect(m_pBZ, SIGNAL(toggled(bool)), this, SLOT(EnableBZ(bool)));
	QObject::connect(m_pShowRealQDir, SIGNAL(toggled(bool)), this, SLOT(EnableRealQDir(bool)));

	QObject::connect(m_pSnapSmallq, SIGNAL(toggled(bool)), &m_sceneRecip, SLOT(setSnapq(bool)));

	QObject::connect(pRecipParams, SIGNAL(triggered()), this, SLOT(ShowRecipParams()));
	QObject::connect(pRealParams, SIGNAL(triggered()), this, SLOT(ShowRealParams()));
	QObject::connect(pView3D, SIGNAL(triggered()), this, SLOT(Show3D()));

	QObject::connect(pRecipExport, SIGNAL(triggered()), this, SLOT(ExportRecip()));
	QObject::connect(pRealExport, SIGNAL(triggered()), this, SLOT(ExportReal()));

	QObject::connect(pResoParams, SIGNAL(triggered()), this, SLOT(ShowResoParams()));
	QObject::connect(pResoEllipses, SIGNAL(triggered()), this, SLOT(ShowResoEllipses()));
	QObject::connect(pResoEllipses3D, SIGNAL(triggered()), this, SLOT(ShowResoEllipses3D()));

	QObject::connect(pNeutronProps, SIGNAL(triggered()), this, SLOT(ShowNeutronDlg()));
	QObject::connect(m_pGoto, SIGNAL(triggered()), this, SLOT(ShowGotoDlg()));
	QObject::connect(pSpuri, SIGNAL(triggered()), this, SLOT(ShowSpurions()));

	QObject::connect(pConn, SIGNAL(triggered()), this, SLOT(ShowConnectDlg()));
	QObject::connect(pDisconn, SIGNAL(triggered()), this, SLOT(Disconnect()));
	QObject::connect(pNetRefresh, SIGNAL(triggered()), this, SLOT(NetRefresh()));
	QObject::connect(pNetCache, SIGNAL(triggered()), this, SLOT(ShowNetCache()));

	QObject::connect(pAbout, SIGNAL(triggered()), this, SLOT(ShowAbout()));
	QObject::connect(pAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));


	setMenuBar(pMenuBar);
	// --------------------------------------------------------------------------------


	// --------------------------------------------------------------------------------
	// context menus
	m_pviewRecip->setContextMenuPolicy(Qt::CustomContextMenu);
	m_pviewReal->setContextMenuPolicy(Qt::CustomContextMenu);

	QObject::connect(m_pviewRecip, SIGNAL(customContextMenuRequested(const QPoint&)),
					this, SLOT(RecipContextMenu(const QPoint&)));
	QObject::connect(m_pviewReal, SIGNAL(customContextMenuRequested(const QPoint&)),
					this, SLOT(RealContextMenu(const QPoint&)));


	// --------------------------------------------------------------------------------
	// tool bars
	QToolBar *pFileTools = new QToolBar(this);
	pFileTools->setWindowTitle("File");
	pFileTools->addAction(pLoad);
	pFileTools->addAction(pSave);
	pFileTools->addAction(pSaveAs);
	addToolBar(pFileTools);

	QToolBar *pRecipTools = new QToolBar(this);
	pRecipTools->setWindowTitle("Reciprocal Space");
	pRecipTools->addAction(m_pGoto);
	pRecipTools->addAction(m_pSmallq);
	pRecipTools->addAction(m_pBZ);
	addToolBar(pRecipTools);

	QToolBar *pResoTools = new QToolBar(this);
	pResoTools->setWindowTitle("Resolution");
	pResoTools->addAction(pResoParams);
	pResoTools->addAction(pResoEllipses);
	addToolBar(pResoTools);

	QToolBar *pNetTools = new QToolBar(this);
	pNetTools->setWindowTitle("Network");
	pNetTools->addAction(pConn);
	pNetTools->addAction(pDisconn);
	pNetTools->addAction(pNetRefresh);
	addToolBar(pNetTools);

	// --------------------------------------------------------------------------------


	RepopulateSpaceGroups();

	m_sceneRecip.GetTriangle()->SetMaxPeaks(s_iMaxPeaks);
	m_sceneRecip.GetTriangle()->SetPlaneDistTolerance(s_dPlaneDistTolerance);
	if(m_pRecip3d)
		m_pRecip3d->SetPlaneDistTolerance(s_dPlaneDistTolerance);	

	UpdateDs();
	CalcPeaks();

	m_sceneRecip.GetTriangle()->SetqVisible(bSmallqVisible);
	m_sceneRecip.GetTriangle()->SetBZVisible(bBZVisible);
	m_sceneRecip.emitUpdate();
	//m_sceneRecip.emitAllParams();
}

TazDlg::~TazDlg()
{
	//log_debug("In ", __func__, ".");

	//m_settings.setValue("main/width", this->width());
	//m_settings.setValue("main/height", this->height());
	m_settings.setValue("main/geo", saveGeometry());

	if(m_pRecip3d) { delete m_pRecip3d; m_pRecip3d = 0; }
	if(m_pviewRecip) { delete m_pviewRecip; m_pviewRecip = 0; }
	if(m_pEllipseDlg) { delete m_pEllipseDlg; m_pEllipseDlg = 0; }
	if(m_pEllipseDlg3D) { delete m_pEllipseDlg3D; m_pEllipseDlg3D = 0; }
	if(m_pReso) { delete m_pReso; m_pReso = 0; }
	if(m_pSpuri) { delete m_pSpuri; m_pSpuri = 0; }
	if(m_pNeutronDlg) { delete m_pNeutronDlg; m_pNeutronDlg = 0; }
	if(m_pGotoDlg) { delete m_pGotoDlg; m_pGotoDlg = 0; }
	if(m_pSrvDlg) { delete m_pSrvDlg; m_pSrvDlg = 0; }
	if(m_pNetCacheDlg) { delete m_pNetCacheDlg; m_pNetCacheDlg = 0; }
	if(m_pNicosCache) { delete m_pNicosCache; m_pNicosCache = 0; }
}


void TazDlg::ShowNeutronDlg()
{
	if(!m_pNeutronDlg)
		m_pNeutronDlg = new NeutronDlg(this, &m_settings);

	m_pNeutronDlg->show();
	m_pNeutronDlg->activateWindow();
}

void TazDlg::ShowGotoDlg()
{
	if(!m_pGotoDlg)
		m_pGotoDlg = new GotoDlg(this, &m_settings);

	m_pGotoDlg->show();
	m_pGotoDlg->activateWindow();
}


void TazDlg::UpdateDs()
{
	double dMonoD = editMonoD->text().toDouble();
	double dAnaD = editAnaD->text().toDouble();

	m_sceneRecip.SetDs(dMonoD, dAnaD);

	ResoParams resoparams;
	resoparams.bMonoDChanged = 1;
	resoparams.bAnaDChanged = 1;
	resoparams.dMonoD = dMonoD;
	resoparams.dAnaD = dAnaD;

	if(m_pGotoDlg)
	{
		m_pGotoDlg->SetD(editMonoD->text().toDouble(), editAnaD->text().toDouble());
		m_pGotoDlg->CalcMonoAna();
		m_pGotoDlg->CalcSample();
	}

	emit ResoParamsChanged(resoparams);
}

std::ostream& operator<<(std::ostream& ostr, const Lattice<double>& lat)
{
	ostr << "a = " << lat.GetA();
	ostr << ", b = " << lat.GetB();
	ostr << ", c = " << lat.GetC();
	ostr << ", alpha = " << lat.GetAlpha();
	ostr << ", beta = " << lat.GetBeta();
	ostr << ", gamma = " << lat.GetGamma();
	return ostr;
}

void TazDlg::SetCrystalType()
{
	m_crystalsys = CrystalSystem::CRYS_NOT_SET;
	
	SpaceGroup *pSpaceGroup = 0;
	int iSpaceGroupIdx = comboSpaceGroups->currentIndex();
	if(iSpaceGroupIdx != 0)
		pSpaceGroup = (SpaceGroup*)comboSpaceGroups->itemData(iSpaceGroupIdx).value<void*>();
	if(pSpaceGroup)
		m_crystalsys = pSpaceGroup->GetCrystalSystem();
		
	CheckCrystalType();
}

// TODO
void TazDlg::CheckCrystalType()
{
	switch(m_crystalsys)
	{
		case CRYS_CUBIC:
			editA->setEnabled(1);
			editB->setEnabled(0);
			editC->setEnabled(0);
			editAlpha->setEnabled(0);
			editBeta->setEnabled(0);
			editGamma->setEnabled(0);

			editARecip->setEnabled(1);
			editBRecip->setEnabled(0);
			editCRecip->setEnabled(0);
			editAlphaRecip->setEnabled(0);
			editBetaRecip->setEnabled(0);
			editGammaRecip->setEnabled(0);

			editB->setText(editA->text());
			editC->setText(editA->text());
			editBRecip->setText(editARecip->text());
			editCRecip->setText(editARecip->text());
			editAlpha->setText("90");
			editBeta->setText("90");
			editGamma->setText("90");
			editAlphaRecip->setText("90");
			editBetaRecip->setText("90");
			editGammaRecip->setText("90");
			break;
			
		case CRYS_HEXAGONAL:
			editA->setEnabled(1);
			editB->setEnabled(0);
			editC->setEnabled(1);
			editAlpha->setEnabled(0);
			editBeta->setEnabled(0);
			editGamma->setEnabled(0);

			editARecip->setEnabled(1);
			editBRecip->setEnabled(0);
			editCRecip->setEnabled(1);
			editAlphaRecip->setEnabled(0);
			editBetaRecip->setEnabled(0);
			editGammaRecip->setEnabled(0);

			editB->setText(editA->text());
			editBRecip->setText(editARecip->text());
			editAlpha->setText("90");
			editBeta->setText("90");
			editGamma->setText("120");
			editAlphaRecip->setText("90");
			editBetaRecip->setText("90");
			editGammaRecip->setText("60");
			break;
			
		case CRYS_MONOCLINIC:
			editA->setEnabled(1);
			editB->setEnabled(1);
			editC->setEnabled(1);
			editAlpha->setEnabled(1);
			editBeta->setEnabled(0);
			editGamma->setEnabled(0);

			editARecip->setEnabled(1);
			editBRecip->setEnabled(1);
			editCRecip->setEnabled(1);
			editAlphaRecip->setEnabled(1);
			editBetaRecip->setEnabled(0);
			editGammaRecip->setEnabled(0);

			editBeta->setText("90");
			editGamma->setText("90");
			editBetaRecip->setText("90");
			editGammaRecip->setText("90");
			break;
			
		case CRYS_ORTHORHOMBIC:
			editA->setEnabled(1);
			editB->setEnabled(1);
			editC->setEnabled(1);
			editAlpha->setEnabled(0);
			editBeta->setEnabled(0);
			editGamma->setEnabled(0);

			editARecip->setEnabled(1);
			editBRecip->setEnabled(1);
			editCRecip->setEnabled(1);
			editAlphaRecip->setEnabled(0);
			editBetaRecip->setEnabled(0);
			editGammaRecip->setEnabled(0);

			editAlpha->setText("90");
			editBeta->setText("90");
			editGamma->setText("90");
			editAlphaRecip->setText("90");
			editBetaRecip->setText("90");
			editGammaRecip->setText("90");
			break;
			
		case CRYS_TETRAGONAL:
			editA->setEnabled(1);
			editB->setEnabled(0);
			editC->setEnabled(1);
			editAlpha->setEnabled(0);
			editBeta->setEnabled(0);
			editGamma->setEnabled(0);

			editARecip->setEnabled(1);
			editBRecip->setEnabled(0);
			editCRecip->setEnabled(1);
			editAlphaRecip->setEnabled(0);
			editBetaRecip->setEnabled(0);
			editGammaRecip->setEnabled(0);

			editB->setText(editA->text());
			editBRecip->setText(editARecip->text());
			editAlpha->setText("90");
			editBeta->setText("90");
			editGamma->setText("90");
			editAlphaRecip->setText("90");
			editBetaRecip->setText("90");
			editGammaRecip->setText("90");
			break;

		case CRYS_TRIGONAL:
			editA->setEnabled(1);
			editB->setEnabled(0);
			editC->setEnabled(0);
			editAlpha->setEnabled(1);
			editBeta->setEnabled(0);
			editGamma->setEnabled(0);

			editARecip->setEnabled(1);
			editBRecip->setEnabled(0);
			editCRecip->setEnabled(0);
			editAlphaRecip->setEnabled(1);
			editBetaRecip->setEnabled(0);
			editGammaRecip->setEnabled(0);

			editB->setText(editA->text());
			editC->setText(editA->text());
			editBRecip->setText(editARecip->text());
			editCRecip->setText(editARecip->text());
			editBeta->setText(editAlpha->text());
			editGamma->setText(editAlpha->text());
			editBetaRecip->setText(editAlphaRecip->text());
			editGammaRecip->setText(editAlphaRecip->text());
			break;

		case CRYS_TRICLINIC:
		case CRYS_NOT_SET:
		default:
			editA->setEnabled(1);
			editB->setEnabled(1);
			editC->setEnabled(1);
			editAlpha->setEnabled(1);
			editBeta->setEnabled(1);
			editGamma->setEnabled(1);

			editARecip->setEnabled(1);
			editBRecip->setEnabled(1);
			editCRecip->setEnabled(1);
			editAlphaRecip->setEnabled(1);
			editBetaRecip->setEnabled(1);
			editGammaRecip->setEnabled(1);
			break;
	}
}

void TazDlg::CalcPeaksRecip()
{
	double a = editARecip->text().toDouble();
	double b = editBRecip->text().toDouble();
	double c = editCRecip->text().toDouble();

	double alpha = editAlphaRecip->text().toDouble()/180.*M_PI;
	double beta = editBetaRecip->text().toDouble()/180.*M_PI;
	double gamma = editGammaRecip->text().toDouble()/180.*M_PI;

	Lattice<double> lattice(a,b,c, alpha,beta,gamma);
	Lattice<double> recip = lattice.GetRecip();

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

	try
	{
		// lattice
		double a = editA->text().toDouble();
		double b = editB->text().toDouble();
		double c = editC->text().toDouble();

		double alpha = editAlpha->text().toDouble()/180.*M_PI;
		double beta = editBeta->text().toDouble()/180.*M_PI;
		double gamma = editGamma->text().toDouble()/180.*M_PI;

		Lattice<double> lattice(a,b,c, alpha,beta,gamma);
		Lattice<double> recip_unrot = lattice.GetRecip();




		// scattering plane
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

		//----------------------------------------------------------------------
		// show integer up vector
		unsigned int iMaxDec = 4;	// TODO: determine max. # of entered decimals
		ublas::vector<int> ivecUp = ::cross_3(
			::make_vec<ublas::vector<int>>({int(dX0*std::pow(10, iMaxDec)),
											int(dX1*std::pow(10, iMaxDec)),
											int(dX2*std::pow(10, iMaxDec))}),
			::make_vec<ublas::vector<int>>({int(dY0*std::pow(10, iMaxDec)),
											int(dY1*std::pow(10, iMaxDec)),
											int(dY2*std::pow(10, iMaxDec))}));
		ivecUp = get_gcd_vec(ivecUp);
		editScatZ0->setText(std::to_string(ivecUp[0]).c_str());
		editScatZ1->setText(std::to_string(ivecUp[1]).c_str());
		editScatZ2->setText(std::to_string(ivecUp[2]).c_str());
		//----------------------------------------------------------------------

		ublas::vector<double> vecX0 = ublas::zero_vector<double>(3);
		Plane<double> plane(vecX0, vecPlaneX, vecPlaneY);
		if(!plane.IsValid())
		{
			log_err("Invalid scattering plane.");
			return;
		}


		if(m_pGotoDlg)
		{
			m_pGotoDlg->SetLattice(lattice);
			m_pGotoDlg->SetScatteringPlane(make_vec({dX0, dX1, dX2}), make_vec({dY0, dY1, dY2}));
			m_pGotoDlg->CalcSample();
		}


		/*// rotated lattice
		double dPhi = spinRotPhi->value() / 180. * M_PI;
		double dTheta = spinRotTheta->value() / 180. * M_PI;
		double dPsi = spinRotPsi->value() / 180. * M_PI;
		//lattice.RotateEuler(dPhi, dTheta, dPsi);*/

		ublas::vector<double> dir0 = plane.GetDir0();
		ublas::vector<double> dirup = plane.GetNorm();
		ublas::vector<double> dir1 = cross_3(dirup, dir0);

		double dDir0Len = ublas::norm_2(dir0);
		double dDir1Len = ublas::norm_2(dir1);
		double dDirUpLen = ublas::norm_2(dirup);

		if(float_equal(dDir0Len, 0.) || float_equal(dDir1Len, 0.) || float_equal(dDirUpLen, 0.)
			|| ::isnan(dDir0Len) || ::isnan(dDir1Len) || ::isnan(dDirUpLen))
		{
			log_err("Invalid scattering plane.");
			return;
		}

		dir0 /= dDir0Len;
		dir1 /= dDir1Len;
		//dirup /= dDirUpLen;

		//lattice.RotateEulerRecip(dir0, dir1, dirup, dPhi, dTheta, dPsi);
		Lattice<double> recip = lattice.GetRecip();



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
		double dVol_recip = recip.GetVol() /*/ (2.*M_PI*2.*M_PI*2.*M_PI)*/;

		std::wostringstream ostrSample;
		ostrSample.precision(4);
		ostrSample << "Sample";
		ostrSample << " - ";
		ostrSample << "Unit Cell Vol.: ";
		ostrSample << "Real: " << dVol << " " << strAA << strThree;
		ostrSample << ", Recip.: " << dVol_recip << " " << strAA << strMinus << strThree;
		groupSample->setTitle(QString::fromWCharArray(ostrSample.str().c_str()));


		const char* pcCryTy = "<not set>";
		SpaceGroup *pSpaceGroup = 0;
		int iSpaceGroupIdx = comboSpaceGroups->currentIndex();
		if(iSpaceGroupIdx != 0)
			pSpaceGroup = (SpaceGroup*)comboSpaceGroups->itemData(iSpaceGroupIdx).value<void*>();
			
		if(pSpaceGroup)
			pcCryTy = pSpaceGroup->GetCrystalSystemName();

		editCrystalSystem->setText(pcCryTy);

		m_sceneRecip.GetTriangle()->CalcPeaks(lattice, recip, recip_unrot, plane, pSpaceGroup);
		if(m_sceneRecip.getSnapq())
			m_sceneRecip.SnapToNearestPeak(m_sceneRecip.GetTriangle()->GetNodeGq());
		m_sceneRecip.emitUpdate();

		if(m_pRecip3d)
			m_pRecip3d->CalcPeaks(lattice, recip, recip_unrot, plane, pSpaceGroup);
	}
	catch(const std::exception& ex)
	{
		m_sceneRecip.GetTriangle()->ClearPeaks();
		log_err(ex.what());
	}
}

void TazDlg::UpdateSampleSense()
{
	const bool bSense = checkSenseS->isChecked();
	m_sceneRecip.SetSampleSense(bSense);

	if(m_pGotoDlg)
	{
		m_pGotoDlg->SetSampleSense(bSense);
		m_pGotoDlg->CalcSample();
	}

	ResoParams params;
	params.bSensesChanged[1] = 1;
	params.bScatterSenses[1] = bSense;
	emit ResoParamsChanged(params);

	m_sceneRecip.emitUpdate();
}

void TazDlg::UpdateMonoSense()
{
	const bool bSense = checkSenseM->isChecked();
	m_sceneRecip.SetMonoSense(bSense);

	if(m_pGotoDlg)
	{
		m_pGotoDlg->SetMonoSense(bSense);
		m_pGotoDlg->CalcMonoAna();
	}

	ResoParams params;
	params.bSensesChanged[0] = 1;
	params.bScatterSenses[0] = bSense;
	emit ResoParamsChanged(params);
}

void TazDlg::UpdateAnaSense()
{
	const bool bSense = checkSenseA->isChecked();
	m_sceneRecip.SetAnaSense(bSense);

	if(m_pGotoDlg)
	{
		m_pGotoDlg->SetAnaSense(bSense);
		m_pGotoDlg->CalcMonoAna();
	}

	ResoParams params;
	params.bSensesChanged[2] = 1;
	params.bScatterSenses[2] = bSense;
	emit ResoParamsChanged(params);
}

void TazDlg::Show3D()
{
	if(!m_pRecip3d)
	{
		m_pRecip3d = new Recip3DDlg(this);

		double dTol = s_dPlaneDistTolerance;
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

void TazDlg::EnableBZ(bool bEnable)
{
	m_sceneRecip.GetTriangle()->SetBZVisible(bEnable);
}

void TazDlg::EnableRealQDir(bool bEnable)
{
	m_sceneReal.GetTasLayout()->SetRealQVisible(bEnable);
}


void TazDlg::RepopulateSpaceGroups()
{
	if(!m_pmapSpaceGroups)
		return;

	for(int iCnt=comboSpaceGroups->count()-1; iCnt>0; --iCnt)
		comboSpaceGroups->removeItem(iCnt);

	std::string strFilter = editSpaceGroupsFilter->text().toStdString();

	for(const t_mapSpaceGroups::value_type& pair : *m_pmapSpaceGroups)
	{
		const std::string& strName = pair.second.GetName();

		typedef const boost::iterator_range<std::string::const_iterator> t_striterrange;
		if(strFilter!="" &&
				!boost::ifind_first(t_striterrange(strName.begin(), strName.end()),
									t_striterrange(strFilter.begin(), strFilter.end())))
			continue;

		comboSpaceGroups->insertItem(comboSpaceGroups->count(),
									strName.c_str(),
									QVariant::fromValue((void*)&pair.second));
	}
}


void TazDlg::RecipCoordsChanged(double dh, double dk, double dl)
{
	std::ostringstream ostrPos;
	ostrPos << "(" << dh << ", " << dk << ", " << dl  << ") rlu";

	m_pCoordStatusMsg->setText(ostrPos.str().c_str());
}

//--------------------------------------------------------------------------------
// loading/saving

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

bool TazDlg::Load(const char* pcFile)
{
	Disconnect();
	m_strCurFile = pcFile;

	const std::string strXmlRoot("taz/");

	std::string strFile1 = pcFile;
	std::string strDir = get_dir(strFile1);


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
	
	std::string strDescr = xml.QueryString((strXmlRoot+"sample/descr").c_str(), "", &bOk);
	if(bOk)
		this->editDescr->setPlainText(strDescr.c_str());


	/*// spin boxes
	for(unsigned int iSpinBox=0; iSpinBox<m_vecSpinBoxesSample.size(); ++iSpinBox)
	{
		double dVal = xml.Query<double>((strXmlRoot+m_vecSpinBoxNamesSample[iSpinBox]).c_str(), 0., &bOk);
		if(bOk)
			m_vecSpinBoxesSample[iSpinBox]->setValue(dVal);
	}*/


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

	int bSmallqSnapped = xml.Query<int>((strXmlRoot + "recip/snap_q").c_str(), 1, &bOk);
	if(bOk)
		m_pSnapSmallq->setChecked(bSmallqSnapped!=0);

	int bBZEnabled = xml.Query<int>((strXmlRoot + "recip/enable_bz").c_str(), 0, &bOk);
	if(bOk)
		m_pBZ->setChecked(bBZEnabled!=0);

	int bRealQEnabled = xml.Query<int>((strXmlRoot + "real/enable_realQDir").c_str(), 0, &bOk);
	if(bOk)
		m_pShowRealQDir->setChecked(bRealQEnabled!=0);

	std::string strSpaceGroup = xml.QueryString((strXmlRoot + "sample/spacegroup").c_str(), "", &bOk);
	trim(strSpaceGroup);
	if(bOk)
	{
		editSpaceGroupsFilter->clear();
		int iSGIdx = comboSpaceGroups->findText(strSpaceGroup.c_str());
		if(iSGIdx >= 0)
			comboSpaceGroups->setCurrentIndex(iSGIdx);
	}

	if(xml.Exists((strXmlRoot + "reso").c_str()))
	{
		InitReso();
		m_pReso->Load(xml, strXmlRoot);
	}

	m_strCurFile = strFile1;
	setWindowTitle((s_strTitle + " - " + m_strCurFile).c_str());

	CalcPeaks();
	return true;
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

	mapConf[strXmlRoot + "sample/descr"] = editDescr->toPlainText().toStdString();

	/*// spin boxes
	for(unsigned int iSpinBox=0; iSpinBox<m_vecSpinBoxesSample.size(); ++iSpinBox)
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << m_vecSpinBoxesSample[iSpinBox]->value();

		mapConf[strXmlRoot + m_vecSpinBoxNamesSample[iSpinBox]] = ostrVal.str();
	}*/


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

	bool bSmallqSnapped = m_sceneRecip.getSnapq();
	mapConf[strXmlRoot + "recip/snap_q"] = (bSmallqSnapped ? "1" : "0");

	bool bBZEnabled = m_pBZ->isChecked();
	mapConf[strXmlRoot + "recip/enable_bz"] = (bBZEnabled ? "1" : "0");

	bool bRealQDir = m_pShowRealQDir->isChecked();
	mapConf[strXmlRoot + "real/enable_realQDir"] = (bRealQDir ? "1" : "0");


	mapConf[strXmlRoot + "sample/spacegroup"] = comboSpaceGroups->currentText().toStdString();


	if(m_pReso)
		m_pReso->Save(mapConf, strXmlRoot);


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
		std::string strDir = get_dir(strFile1);

		m_strCurFile = strFile1;
		setWindowTitle((s_strTitle + " - " + m_strCurFile).c_str());
		bool bOk = Save();

		if(bOk)
			m_settings.setValue("main/last_dir", QString(strDir.c_str()));

		return bOk;
	}

	return false;
}
//--------------------------------------------------------------------------------

void TazDlg::ShowSpurions()
{
	if(!m_pSpuri)
	{
		m_pSpuri = new SpurionDlg(this, &m_settings);

		QObject::connect(&m_sceneRecip, SIGNAL(paramsChanged(const RecipParams&)),
						m_pSpuri, SLOT(paramsChanged(const RecipParams&)));

		m_sceneRecip.emitAllParams();
	}

	m_pSpuri->show();
	m_pSpuri->activateWindow();
}

void TazDlg::spurionInfo(const ElasticSpurion& spuri,
					const std::vector<InelasticSpurion>& vecInelCKI,
					const std::vector<InelasticSpurion>& vecInelCKF)
{
	std::ostringstream ostrMsg;
	if(spuri.bAType || spuri.bMType || vecInelCKI.size() || vecInelCKF.size())
		ostrMsg << "Warning: ";

	if(spuri.bAType || spuri.bMType)
	{
		ostrMsg << "Spurious elastic scattering of type ";
		if(spuri.bAType && spuri.bMType)
		{
			ostrMsg << "A and M";
			ostrMsg << (spuri.bAKfSmallerKi ? " (kf<ki)" : " (kf>ki)");
		}
		else if(spuri.bAType)
		{
			ostrMsg << "A";
			ostrMsg << (spuri.bAKfSmallerKi ? " (kf<ki)" : " (kf>ki)");
		}
		else if(spuri.bMType)
		{
			ostrMsg << "M";
			ostrMsg << (spuri.bMKfSmallerKi ? " (kf<ki)" : " (kf>ki)");
		}
		ostrMsg << " detected.";

		if(vecInelCKI.size() || vecInelCKF.size())
			ostrMsg << " ";
	}

	const std::string& strDelta = ::get_spec_char_utf8("Delta");

	if(vecInelCKI.size())
	{
		ostrMsg << "Spurious inelastic CKI scattering at "
				<< strDelta << "E = ";
		for(unsigned int iInel=0; iInel<vecInelCKI.size(); ++iInel)
		{
			ostrMsg << vecInelCKI[iInel].dE_meV << " meV";
			if(iInel != vecInelCKI.size()-1)
				ostrMsg << ", ";
		}
		ostrMsg << " detected.";

		if(vecInelCKF.size())
			ostrMsg << " ";
	}

	if(vecInelCKF.size())
	{
		ostrMsg << "Spurious inelastic CKF scattering at "
				<< strDelta << "E = ";
		for(unsigned int iInel=0; iInel<vecInelCKF.size(); ++iInel)
		{
			ostrMsg << vecInelCKF[iInel].dE_meV << " meV";
			if(iInel != vecInelCKF.size()-1)
				ostrMsg << ", ";
		}
		ostrMsg << " detected.";
	}

	m_pStatusMsg->setText(QString::fromUtf8(ostrMsg.str().c_str(), ostrMsg.str().size()));
}

//--------------------------------------------------------------------------------
// parameter dialogs

void TazDlg::ShowRecipParams()
{
	m_dlgRecipParam.show();
	m_dlgRecipParam.activateWindow();
}

void TazDlg::ShowRealParams()
{
	m_dlgRealParam.show();
	m_dlgRealParam.activateWindow();
}


//--------------------------------------------------------------------------------
// reso stuff
void TazDlg::InitReso()
{
	if(!m_pReso)
	{
		m_pReso = new ResoDlg(this, &m_settings);

		QObject::connect(this, SIGNAL(ResoParamsChanged(const ResoParams&)),
						m_pReso, SLOT(ResoParamsChanged(const ResoParams&)));
		QObject::connect(&m_sceneRecip, SIGNAL(paramsChanged(const RecipParams&)),
						m_pReso, SLOT(RecipParamsChanged(const RecipParams&)));
		QObject::connect(&m_sceneReal, SIGNAL(paramsChanged(const RealParams&)),
						m_pReso, SLOT(RealParamsChanged(const RealParams&)));

		UpdateDs();
		UpdateMonoSense();
		UpdateSampleSense();
		UpdateAnaSense();

		m_sceneRecip.emitAllParams();
		m_sceneReal.emitAllParams();
	}
}

void TazDlg::ShowResoParams()
{
	InitReso();

	m_pReso->show();
	m_pReso->activateWindow();
}

void TazDlg::ShowResoEllipses()
{
	InitReso();

	if(!m_pEllipseDlg)
	{
		m_pEllipseDlg = new EllipseDlg(this, &m_settings);
		QObject::connect(m_pReso, SIGNAL(ResoResults(const ublas::matrix<double>&, const ublas::vector<double>&)),
						 m_pEllipseDlg, SLOT(SetParams(const ublas::matrix<double>&, const ublas::vector<double>&)));

		m_pReso->EmitResults();
	}

	m_pEllipseDlg->show();
	m_pEllipseDlg->activateWindow();
}

void TazDlg::ShowResoEllipses3D()
{
	InitReso();

	if(!m_pEllipseDlg3D)
	{
		m_pEllipseDlg3D = new EllipseDlg3D(this, &m_settings);
		QObject::connect(m_pReso, SIGNAL(ResoResults(const ublas::matrix<double>&, const ublas::vector<double>&)),
						 m_pEllipseDlg3D, SLOT(SetParams(const ublas::matrix<double>&, const ublas::vector<double>&)));

		m_pReso->EmitResults();
	}

	m_pEllipseDlg3D->show();
	m_pEllipseDlg3D->activateWindow();
}



//--------------------------------------------------------------------------------
// context menus

void TazDlg::RecipContextMenu(const QPoint& _pt)
{
	if(!m_pviewRecip) return;

	QPoint pt = this->m_pviewRecip->mapToGlobal(_pt);
	m_pMenuViewRecip->exec(pt);
}

void TazDlg::RealContextMenu(const QPoint& _pt)
{
	if(!m_pviewReal) return;

	QPoint pt = this->m_pviewReal->mapToGlobal(_pt);
	m_pMenuViewReal->exec(pt);
}



//--------------------------------------------------------------------------------
// svg export
#include <QtSvg/QSvgGenerator>

void TazDlg::ExportReal() { ExportSceneSVG(m_sceneReal); }
void TazDlg::ExportRecip() { ExportSceneSVG(m_sceneRecip); }

void TazDlg::ExportSceneSVG(QGraphicsScene& scene)
{
	QString strDirLast = m_settings.value("main/last_dir_export", ".").toString();
	QString strFile = QFileDialog::getSaveFileName(this,
								"Export SVG",
								strDirLast,
								"SVG files (*.svg *.SVG)");
	if(strFile == "")
		return;


	QRectF rect = scene.sceneRect();

	QSvgGenerator svg;
	svg.setFileName(/*"/home/tweber/0000.svg"*/ strFile);
	svg.setSize(QSize(rect.width(), rect.height()));
	//svg.setResolution(300);
	svg.setViewBox(QRectF(0, 0, rect.width(), rect.height()));
	svg.setDescription("Created with Takin");

	QPainter painter;
	painter.begin(&svg);
	scene.render(&painter);
	painter.end();


	std::string strDir = get_dir(strFile.toStdString());
	m_settings.setValue("main/last_dir_export", QString(strDir.c_str()));
}



//--------------------------------------------------------------------------------
// server stuff

void TazDlg::ShowConnectDlg()
{
	if(!m_pSrvDlg)
	{
		m_pSrvDlg = new SrvDlg(this, &m_settings);
		QObject::connect(m_pSrvDlg, SIGNAL(ConnectTo(const QString&, const QString&)),
						this, SLOT(ConnectTo(const QString&, const QString&)));
	}

	m_pSrvDlg->show();
	m_pSrvDlg->activateWindow();
}

void TazDlg::ConnectTo(const QString& _strHost, const QString& _strPort)
{
	Disconnect();

	std::string strHost =  _strHost.toStdString();
	std::string strPort =  _strPort.toStdString();
	
	m_pNicosCache = new NicosCache();
	QObject::connect(m_pNicosCache, SIGNAL(vars_changed(const CrystalOptions&, const TriangleOptions&)),
					this, SLOT(VarsChanged(const CrystalOptions&, const TriangleOptions&)));
	QObject::connect(m_pNicosCache, SIGNAL(connected(const QString&, const QString&)),
					this, SLOT(Connected(const QString&, const QString&)));
	QObject::connect(m_pNicosCache, SIGNAL(disconnected()),
					this, SLOT(Disconnected()));
					
	if(!m_pNetCacheDlg)
		m_pNetCacheDlg = new NetCacheDlg(this, &m_settings);

	m_pNetCacheDlg->ClearAll();
	QObject::connect(m_pNicosCache, SIGNAL(updated_cache_value(const std::string&, const CacheVal&)), 
					m_pNetCacheDlg, SLOT(UpdateValue(const std::string&, const CacheVal&)));

	m_pNicosCache->connect(strHost, strPort);
}

void TazDlg::Disconnect()
{
	if(m_pNicosCache)
	{
		m_pNicosCache->disconnect();
		
		QObject::disconnect(m_pNicosCache, SIGNAL(vars_changed(const CrystalOptions&, const TriangleOptions&)),
						this, SLOT(VarsChanged(const CrystalOptions&, const TriangleOptions&)));
		QObject::disconnect(m_pNicosCache, SIGNAL(connected(const QString&, const QString&)),
						this, SLOT(Connected(const QString&, const QString&)));
		QObject::disconnect(m_pNicosCache, SIGNAL(disconnected()),
						this, SLOT(Disconnected()));
						
		QObject::disconnect(m_pNicosCache, SIGNAL(updated_cache_value(const std::string&, const CacheVal&)), 
						m_pNetCacheDlg, SLOT(UpdateValue(const std::string&, const CacheVal&)));
		
		delete m_pNicosCache;
		m_pNicosCache = 0;
	}

	statusBar()->showMessage("Disconnected.", DEFAULT_MSG_TIMEOUT);
}

void TazDlg::ShowNetCache()
{
	if(!m_pNetCacheDlg)
		m_pNetCacheDlg = new NetCacheDlg(this, &m_settings);

	m_pNetCacheDlg->show();
	m_pNetCacheDlg->activateWindow();
}

void TazDlg::NetRefresh()
{
	if(!m_pNicosCache)
	{
		QMessageBox::warning(this, "Warning", "Not connected to a server.");
		return;
	}

	m_pNicosCache->RefreshKeys();
}

void TazDlg::Connected(const QString& strHost, const QString& strSrv)
{
	m_strCurFile = "";

	setWindowTitle((s_strTitle + " - ").c_str() + strHost + ":" + strSrv);
	statusBar()->showMessage("Connected to " + strHost + " on port " + strSrv + ".", DEFAULT_MSG_TIMEOUT);
}

void TazDlg::Disconnected()
{
	setWindowTitle((s_strTitle).c_str());
}

void TazDlg::VarsChanged(const CrystalOptions& crys, const TriangleOptions& triag)
{
	if(crys.strSampleName != "")
	{
		this->editDescr->setPlainText(crys.strSampleName.c_str());
	}

	if(crys.bChangedLattice)
	{
		this->editA->setText(QString::number(crys.dLattice[0]));
		this->editB->setText(QString::number(crys.dLattice[1]));
		this->editC->setText(QString::number(crys.dLattice[2]));

		CalcPeaks();
	}

	if(crys.bChangedLatticeAngles)
	{
		this->editAlpha->setText(QString::number(crys.dLatticeAngles[0]));
		this->editBeta->setText(QString::number(crys.dLatticeAngles[1]));
		this->editGamma->setText(QString::number(crys.dLatticeAngles[2]));

		CalcPeaks();
	}

	if(crys.bChangedSpacegroup)
	{
		editSpaceGroupsFilter->clear();
		int iSGIdx = comboSpaceGroups->findText(crys.strSpacegroup.c_str());
		if(iSGIdx >= 0)
			comboSpaceGroups->setCurrentIndex(iSGIdx);

		CalcPeaks();
	}

	if(crys.bChangedPlane1)
	{
		this->editScatX0->setText(QString::number(crys.dPlane1[0]));
		this->editScatX1->setText(QString::number(crys.dPlane1[1]));
		this->editScatX2->setText(QString::number(crys.dPlane1[2]));

		CalcPeaks();
	}
	if(crys.bChangedPlane2)
	{
		this->editScatY0->setText(QString::number(crys.dPlane2[0]));
		this->editScatY1->setText(QString::number(crys.dPlane2[1]));
		this->editScatY2->setText(QString::number(crys.dPlane2[2]));

		CalcPeaks();
	}

	if(triag.bChangedMonoD)
	{
		this->editMonoD->setText(QString::number(triag.dMonoD));
		UpdateDs();
	}
	if(triag.bChangedAnaD)
	{
		this->editAnaD->setText(QString::number(triag.dAnaD));
		UpdateDs();
	}


	// hack!
	if(triag.bChangedTwoTheta && !checkSenseS->isChecked())
		const_cast<TriangleOptions&>(triag).dTwoTheta = -triag.dTwoTheta;

	m_sceneReal.triangleChanged(triag);
	m_sceneReal.emitUpdate(triag);

	UpdateMonoSense();
	UpdateAnaSense();
	UpdateSampleSense();
	//m_sceneReal.emitAllParams();


	if(triag.bChangedAngleKiVec0)
	{
		m_sceneRecip.tasChanged(triag);
		m_sceneRecip.emitUpdate();
		//m_sceneRecip.emitAllParams();
	}
}


//--------------------------------------------------------------------------------
// about dialog
#include <boost/version.hpp>
#include <qwt_global.h>

void TazDlg::ShowAbout()
{
	const std::wstring& _strRet = get_spec_char_utf16("return");
	const std::wstring& _strBullet = get_spec_char_utf16("bullet");
	const std::wstring& _strArrow = get_spec_char_utf16("rightarrow");

	const QString strRet = QString::fromUtf16((ushort*)_strRet.c_str(), _strRet.length());
	const QString strBullet = QString::fromUtf16((ushort*)_strBullet.c_str(), _strBullet.length());
	const QString strArrow = QString::fromUtf16((ushort*)_strArrow.c_str(), _strArrow.length());


	QString strAbout;
	strAbout += "Takin version 0.8\n";
	strAbout += "Written by Tobias Weber, 2014";
	strAbout += "\n\n";
	
	strAbout += "Takin is free software: you can redistribute it and/or modify ";
	strAbout += "it under the terms of the GNU General Public License as published by ";
	strAbout += "the Free Software Foundation, either version 3 of the License, or ";
	strAbout += "(at your option) any later version.\n\n";
	strAbout += "Takin is distributed in the hope that it will be useful, ";
	strAbout += "but WITHOUT ANY WARRANTY; without even the implied warranty of ";
	strAbout += "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the ";
	strAbout += "GNU General Public License for more details.\n\n";
	strAbout += "You should have received a copy of the GNU General Public License ";
	strAbout += "along with Takin.  If not, see <http://www.gnu.org/licenses/>.\n\n";

	strAbout += "Built with CC version ";
	strAbout += QString(__VERSION__);
	strAbout += "\n";

	strAbout += "Build date: ";
	strAbout += QString(__DATE__);
	strAbout += ", ";
	strAbout += QString(__TIME__);
	strAbout += "\n\n";

	strAbout += strBullet + " ";
	strAbout += "Uses Qt version ";
	strAbout += QString(QT_VERSION_STR);
	strAbout += "       \t" + strArrow + " http://qt-project.org\n";
	strAbout += strBullet + " ";
	strAbout += "Uses Qwt version ";
	strAbout += QString(QWT_VERSION_STR);
	strAbout += "      \t" + strArrow + " http://qwt.sourceforge.net\n";

	strAbout += strBullet + " ";
	strAbout += "Uses Boost version ";
	std::string strBoost = BOOST_LIB_VERSION;
	find_all_and_replace<std::string>(strBoost, "_", ".");
	strAbout += strBoost.c_str();
	strAbout += "    \t" + strArrow + " http://www.boost.org\n";

	strAbout += strBullet + " ";
	strAbout += "Uses Lapack/e version 3";
	strAbout += "    \t" + strArrow + " http://www.netlib.org/lapack\n";

	strAbout += "\n";
	strAbout += strBullet + " ";
	strAbout += "Uses space group calculations ported from Nicos version 2";
	strAbout += "\n\t " + strArrow + " https://forge.frm2.tum.de/redmine/projects/nicos\n";
	strAbout += strBullet + " ";
	strAbout += "Uses space group data from PowderCell version 2.3";
	strAbout += "\n\t " + strArrow
			+ " www.bam.de/de/service/publikationen/" + strRet
			+ "\n\t\tpowder_cell_a.htm\n";

	strAbout += strBullet + " ";
	strAbout += "Uses resolution algorithms ported from Rescal version 5";
	strAbout += "\n\t " + strArrow
			+ " www.ill.eu/en/html/instruments-support/" + strRet
			+ "\n\t\tcomputing-for-science/cs-software/" + strRet
			+ "\n\t\tall-software/matlab-ill/rescal-for-matlab\n";

	QMessageBox::information(this, "About Takin", strAbout);
}

#include "taz.moc"
