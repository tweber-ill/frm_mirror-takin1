/*
 * TAS tool
 * @author tweber
 * @date feb-2014
 * @license GPLv2
 */

#include "taz.h"
#include "helper/globals.h"

#include <iostream>
#include <algorithm>
#include <vector>
#include <boost/algorithm/string.hpp>

#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollBar>

#include "tlibs/math/lattice.h"
#include "tlibs/string/spec_char.h"
#include "tlibs/string/string.h"
#include "tlibs/helper/flags.h"
#include "tlibs/file/recent.h"
#include "tlibs/log/log.h"


const std::string TazDlg::s_strTitle = "Takin";


TazDlg::TazDlg(QWidget* pParent)
		: QMainWindow(pParent),
		  m_settings("tobis_stuff", "takin"),
		  m_pSettingsDlg(new SettingsDlg(this, &m_settings)),
		  m_pStatusMsg(new QLabel(this)),
		  m_pCoordQStatusMsg(new QLabel(this)),
		  m_pCoordCursorStatusMsg(new QLabel(this)),
		  m_dlgRecipParam(this, &m_settings),
		  m_dlgRealParam(this, &m_settings),
		  m_pGotoDlg(new GotoDlg(this, &m_settings))
{
	//log_debug("In ", __func__, ".");

	const bool bSmallqVisible = 0;
	const bool bBZVisible = 1;
	const bool bWSVisible = 1;
	const bool bEwald = 1;

	this->setupUi(this);
	this->setWindowTitle(s_strTitle.c_str());
	btnAtoms->setEnabled(g_bHasScatlens);

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
	m_pCoordQStatusMsg->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	m_pCoordCursorStatusMsg->setFrameStyle(QFrame::Panel | QFrame::Sunken);

	QStatusBar *pStatusBar = new QStatusBar(this);
	pStatusBar->addWidget(m_pStatusMsg, 1);
	pStatusBar->addPermanentWidget(m_pCoordQStatusMsg, 0);
	pStatusBar->addPermanentWidget(m_pCoordCursorStatusMsg, 0);
	m_pCoordQStatusMsg->setMinimumWidth(350);
	m_pCoordQStatusMsg->setAlignment(Qt::AlignCenter);
	m_pCoordCursorStatusMsg->setMinimumWidth(325);
	m_pCoordCursorStatusMsg->setAlignment(Qt::AlignCenter);
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

		editRealX0, editRealX1, editRealX2,
		editRealY0, editRealY1, editRealY2,
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

		"plane/real_x0", "plane/real_x1", "plane/real_x2",
		"plane/real_y0", "plane/real_y1", "plane/real_y2",
	};
	m_vecEditNames_monoana =
	{
		"tas/mono_d", "tas/ana_d"
	};

	m_vecSpinBoxNamesSample = {"sample/phi", "sample/theta", "sample/psi"};
	m_vecCheckBoxNamesSenses = {"tas/sense_m", "tas/sense_s", "tas/sense_a"};


	m_pviewRecip = new ScatteringTriangleView(groupRecip);
	groupRecip->addTab(m_pviewRecip, "Reciprocal Lattice");

	m_pviewRealLattice = new LatticeView(groupReal);
	groupReal->addTab(m_pviewRealLattice, "Real Lattice");

	m_pviewReal = new TasLayoutView(groupReal);
	groupReal->addTab(m_pviewReal, "Instrument Layout");

	if(m_settings.contains("main/real_tab"))
		groupReal->setCurrentIndex(m_settings.value("main/real_tab").value<int>());


	m_pviewRecip->setScene(&m_sceneRecip);
	m_pviewRealLattice->setScene(&m_sceneRealLattice);
	m_pviewReal->setScene(&m_sceneReal);


	QObject::connect(&m_sceneRecip, SIGNAL(triangleChanged(const TriangleOptions&)),
					&m_sceneReal, SLOT(triangleChanged(const TriangleOptions&)));
	QObject::connect(&m_sceneReal, SIGNAL(tasChanged(const TriangleOptions&)),
					&m_sceneRecip, SLOT(tasChanged(const TriangleOptions&)));
	QObject::connect(&m_sceneRecip, SIGNAL(paramsChanged(const RecipParams&)),
					&m_sceneReal, SLOT(recipParamsChanged(const RecipParams&)));

	QObject::connect(m_pviewRecip, SIGNAL(scaleChanged(double)),
					&m_sceneRecip, SLOT(scaleChanged(double)));
	QObject::connect(m_pviewRealLattice, SIGNAL(scaleChanged(double)),
					&m_sceneRealLattice, SLOT(scaleChanged(double)));
	QObject::connect(m_pviewReal, SIGNAL(scaleChanged(double)),
					&m_sceneReal, SLOT(scaleChanged(double)));

	QObject::connect(&m_sceneRecip, SIGNAL(paramsChanged(const RecipParams&)),
					&m_dlgRecipParam, SLOT(paramsChanged(const RecipParams&)));
	QObject::connect(&m_sceneReal, SIGNAL(paramsChanged(const RealParams&)),
					&m_dlgRealParam, SLOT(paramsChanged(const RealParams&)));

	// cursor position
	QObject::connect(&m_sceneRecip, SIGNAL(coordsChanged(double, double, double, bool, double, double, double)),
					this, SLOT(RecipCoordsChanged(double, double, double, bool, double, double, double)));
	QObject::connect(&m_sceneRealLattice, SIGNAL(coordsChanged(double, double, double, bool, double, double, double)),
					this, SLOT(RealCoordsChanged(double, double, double, bool, double, double, double)));

	QObject::connect(&m_sceneRecip, SIGNAL(spurionInfo(const tl::ElasticSpurion&, const std::vector<tl::InelasticSpurion<double>>&, const std::vector<tl::InelasticSpurion<double>>&)),
					this, SLOT(spurionInfo(const tl::ElasticSpurion&, const std::vector<tl::InelasticSpurion<double>>&, const std::vector<tl::InelasticSpurion<double>>&)));

	QObject::connect(m_pGotoDlg, SIGNAL(vars_changed(const CrystalOptions&, const TriangleOptions&)),
					this, SLOT(VarsChanged(const CrystalOptions&, const TriangleOptions&)));
	QObject::connect(&m_sceneRecip, SIGNAL(paramsChanged(const RecipParams&)),
					m_pGotoDlg, SLOT(RecipParamsChanged(const RecipParams&)));

	QObject::connect(&m_sceneRecip, SIGNAL(paramsChanged(const RecipParams&)),
					this, SLOT(recipParamsChanged(const RecipParams&)));


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

	QObject::connect(btnAtoms, SIGNAL(clicked(bool)), this, SLOT(ShowAtomsDlg()));



	// --------------------------------------------------------------------------------
	// file menu
	QMenu *pMenuFile = new QMenu(this);
	pMenuFile->setTitle("File");

	QAction *pNew = new QAction(this);
	pNew->setText("New");
	pNew->setIcon(load_icon("res/document-new.svg"));
	pMenuFile->addAction(pNew);

	pMenuFile->addSeparator();

	QAction *pLoad = new QAction(this);
	pLoad->setText("Load...");
	pLoad->setIcon(load_icon("res/document-open.svg"));
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
	pSave->setIcon(load_icon("res/document-save.svg"));
	pMenuFile->addAction(pSave);

	QAction *pSaveAs = new QAction(this);
	pSaveAs->setText("Save as...");
	pSaveAs->setIcon(load_icon("res/document-save-as.svg"));
	pMenuFile->addAction(pSaveAs);

	pMenuFile->addSeparator();

	QAction *pImport = new QAction(this);
	pImport->setText("Import...");
	pImport->setIcon(load_icon("res/drive-harddisk.svg"));
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

	QAction *pSettings = new QAction(this);
	pSettings->setText("Settings...");
	pSettings->setIcon(load_icon("res/preferences-system.svg"));
	pMenuFile->addAction(pSettings);

	pMenuFile->addSeparator();

	QAction *pExit = new QAction(this);
	pExit->setText("Exit");
	pExit->setIcon(load_icon("res/system-log-out.svg"));
	pMenuFile->addAction(pExit);


	// --------------------------------------------------------------------------------
	// recip menu
	m_pMenuViewRecip = new QMenu(this);
	m_pMenuViewRecip->setTitle("Reciprocal Space");

	m_pGoto = new QAction(this);
	m_pGoto->setText("Go to Position...");
	m_pGoto->setIcon(load_icon("res/goto.svg"));
	m_pMenuViewRecip->addAction(m_pGoto);

	QAction *pRecipParams = new QAction(this);
	pRecipParams->setText("Parameters...");
	m_pMenuViewRecip->addAction(pRecipParams);
	m_pMenuViewRecip->addSeparator();

	m_pSmallq = new QAction(this);
	m_pSmallq->setText("Show Reduced Scattering Vector q");
	m_pSmallq->setIcon(load_icon("res/q.svg"));
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
	m_pBZ->setIcon(load_icon("res/brillouin.svg"));
	m_pBZ->setCheckable(1);
	m_pBZ->setChecked(bBZVisible);
	m_pMenuViewRecip->addAction(m_pBZ);

	m_pEwaldSphere = new QAction(this);
	m_pEwaldSphere->setText("Show Ewald Sphere");
	//m_pEwaldSphere->setIcon(load_icon("res/brillouin.svg"));
	m_pEwaldSphere->setCheckable(1);
	m_pEwaldSphere->setChecked(bEwald);
	m_pMenuViewRecip->addAction(m_pEwaldSphere);

	m_pMenuViewRecip->addSeparator();

