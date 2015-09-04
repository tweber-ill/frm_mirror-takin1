/*
 * TAS tool
 * @author tweber
 * @date feb-2014
 * @copyright GPLv2
 */

#include "taz.h"

#include <iostream>
#include <algorithm>
#include <vector>
#include <boost/algorithm/string.hpp>

#include <QApplication>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>

#include "tlibs/math/lattice.h"
#include "tlibs/string/spec_char.h"
#include "tlibs/string/string.h"
#include "tlibs/helper/flags.h"
#include "tlibs/file/xml.h"
#include "tlibs/file/recent.h"
#include "tlibs/helper/log.h"

const std::string TazDlg::s_strTitle = "Takin";

TazDlg::TazDlg(QWidget* pParent)
		: QMainWindow(pParent),
		  m_settings("tobis_stuff", "takin"),
		  m_pSettingsDlg(new SettingsDlg(this, &m_settings)),
		  m_pStatusMsg(new QLabel(this)),
		  m_pCoordStatusMsg(new QLabel(this)),
		  m_pmapSpaceGroups(get_space_groups()),
		  m_dlgRecipParam(this, &m_settings),
		  m_dlgRealParam(this, &m_settings),
		  m_pGotoDlg(new GotoDlg(this, &m_settings))
{
	//log_debug("In ", __func__, ".");

	const bool bSmallqVisible = 0;
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
	m_pCoordStatusMsg->setMinimumWidth(350);
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

	QObject::connect(&m_sceneRecip, SIGNAL(coordsChanged(double, double, double, bool, double, double, double)),
					this, SLOT(RecipCoordsChanged(double, double, double, bool, double, double, double)));

	QObject::connect(&m_sceneRecip, SIGNAL(spurionInfo(const tl::ElasticSpurion&, const std::vector<tl::InelasticSpurion<double>>&, const std::vector<tl::InelasticSpurion<double>>&)),
					this, SLOT(spurionInfo(const tl::ElasticSpurion&, const std::vector<tl::InelasticSpurion<double>>&, const std::vector<tl::InelasticSpurion<double>>&)));

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
	QObject::connect(checkPowder, SIGNAL(stateChanged(int)), this, SLOT(CalcPeaks()));



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

	m_pMenuRecent = new QMenu(this);
	m_pMenuRecent->setTitle("Recently Loaded");
	tl::RecentFiles recent(&m_settings, "main/recent");
	m_pMapperRecent = new QSignalMapper(m_pMenuRecent);
	QObject::connect(m_pMapperRecent, SIGNAL(mapped(const QString&)),
					this, SLOT(LoadFile(const QString&)));
	recent.FillMenu(m_pMenuRecent, m_pMapperRecent);
	pMenuFile->addMenu(m_pMenuRecent);

	QAction *pSave = new QAction(this);
	pSave->setText("Save");
	pSave->setIcon(QIcon::fromTheme("document-save"));
	pMenuFile->addAction(pSave);

	QAction *pSaveAs = new QAction(this);
	pSaveAs->setText("Save as...");
	pSaveAs->setIcon(QIcon::fromTheme("document-save-as"));
	pMenuFile->addAction(pSaveAs);

	pMenuFile->addSeparator();

	QAction *pImport = new QAction(this);
	pImport->setText("Import...");
	pImport->setIcon(QIcon::fromTheme(/*"text-x-generic-template"*/"emblem-documents"));
	pMenuFile->addAction(pImport);

	m_pMenuRecentImport = new QMenu(this);
	m_pMenuRecentImport->setTitle("Recently Imported");
	tl::RecentFiles recentimport(&m_settings, "main/recent_import");
	m_pMapperRecentImport = new QSignalMapper(m_pMenuRecentImport);
	QObject::connect(m_pMapperRecentImport, SIGNAL(mapped(const QString&)),
					this, SLOT(ImportFile(const QString&)));
	recentimport.FillMenu(m_pMenuRecentImport, m_pMapperRecentImport);
	pMenuFile->addMenu(m_pMenuRecentImport);

	pMenuFile->addSeparator();

	QAction *pScanViewer = new QAction(this);
	pScanViewer->setText("Scan Viewer...");
	pMenuFile->addAction(pScanViewer);

	pMenuFile->addSeparator();

	QAction *pSettings = new QAction(this);
	pSettings->setText("Settings...");
	pSettings->setIcon(QIcon::fromTheme("preferences-system"));
	pMenuFile->addAction(pSettings);

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

	QAction *pKeepAbsKiKf = new QAction(this);
	pKeepAbsKiKf->setText("Keep |ki| and |kf| Fixed");
	pKeepAbsKiKf->setCheckable(1);
	pKeepAbsKiKf->setChecked(m_sceneRecip.getKeepAbsKiKf());
	m_pMenuViewRecip->addAction(pKeepAbsKiKf);

	m_pBZ = new QAction(this);
	m_pBZ->setText("Show First Brillouin Zone");
	m_pBZ->setIcon(QIcon("res/brillouin.svg"));
	m_pBZ->setCheckable(1);
	m_pBZ->setChecked(bBZVisible);
	m_pMenuViewRecip->addAction(m_pBZ);

	m_pMenuViewRecip->addSeparator();

#if !defined NO_3D
	QAction *pView3D = new QAction(this);
	pView3D->setText("3D View...");
	pView3D->setIcon(QIcon::fromTheme("applications-graphics"));
	m_pMenuViewRecip->addAction(pView3D);

	m_pMenuViewRecip->addSeparator();
#endif

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

#if !defined NO_3D
	QAction *pResoEllipses3D = new QAction(this);
	pResoEllipses3D->setText("3D Ellipsoids...");
	pMenuReso->addAction(pResoEllipses3D);
#endif

	pMenuReso->addSeparator();

	QAction *pResoConv = new QAction(this);
	pResoConv->setText("Convolution...");
	pMenuReso->addAction(pResoConv);


	// --------------------------------------------------------------------------------
	// calc menu
	QMenu *pMenuCalc = new QMenu(this);
	pMenuCalc->setTitle("Calculation");

	QAction *pNeutronProps = new QAction(this);
	pNeutronProps->setText("Neutron Properties...");
	pMenuCalc->addAction(pNeutronProps);

	QAction *pPowder = new QAction(this);
	pPowder->setText("Powder Lines...");
	pMenuCalc->addAction(pPowder);

	QAction *pDW = new QAction(this);
	pDW->setText("Scattering Factors...");
	pMenuCalc->addAction(pDW);

	pMenuCalc->addSeparator();

	QAction *pDynPlane = new QAction(this);
	pDynPlane->setText("Kinematic Plane...");
	pMenuCalc->addAction(pDynPlane);

	QAction *pSpuri = new QAction(this);
	pSpuri->setText("Spurious Scattering...");
	pMenuCalc->addAction(pSpuri);


#if !defined NO_NET
	// --------------------------------------------------------------------------------
	// network menu
	QMenu *pMenuNet = new QMenu(this);
	pMenuNet->setTitle("Network");

	QAction *pConn = new QAction(this);
	pConn->setText("Connect to Instrument...");
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
#endif


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
#if !defined NO_NET
	pMenuBar->addMenu(pMenuNet);
#endif
	pMenuBar->addMenu(pMenuHelp);



	QObject::connect(pLoad, SIGNAL(triggered()), this, SLOT(Load()));
	QObject::connect(pSave, SIGNAL(triggered()), this, SLOT(Save()));
	QObject::connect(pSaveAs, SIGNAL(triggered()), this, SLOT(SaveAs()));
	QObject::connect(pImport, SIGNAL(triggered()), this, SLOT(Import()));
	QObject::connect(pScanViewer, SIGNAL(triggered()), this, SLOT(ShowScanViewer()));
	QObject::connect(pSettings, SIGNAL(triggered()), this, SLOT(ShowSettingsDlg()));
	QObject::connect(pExit, SIGNAL(triggered()), this, SLOT(close()));

	QObject::connect(m_pSmallq, SIGNAL(toggled(bool)), this, SLOT(EnableSmallq(bool)));
	QObject::connect(m_pBZ, SIGNAL(toggled(bool)), this, SLOT(EnableBZ(bool)));
	QObject::connect(m_pShowRealQDir, SIGNAL(toggled(bool)), this, SLOT(EnableRealQDir(bool)));

	QObject::connect(m_pSnapSmallq, SIGNAL(toggled(bool)), &m_sceneRecip, SLOT(setSnapq(bool)));
	QObject::connect(pKeepAbsKiKf, SIGNAL(toggled(bool)), &m_sceneRecip, SLOT(setKeepAbsKiKf(bool)));

	QObject::connect(pRecipParams, SIGNAL(triggered()), this, SLOT(ShowRecipParams()));
	QObject::connect(pRealParams, SIGNAL(triggered()), this, SLOT(ShowRealParams()));

#if !defined NO_3D
	QObject::connect(pView3D, SIGNAL(triggered()), this, SLOT(Show3D()));
	QObject::connect(pResoEllipses3D, SIGNAL(triggered()), this, SLOT(ShowResoEllipses3D()));
#endif

	QObject::connect(pRecipExport, SIGNAL(triggered()), this, SLOT(ExportRecip()));
	QObject::connect(pRealExport, SIGNAL(triggered()), this, SLOT(ExportReal()));

	QObject::connect(pResoParams, SIGNAL(triggered()), this, SLOT(ShowResoParams()));
	QObject::connect(pResoEllipses, SIGNAL(triggered()), this, SLOT(ShowResoEllipses()));
	QObject::connect(pResoConv, SIGNAL(triggered()), this, SLOT(ShowResoConv()));

	QObject::connect(pNeutronProps, SIGNAL(triggered()), this, SLOT(ShowNeutronDlg()));
	QObject::connect(m_pGoto, SIGNAL(triggered()), this, SLOT(ShowGotoDlg()));
	QObject::connect(pPowder, SIGNAL(triggered()), this, SLOT(ShowPowderDlg()));
	QObject::connect(pSpuri, SIGNAL(triggered()), this, SLOT(ShowSpurions()));
	QObject::connect(pDW, SIGNAL(triggered()), this, SLOT(ShowDWDlg()));
	QObject::connect(pDynPlane, SIGNAL(triggered()), this, SLOT(ShowDynPlaneDlg()));

#if !defined NO_NET
	QObject::connect(pConn, SIGNAL(triggered()), this, SLOT(ShowConnectDlg()));
	QObject::connect(pDisconn, SIGNAL(triggered()), this, SLOT(Disconnect()));
	QObject::connect(pNetRefresh, SIGNAL(triggered()), this, SLOT(NetRefresh()));
	QObject::connect(pNetCache, SIGNAL(triggered()), this, SLOT(ShowNetCache()));
#endif

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
	pFileTools->addAction(pImport);
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

#if !defined NO_NET
	QToolBar *pNetTools = new QToolBar(this);
	pNetTools->setWindowTitle("Network");
	pNetTools->addAction(pConn);
	pNetTools->addAction(pDisconn);
	pNetTools->addAction(pNetRefresh);
	addToolBar(pNetTools);
#endif

	// --------------------------------------------------------------------------------


	RepopulateSpaceGroups();

	m_sceneRecip.GetTriangle()->SetMaxPeaks(s_iMaxPeaks);
	m_sceneRecip.GetTriangle()->SetPlaneDistTolerance(s_dPlaneDistTolerance);

#if !defined NO_3D
	if(m_pRecip3d)
		m_pRecip3d->SetPlaneDistTolerance(s_dPlaneDistTolerance);
#endif

	UpdateDs();
	CalcPeaks();

	m_sceneRecip.GetTriangle()->SetqVisible(bSmallqVisible);
	m_sceneRecip.GetTriangle()->SetBZVisible(bBZVisible);
	m_sceneRecip.emitUpdate();
	//m_sceneRecip.emitAllParams();

	m_pviewReal->centerOn(m_sceneReal.GetTasLayout());
	m_pviewRecip->centerOn(m_sceneRecip.GetTriangle());
}

