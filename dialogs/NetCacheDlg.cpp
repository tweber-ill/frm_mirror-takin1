/*
 * Cache dialog
 * @author Tobias Weber
 * @date 21-oct-2014
 * @license GPLv2
 */

#include "NetCacheDlg.h"
#include "tlibs/string/string.h"
#include "tlibs/helper/flags.h"
#include <chrono>
#include <iostream>

using t_real = t_real_glob;

NetCacheDlg::NetCacheDlg(QWidget* pParent, QSettings* pSett)
	: QDialog(pParent), m_pSettings(pSett)
{
	this->setupUi(this);
	if(m_pSettings)
	{
		QFont font;
		if(m_pSettings->contains("main/font_gen") && font.fromString(m_pSettings->value("main/font_gen", "").toString()))
			setFont(font);
	}

	tableCache->setColumnCount(4);
	tableCache->setRowCount(0);
	tableCache->setColumnWidth(0, 200);
	tableCache->setColumnWidth(1, 200);
	tableCache->setColumnWidth(2, 140);
	tableCache->setColumnWidth(3, 140);
	tableCache->verticalHeader()->setDefaultSectionSize(tableCache->verticalHeader()->minimumSectionSize()+2);

	tableCache->setHorizontalHeaderItem(0, new QTableWidgetItem("Name"));
	tableCache->setHorizontalHeaderItem(1, new QTableWidgetItem("Value"));
	tableCache->setHorizontalHeaderItem(2, new QTableWidgetItem("Time Stamp"));
	tableCache->setHorizontalHeaderItem(3, new QTableWidgetItem("Age"));

#if QT_VER >= 5
	QObject::connect(&m_timer, &QTimer::timeout, this, &NetCacheDlg::UpdateTimer);
#else
	QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(UpdateTimer()));
#endif
	m_timer.start(s_iTimer);


	if(m_pSettings && m_pSettings->contains("net_cache/geo"))
		restoreGeometry(m_pSettings->value("net_cache/geo").toByteArray());
}

NetCacheDlg::~NetCacheDlg()
{}

void NetCacheDlg::UpdateTimer()
{
	UpdateAge(-1);
}

void NetCacheDlg::hideEvent(QHideEvent *pEvt)
{
	m_timer.stop();
}

void NetCacheDlg::showEvent(QShowEvent *pEvt)
{
	m_timer.start(s_iTimer);
}

void NetCacheDlg::accept()
{
	if(m_pSettings)
		m_pSettings->setValue("net_cache/geo", saveGeometry());

	QDialog::accept();
}

void NetCacheDlg::UpdateValue(const std::string& strKey, const CacheVal& val)
{
	int iRow = 0;
	QTableWidgetItem *pItem = 0;
	QString qstrKey = strKey.c_str();
	QString qstrVal = val.strVal.c_str();
	QString qstrTimestamp = std::to_string(val.dTimestamp).c_str();

	for(iRow=0; iRow<tableCache->rowCount(); ++iRow)
	{
		if(tableCache->item(iRow, 0)->text() == qstrKey)
		{
			pItem = tableCache->item(iRow, 0);
			break;
		}
	}

	if(pItem) 	// update items
	{
		tableCache->item(iRow, 1)->setText(qstrVal);
		tableCache->item(iRow, 2)->setText(qstrTimestamp);
	}
	else		// insert new items
	{
		iRow = tableCache->rowCount();
		tableCache->setRowCount(iRow+1);
		tableCache->setVerticalHeaderItem(iRow, new QTableWidgetItem(qstrKey));
		tableCache->setItem(iRow, 1, new QTableWidgetItem(qstrVal));
		tableCache->setItem(iRow, 2, new QTableWidgetItem(qstrTimestamp));
		tableCache->setItem(iRow, 3, new QTableWidgetItem());
	}

	UpdateAge(iRow);
}

void NetCacheDlg::UpdateAll(const t_mapCacheVal& map)
{
	for(const t_mapCacheVal::value_type& pair : map)
	{
		const std::string& strKey = pair.first;
		const CacheVal& val = pair.second;

		UpdateValue(strKey, val);
	}
}

void NetCacheDlg::UpdateAge(int iRow)
{
	// update all
	if(iRow<0)
	{
		for(int iCurRow=0; iCurRow<tableCache->rowCount(); ++iCurRow)
			UpdateAge(iCurRow);

		return;
	}

	QTableWidgetItem *pItemTimestamp = tableCache->item(iRow, 1);
	QTableWidgetItem *pItem = tableCache->item(iRow, 2);
	if(!pItemTimestamp || !pItem) return;

	t_real dTimestamp = tl::str_to_var<t_real, std::string>(pItemTimestamp->text().toStdString());
	t_real dNow = std::chrono::system_clock::now().time_since_epoch().count();
	dNow *= t_real(std::chrono::system_clock::period::num) / t_real(std::chrono::system_clock::period::den);
	t_real dAgeS = dNow - dTimestamp;
	//std::cout << strKey << " = " << val.strVal << " (age: " << dAge << "s)" << std::endl;

	int iAgeS = int(dAgeS);
	int iAgeM = 0;
	int iAgeH = 0;
	int iAgeD = 0;

	if(iAgeS > 60)
	{
		iAgeM = iAgeS / 60;
		iAgeS = iAgeS % 60;
	}
	if(iAgeM > 60)
	{
		iAgeH = iAgeM / 60;
		iAgeM = iAgeM  % 60;
	}
	if(iAgeH > 24)
	{
		iAgeD = iAgeH / 24;
		iAgeH = iAgeH  % 24;
	}

	bool bHadPrev = 0;
	std::string strAge;
	if(iAgeD || bHadPrev) { strAge += std::to_string(iAgeD) + "d "; bHadPrev = 1; }
	if(iAgeH || bHadPrev) { strAge += std::to_string(iAgeH) + "h "; bHadPrev = 1; }
	if(iAgeM || bHadPrev) { strAge += std::to_string(iAgeM) + "m "; bHadPrev = 1; }
	/*if(iAgeS || bHadPrev)*/ { strAge += std::to_string(iAgeS) + "s "; bHadPrev = 1; }
	pItem->setText(strAge.c_str());
}

void NetCacheDlg::ClearAll()
{
	tableCache->clearContents();
	tableCache->setRowCount(0);
}

#include "NetCacheDlg.moc"