#if !defined NO_3D
	QAction *pView3D = new QAction(this);
	pView3D->setText("3D View...");
	//pView3D->setIcon(QIcon::fromTheme("applications-graphics"));
	m_pMenuViewRecip->addAction(pView3D);

	m_pMenuViewRecip->addSeparator();
#endif

	QAction *pRecipExport = new QAction(this);
	pRecipExport->setText("Export Lattice Graphics...");
	pRecipExport->setIcon(load_icon("res/image-x-generic.svg"));
	m_pMenuViewRecip->addAction(pRecipExport);

#ifdef USE_GIL
	QAction *pBZExport = new QAction(this);
	pBZExport->setText("Export Brillouin Zone Image...");
	pBZExport->setIcon(load_icon("res/image-x-generic.svg"));
	m_pMenuViewRecip->addAction(pBZExport);
#endif


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

	m_pWS = new QAction(this);
	m_pWS->setText("Show Wigner-Seitz Cell");
	m_pWS->setIcon(load_icon("res/brillouin.svg"));
	m_pWS->setCheckable(1);
	m_pWS->setChecked(bWSVisible);
	m_pMenuViewReal->addAction(m_pWS);

	m_pMenuViewReal->addSeparator();

	QAction *pRealLatticeExport = new QAction(this);
	pRealLatticeExport->setText("Export Lattice Graphics...");
	pRealLatticeExport->setIcon(load_icon("res/image-x-generic.svg"));
	m_pMenuViewReal->addAction(pRealLatticeExport);

	QAction *pRealExport = new QAction(this);
	pRealExport->setText("Export Instrument Graphics...");
	pRealExport->setIcon(load_icon("res/image-x-generic.svg"));
	m_pMenuViewReal->addAction(pRealExport);