TazDlg::~TazDlg()
{
	//log_debug("In ", __func__, ".");

	//m_settings.setValue("main/width", this->width());
	//m_settings.setValue("main/height", this->height());
	m_settings.setValue("main/geo", saveGeometry());

	if(m_pviewRecip) { delete m_pviewRecip; m_pviewRecip = 0; }
	if(m_pEllipseDlg) { delete m_pEllipseDlg; m_pEllipseDlg = 0; }
	if(m_pReso) { delete m_pReso; m_pReso = 0; }
	if(m_pConvoDlg) { delete m_pConvoDlg; m_pConvoDlg = 0; }
	if(m_pSpuri) { delete m_pSpuri; m_pSpuri = 0; }
	if(m_pNeutronDlg) { delete m_pNeutronDlg; m_pNeutronDlg = 0; }
	if(m_pGotoDlg) { delete m_pGotoDlg; m_pGotoDlg = 0; }
	if(m_pPowderDlg) { delete m_pPowderDlg; m_pPowderDlg = 0; }
	if(m_pSettingsDlg) { delete m_pSettingsDlg; m_pSettingsDlg = 0; }
	if(m_pDWDlg) { delete m_pDWDlg; m_pDWDlg = 0; }
	if(m_pDynPlaneDlg) { delete m_pDynPlaneDlg; m_pDynPlaneDlg = 0; }
	if(m_pScanViewer) { delete m_pScanViewer; m_pScanViewer = nullptr; }

#if !defined NO_3D
	if(m_pRecip3d) { delete m_pRecip3d; m_pRecip3d = 0; }
	if(m_pEllipseDlg3D) { delete m_pEllipseDlg3D; m_pEllipseDlg3D = 0; }
#endif

#if !defined NO_NET
	if(m_pSrvDlg) { delete m_pSrvDlg; m_pSrvDlg = 0; }
	if(m_pNetCacheDlg) { delete m_pNetCacheDlg; m_pNetCacheDlg = 0; }
	if(m_pNicosCache) { delete m_pNicosCache; m_pNicosCache = 0; }
#endif
}


