/*
 * TAS tool (server stuff)
 * @author tweber
 * @date feb-2014
 * @copyright GPLv2
 */

#include "taz.h"

#include <QtGui/QStatusBar>
#include <QtGui/QMessageBox>

#define DEFAULT_MSG_TIMEOUT 4000

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

	m_pNicosCache = new NicosCache(&m_settings);
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
		
	//if(triag.bChangedTwoTheta)
	//	log_info("2theta: ", triag.dTwoTheta/M_PI*180.);

	m_sceneReal.triangleChanged(triag);
	m_sceneReal.emitUpdate(triag);
	//m_sceneReal.emitAllParams();

	UpdateMonoSense();
	UpdateAnaSense();
	UpdateSampleSense();

	if(triag.bChangedAngleKiVec0)
	{
		m_sceneRecip.tasChanged(triag);
		m_sceneRecip.emitUpdate();
		//m_sceneRecip.emitAllParams();
	}
}