#ifdef USE_GIL
	QAction *pWSExport = new QAction(this);
	pWSExport->setText("Export Wigner-Seitz Cell Image...");
	pWSExport->setIcon(load_icon("res/image-x-generic.svg"));
	m_pMenuViewReal->addAction(pWSExport);
#endif

	QAction *pExportUC = new QAction(this);
	pExportUC->setText("Export Unit Cell Model...");
	pExportUC->setIcon(load_icon("res/image-x-generic.svg"));
	m_pMenuViewReal->addAction(pExportUC);


	// --------------------------------------------------------------------------------
	// resolution menu
	QMenu *pMenuReso = new QMenu(this);
	pMenuReso->setTitle("Resolution");

	QAction *pResoParams = new QAction(this);
	pResoParams->setText("Parameters...");
	pResoParams->setIcon(load_icon("res/accessories-calculator.svg"));
	pMenuReso->addAction(pResoParams);

	pMenuReso->addSeparator();

	QAction *pResoEllipses = new QAction(this);
	pResoEllipses->setText("Ellipses...");
	pResoEllipses->setIcon(load_icon("res/ellipses.svg"));
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
	pNeutronProps->setIcon(load_icon("res/x-office-spreadsheet-template.svg"));
	pMenuCalc->addAction(pNeutronProps);

	pMenuCalc->addSeparator();

	QAction *pPowder = new QAction(this);
	pPowder->setText("Powder Lines...");
	pPowder->setIcon(load_icon("res/weather-snow.svg"));
	pMenuCalc->addAction(pPowder);

	QAction *pDW = new QAction(this);
	pDW->setText("Scattering Factors...");
	pMenuCalc->addAction(pDW);

	QAction *pFormfactor = nullptr;
	if(g_bHasFormfacts && g_bHasScatlens)
	{
		pFormfactor = new QAction(this);
		pFormfactor->setText("Elements...");
		pMenuCalc->addAction(pFormfactor);
	}

	QAction *pSgList = new QAction(this);
	pSgList->setText("Space Group Types...");
	pMenuCalc->addAction(pSgList);

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
	pConn->setIcon(load_icon("res/network-transmit-receive.svg"));
	pMenuNet->addAction(pConn);

	QAction *pDisconn = new QAction(this);
	pDisconn->setText("Disconnect");
	pDisconn->setIcon(load_icon("res/network-offline.svg"));
	pMenuNet->addAction(pDisconn);

	QAction *pNetCache = new QAction(this);
	pNetCache->setText("Show Network Cache...");
	pMenuNet->addSeparator();
	pMenuNet->addAction(pNetCache);

	QAction *pNetRefresh = new QAction(this);
	pNetRefresh->setText("Refresh");
	pNetRefresh->setIcon(load_icon("res/view-refresh.svg"));
	pMenuNet->addSeparator();
	pMenuNet->addAction(pNetRefresh);