void TazDlg::ShowNeutronDlg()
{
	if(!m_pNeutronDlg)
		m_pNeutronDlg = new NeutronDlg(this, &m_settings);

	m_pNeutronDlg->show();
	m_pNeutronDlg->activateWindow();
}

void TazDlg::InitGoto()
{
	if(!m_pGotoDlg)
		m_pGotoDlg = new GotoDlg(this, &m_settings);
}

void TazDlg::ShowGotoDlg()
{
	InitGoto();
	m_pGotoDlg->show();
	m_pGotoDlg->activateWindow();
}

void TazDlg::ShowPowderDlg()
{
	if(!m_pPowderDlg)
		m_pPowderDlg = new PowderDlg(this, &m_settings);

	m_pPowderDlg->show();
	m_pPowderDlg->activateWindow();
}

void TazDlg::ShowSettingsDlg()
{
	if(!m_pSettingsDlg)
		m_pSettingsDlg = new SettingsDlg(this, &m_settings);

	m_pSettingsDlg->show();
	m_pSettingsDlg->activateWindow();
}

void TazDlg::ShowDWDlg()
{
	if(!m_pDWDlg)
		m_pDWDlg = new DWDlg(this, &m_settings);

	m_pDWDlg->show();
	m_pDWDlg->activateWindow();
}