#endif


	// --------------------------------------------------------------------------------
	// tools menu

	QMenu *pMenuTools = new QMenu(this);
	pMenuTools->setTitle("Tools");

	QAction *pScanViewer = new QAction(this);
	pScanViewer->setText("Scan Viewer...");
	pMenuTools->addAction(pScanViewer);



	// --------------------------------------------------------------------------------
	// help menu
	QMenu *pMenuHelp = new QMenu(this);
	pMenuHelp->setTitle("Help");

	QAction *pAboutQt = new QAction(this);
	pAboutQt->setText("About Qt...");
	//pAboutQt->setIcon(QIcon::fromTheme("help-about"));
	pMenuHelp->addAction(pAboutQt);

	//pMenuHelp->addSeparator();
	QAction *pAbout = new QAction(this);
	pAbout->setText("About Takin...");
	pAbout->setIcon(load_icon("res/dialog-information.svg"));
	pMenuHelp->addAction(pAbout);



	// --------------------------------------------------------------------------------
	QMenuBar *pMenuBar = new QMenuBar(this);
	pMenuBar->addMenu(pMenuFile);
	pMenuBar->addMenu(m_pMenuViewRecip);
	pMenuBar->addMenu(m_pMenuViewReal);
	pMenuBar->addMenu(pMenuReso);
	pMenuBar->addMenu(pMenuCalc);
	pMenuBar->addMenu(pMenuTools);
#if !defined NO_NET
	pMenuBar->addMenu(pMenuNet);
#endif
	pMenuBar->addMenu(pMenuHelp);


	QObject::connect(pNew, SIGNAL(triggered()), this, SLOT(New()));
	QObject::connect(pLoad, SIGNAL(triggered()), this, SLOT(Load()));
	QObject::connect(pSave, SIGNAL(triggered()), this, SLOT(Save()));
	QObject::connect(pSaveAs, SIGNAL(triggered()), this, SLOT(SaveAs()));
	QObject::connect(pImport, SIGNAL(triggered()), this, SLOT(Import()));
	QObject::connect(pScanViewer, SIGNAL(triggered()), this, SLOT(ShowScanViewer()));
	QObject::connect(pSettings, SIGNAL(triggered()), this, SLOT(ShowSettingsDlg()));
	QObject::connect(pExit, SIGNAL(triggered()), this, SLOT(close()));

	QObject::connect(m_pSmallq, SIGNAL(toggled(bool)), this, SLOT(EnableSmallq(bool)));
	QObject::connect(m_pBZ, SIGNAL(toggled(bool)), this, SLOT(EnableBZ(bool)));
	QObject::connect(m_pWS, SIGNAL(toggled(bool)), this, SLOT(EnableWS(bool)));
	QObject::connect(m_pEwaldSphere, SIGNAL(toggled(bool)), this, SLOT(ShowEwaldSphere(bool)));
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
	QObject::connect(pRealLatticeExport, SIGNAL(triggered()), this, SLOT(ExportRealLattice()));

	QObject::connect(pExportUC, SIGNAL(triggered()), this, SLOT(ExportUCModel()));

#ifdef USE_GIL
	QObject::connect(pBZExport, SIGNAL(triggered()), this, SLOT(ExportBZImage()));
	QObject::connect(pWSExport, SIGNAL(triggered()), this, SLOT(ExportWSImage()));
#endif

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

	QObject::connect(pSgList, SIGNAL(triggered()), this, SLOT(ShowSgListDlg()));

	if(pFormfactor)
		QObject::connect(pFormfactor, SIGNAL(triggered()), this, SLOT(ShowFormfactorDlg()));

	QObject::connect(pAbout, SIGNAL(triggered()), this, SLOT(ShowAbout()));
	QObject::connect(pAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));


	setMenuBar(pMenuBar);
	// --------------------------------------------------------------------------------


	// --------------------------------------------------------------------------------
	// context menus
	m_pviewRecip->setContextMenuPolicy(Qt::CustomContextMenu);
	m_pviewRealLattice->setContextMenuPolicy(Qt::CustomContextMenu);
	m_pviewReal->setContextMenuPolicy(Qt::CustomContextMenu);

	QObject::connect(m_pviewRecip, SIGNAL(customContextMenuRequested(const QPoint&)),
					this, SLOT(RecipContextMenu(const QPoint&)));
	QObject::connect(m_pviewRealLattice, SIGNAL(customContextMenuRequested(const QPoint&)),
					this, SLOT(RealContextMenu(const QPoint&)));
	QObject::connect(m_pviewReal, SIGNAL(customContextMenuRequested(const QPoint&)),
					this, SLOT(RealContextMenu(const QPoint&)));


	// --------------------------------------------------------------------------------
	// tool bars
	QToolBar *pFileTools = new QToolBar(this);
	pFileTools->setWindowTitle("File");
	pFileTools->addAction(pNew);
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
	//pRecipTools->addAction(m_pEwaldSphere);
	addToolBar(pRecipTools);

	QToolBar *pResoTools = new QToolBar(this);
	pResoTools->setWindowTitle("Resolution");
	pResoTools->addAction(pResoParams);
	pResoTools->addAction(pResoEllipses);
	addToolBar(pResoTools);

	QToolBar *pCalcTools = new QToolBar(this);
	pCalcTools->setWindowTitle("Calculation");
	pCalcTools->addAction(pNeutronProps);
	pCalcTools->addAction(pPowder);
	addToolBar(pCalcTools);

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

	unsigned int iMaxPeaks = m_settings.value("main/max_peaks", 10).toUInt();

	m_sceneRecip.GetTriangle()->SetMaxPeaks(iMaxPeaks);
	m_sceneRecip.GetTriangle()->SetPlaneDistTolerance(s_dPlaneDistTolerance);

	m_sceneRealLattice.GetLattice()->SetMaxPeaks(iMaxPeaks);
	m_sceneRealLattice.GetLattice()->SetPlaneDistTolerance(s_dPlaneDistTolerance);