void TazDlg::ShowDynPlaneDlg()
{
	if(!m_pDynPlaneDlg)
	{
		m_pDynPlaneDlg = new DynPlaneDlg(this, &m_settings);
		QObject::connect(&m_sceneRecip, SIGNAL(paramsChanged(const RecipParams&)),
						m_pDynPlaneDlg, SLOT(RecipParamsChanged(const RecipParams&)));
		m_sceneRecip.emitAllParams();
	}

	m_pDynPlaneDlg->show();
	m_pDynPlaneDlg->activateWindow();
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

#if !defined NO_3D
void TazDlg::Show3D()
{
	if(!m_pRecip3d)
	{
		m_pRecip3d = new Recip3DDlg(this, &m_settings);

		double dTol = s_dPlaneDistTolerance;
		m_pRecip3d->SetPlaneDistTolerance(dTol);
	}

	if(!m_pRecip3d->isVisible())
		m_pRecip3d->show();
	m_pRecip3d->activateWindow();

	CalcPeaks();
}
#else
void TazDlg::Show3D() {}
#endif

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

void TazDlg::RecipCoordsChanged(double dh, double dk, double dl,
	bool bHasNearest, double dNearestH, double dNearestK, double dNearestL)
{
	std::ostringstream ostrPos;
	ostrPos << "(" << dh << ", " << dk << ", " << dl  << ") rlu";
	if(bHasNearest)
		ostrPos << ", in 1st BZ of ("
			<< dNearestH << ", " << dNearestK << ", " << dNearestL << ")";

	m_pCoordStatusMsg->setText(ostrPos.str().c_str());
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


	std::string strDir = tl::get_dir(strFile.toStdString());
	m_settings.setValue("main/last_dir_export", QString(strDir.c_str()));
}




//--------------------------------------------------------------------------------
// about dialog

#include <boost/config.hpp>
#include <boost/version.hpp>
#include <qwt_global.h>
#include "tlibs/version.h"

void TazDlg::ShowAbout()
{
	const std::wstring& _strRet = tl::get_spec_char_utf16("return");
	const std::wstring& _strBullet = tl::get_spec_char_utf16("bullet");
	const std::wstring& _strArrow = tl::get_spec_char_utf16("rightarrow");

	const QString strRet = QString::fromUtf16((ushort*)_strRet.c_str(), _strRet.length());
	const QString strBullet = QString::fromUtf16((ushort*)_strBullet.c_str(), _strBullet.length());
	const QString strArrow = QString::fromUtf16((ushort*)_strArrow.c_str(), _strArrow.length());


	QString strAbout;
	strAbout += "Takin version 0.9.3\n";
	strAbout += "Written by Tobias Weber, 2014-2015.";
	strAbout += "\n\n";

	strAbout += "Takin is free software: you can redistribute it and/or modify ";
	strAbout += "it under the terms of the GNU General Public License version 2 ";
	strAbout += "as published by the Free Software Foundation.\n\n";
	strAbout += "Takin is distributed in the hope that it will be useful, ";
	strAbout += "but WITHOUT ANY WARRANTY; without even the implied warranty of ";
	strAbout += "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the ";
	strAbout += "GNU General Public License for more details.\n\n";
	strAbout += "You should have received a copy of the GNU General Public License ";
	strAbout += "along with Takin.  If not, see <http://www.gnu.org/licenses/>.\n\n";

	strAbout += "Built with ";
	strAbout += QString(BOOST_COMPILER);
	strAbout += ".\n";

	strAbout += "Build date: ";
	strAbout += QString(__DATE__);
	strAbout += ", ";
	strAbout += QString(__TIME__);
	strAbout += ".\n\n";

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
	tl::find_all_and_replace<std::string>(strBoost, "_", ".");
	strAbout += strBoost.c_str();
	strAbout += "    \t" + strArrow + " http://www.boost.org\n";

#ifndef NO_LAPACK
	strAbout += strBullet + " ";
	strAbout += "Uses Lapack/e version 3";
	strAbout += "    \t" + strArrow + " http://www.netlib.org/lapack\n";
#endif

	strAbout += strBullet + " ";
	strAbout += "Uses TLIBS version " + QString(TLIBS_VERSION);
	strAbout += "\n";

	strAbout += "\n";
	strAbout += strBullet + " ";
	strAbout += "Uses space group calculations ported from Nicos version 2";
	strAbout += "\n\t " + strArrow + " https://forge.frm2.tum.de/redmine/projects/nicos\n";
	strAbout += "   which uses space group data adapted from PowderCell version 2.3";
	strAbout += "\n\t " + strArrow
			+ " www.bam.de/de/service/publikationen/" + strRet
			+ "\n\t\tpowder_cell_a.htm\n";

	strAbout += strBullet + " ";
	strAbout += "Uses resolution algorithms ported from Rescal version 5";
	strAbout += "\n\t " + strArrow
			+ " www.ill.eu/en/html/instruments-support/" + strRet
			+ "\n\t\tcomputing-for-science/cs-software/" + strRet
			+ "\n\t\tall-software/matlab-ill/rescal-for-matlab\n";

	strAbout += "\nSee the LICENSES file in the Takin root directory.";

	QMessageBox::information(this, "About Takin", strAbout);
}

#include "taz.moc"