#if !defined NO_3D
	if(m_pRecip3d)
	{
		m_pRecip3d->SetMaxPeaks((double)iMaxPeaks);
		m_pRecip3d->SetPlaneDistTolerance(s_dPlaneDistTolerance);
	}
#endif

	m_bReady = 1;
	UpdateDs();
	CalcPeaks();


	m_sceneRecip.GetTriangle()->SetqVisible(bSmallqVisible);
	m_sceneRecip.GetTriangle()->SetBZVisible(bBZVisible);
	m_sceneRecip.GetTriangle()->SetEwaldSphereVisible(bEwald);
	m_sceneRealLattice.GetLattice()->SetWSVisible(bWSVisible);
	m_sceneRecip.emitUpdate();
	//m_sceneRecip.emitAllParams();
}

TazDlg::~TazDlg()
{
	//log_debug("In ", __func__, ".");
	DeleteDialogs();

	// don't delete non-optional sub-modules in DeleteDialogs()
	if(m_pGotoDlg) { delete m_pGotoDlg; m_pGotoDlg = 0; }
	if(m_pSettingsDlg) { delete m_pSettingsDlg; m_pSettingsDlg = 0; }

	if(m_pviewRecip) { delete m_pviewRecip; m_pviewRecip = 0; }
	if(m_pviewRealLattice) { delete m_pviewRealLattice; m_pviewRealLattice = 0; }
	if(m_pviewReal) { delete m_pviewReal; m_pviewReal = 0; }

	comboSpaceGroups->clear();
}

void TazDlg::DeleteDialogs()
{
	if(m_pAboutDlg) { delete m_pAboutDlg; m_pAboutDlg = 0; }
	if(m_pEllipseDlg) { delete m_pEllipseDlg; m_pEllipseDlg = 0; }
	if(m_pReso) { delete m_pReso; m_pReso = 0; }
	if(m_pConvoDlg) { delete m_pConvoDlg; m_pConvoDlg = 0; }
	if(m_pSpuri) { delete m_pSpuri; m_pSpuri = 0; }
	if(m_pNeutronDlg) { delete m_pNeutronDlg; m_pNeutronDlg = 0; }
	if(m_pPowderDlg) { delete m_pPowderDlg; m_pPowderDlg = 0; }
	if(m_pDWDlg) { delete m_pDWDlg; m_pDWDlg = 0; }
	if(m_pDynPlaneDlg) { delete m_pDynPlaneDlg; m_pDynPlaneDlg = 0; }
	if(m_pScanViewer) { delete m_pScanViewer; m_pScanViewer = nullptr; }
	if(m_pAtomsDlg) { delete m_pAtomsDlg; m_pAtomsDlg = nullptr; }

#if !defined NO_3D
	if(m_pRecip3d) { delete m_pRecip3d; m_pRecip3d = 0; }
	if(m_pEllipseDlg3D) { delete m_pEllipseDlg3D; m_pEllipseDlg3D = 0; }
#endif

#if !defined NO_NET
	if(m_pSrvDlg) { delete m_pSrvDlg; m_pSrvDlg = 0; }
	if(m_pNetCacheDlg) { delete m_pNetCacheDlg; m_pNetCacheDlg = 0; }
	if(m_pNicosCache) { delete m_pNicosCache; m_pNicosCache = 0; }
	if(m_pSicsCache) { delete m_pSicsCache; m_pSicsCache = 0; }
#endif

	if(m_pSgListDlg) { delete m_pSgListDlg; m_pSgListDlg = 0; }
	if(m_pFormfactorDlg) { delete m_pFormfactorDlg; m_pFormfactorDlg = 0; }
}


void TazDlg::showEvent(QShowEvent *pEvt)
{
	QMainWindow::showEvent(pEvt);

	static bool bInitialShow = 1;
	if(bInitialShow)
	{
		bInitialShow = 0;

		m_pviewReal->centerOn(m_sceneReal.GetTasLayout());
		m_pviewRecip->centerOn(m_sceneRecip.GetTriangle()->GetGfxMid());
		m_pviewRealLattice->centerOn(0.,0.);

		/*for(QScrollBar* pSB : {
			m_pviewRealLattice->horizontalScrollBar(),
			m_pviewRealLattice->verticalScrollBar(),
			m_pviewReal->horizontalScrollBar(),
			m_pviewReal->verticalScrollBar(),
			m_pviewRecip->horizontalScrollBar(),
			m_pviewRecip->verticalScrollBar() })
			pSB->setValue(pSB->minimum() + (pSB->maximum()-pSB->minimum())/2);*/
	}
}

void TazDlg::closeEvent(QCloseEvent* pEvt)
{
	//m_settings.setValue("main/width", this->width());
	//m_settings.setValue("main/height", this->height());
	m_settings.setValue("main/geo", saveGeometry());
	m_settings.setValue("main/real_tab", groupReal->currentIndex());

	QMainWindow::closeEvent(pEvt);
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

void TazDlg::EnableWS(bool bEnable)
{
	m_sceneRealLattice.GetLattice()->SetWSVisible(bEnable);
}

void TazDlg::ShowEwaldSphere(bool bEnable)
{
	m_sceneRecip.GetTriangle()->SetEwaldSphereVisible(bEnable);
}

void TazDlg::EnableRealQDir(bool bEnable)
{
	m_sceneReal.GetTasLayout()->SetRealQVisible(bEnable);
}

// Q position
void TazDlg::recipParamsChanged(const RecipParams& params)
{
	double dQx = -params.Q_rlu[0], dQy = -params.Q_rlu[1], dQz = -params.Q_rlu[2];
	double dE = params.dE;

	tl::set_eps_0(dQx); tl::set_eps_0(dQy); tl::set_eps_0(dQz);
	tl::set_eps_0(dE);

	std::ostringstream ostrPos;
	ostrPos.precision(g_iPrecGfx);
	ostrPos << "Q = (" << dQx << ", " << dQy << ", " << dQz  << ") rlu";
	ostrPos << ", E = " << dE << " meV";

	ostrPos << ", BZ: ("
		<< params.G_rlu_accurate[0] << ", "
		<< params.G_rlu_accurate[1] << ", "
		<< params.G_rlu_accurate[2] << ")";

	m_pCoordQStatusMsg->setText(ostrPos.str().c_str());
}

// cursor position
void TazDlg::RecipCoordsChanged(double dh, double dk, double dl,
	bool bHasNearest, double dNearestH, double dNearestK, double dNearestL)
{
	tl::set_eps_0(dh); tl::set_eps_0(dk); tl::set_eps_0(dl);
	tl::set_eps_0(dNearestH); tl::set_eps_0(dNearestK); tl::set_eps_0(dNearestL);

	std::ostringstream ostrPos;
	ostrPos.precision(g_iPrecGfx);
	ostrPos << "Cur: (" << dh << ", " << dk << ", " << dl  << ") rlu";
	if(bHasNearest)
		ostrPos << ", BZ: ("
			<< dNearestH << ", " << dNearestK << ", " << dNearestL << ")";

	m_pCoordCursorStatusMsg->setText(ostrPos.str().c_str());
}

// cursor position
void TazDlg::RealCoordsChanged(double dh, double dk, double dl,
	bool bHasNearest, double dNearestH, double dNearestK, double dNearestL)
{
	tl::set_eps_0(dh); tl::set_eps_0(dk); tl::set_eps_0(dl);
	tl::set_eps_0(dNearestH); tl::set_eps_0(dNearestK); tl::set_eps_0(dNearestL);

	std::ostringstream ostrPos;
	ostrPos.precision(g_iPrecGfx);
	ostrPos << "Cur: (" << dh << ", " << dk << ", " << dl  << ") frac";
	if(bHasNearest)
		ostrPos << ", WS: ("
			<< dNearestH << ", " << dNearestK << ", " << dNearestL << ")";

	m_pCoordCursorStatusMsg->setText(ostrPos.str().c_str());
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
// about dialog

void TazDlg::ShowAbout()
{
	if(!m_pAboutDlg)
		m_pAboutDlg = new AboutDlg(this, &m_settings);

	m_pAboutDlg->show();
	m_pAboutDlg->activateWindow();
}

#include "taz.moc"
