/*
 * Scan viewer
 * @author tweber
 * @date mar-2015
 * @copyright GPLv2
 */

#include "scanviewer.h"
#include <QtGui/QFileDialog>
#include <iostream>
#include <list>
#include <string>
#include <algorithm>
#include <iterator>
#include <boost/filesystem.hpp>
#include "tlibs/string/string.h"

namespace fs = boost::filesystem;

#ifndef QWT_VER
	#define QWT_VER 6
#endif


ScanViewerDlg::ScanViewerDlg(QWidget* pParent)
	: QDialog(pParent), m_settings("tobis_stuff", "scanviewer"),
		m_vecExts({	".dat", ".DAT", ".scn", ".SCN" })
{
	this->setupUi(this);
	splitter->setStretchFactor(0, 1);
	splitter->setStretchFactor(1, 2);


	// -------------------------------------------------------------------------
	// plot stuff
	m_pCurve = new QwtPlotCurve("Scan Curve");
	m_pPoints = new QwtPlotCurve("Scan Points");

	QPen penCurve;
	penCurve.setColor(QColor(0,0,0x99));
	penCurve.setWidth(2);
	m_pCurve->setPen(penCurve);
	m_pCurve->setStyle(QwtPlotCurve::CurveStyle::Lines);
	m_pCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	m_pCurve->attach(plot);

	QPen penPoints;
	penPoints.setColor(QColor(0xff,0,0));
	penPoints.setWidth(4);
	m_pPoints->setPen(penPoints);
	m_pPoints->setStyle(QwtPlotCurve::CurveStyle::Dots);
	m_pPoints->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	m_pPoints->attach(plot);
	// -------------------------------------------------------------------------


	QObject::connect(editPath, SIGNAL(textEdited(const QString&)),
					this, SLOT(ChangedPath()));
	//QObject::connect(listFiles, SIGNAL(itemSelectionChanged()),
	//				this, SLOT(FileSelected()));
	QObject::connect(listFiles, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
					this, SLOT(FileSelected(QListWidgetItem*, QListWidgetItem*)));
	QObject::connect(btnBrowse, SIGNAL(clicked(bool)),
					this, SLOT(SelectDir()));
	QObject::connect(comboX, SIGNAL(currentIndexChanged(const QString&)),
					this, SLOT(XAxisSelected(const QString&)));
	QObject::connect(comboY, SIGNAL(currentIndexChanged(const QString&)),
					this, SLOT(YAxisSelected(const QString&)));

	QString strDir = m_settings.value("last_dir", fs::current_path().native().c_str()).toString();
	editPath->setText(strDir);

	m_bDoUpdate = 1;
	ChangedPath();
}

ScanViewerDlg::~ScanViewerDlg()
{
	ClearPlot();
}

void ScanViewerDlg::ClearPlot()
{
	if(m_pInstr)
	{
		delete m_pInstr;
		m_pInstr = nullptr;
	}

	m_vecX.clear();
	m_vecY.clear();

#if QWT_VER==6
	m_pCurve->setRawSamples(m_vecX.data(), m_vecY.data(), m_vecY.size());
	m_pPoints->setRawSamples(m_vecX.data(), m_vecY.data(), m_vecY.size());
#elif QWT_VER==5
	m_pCurve->setRawData(m_vecX.data(), m_vecY.data(), m_vecY.size());
	m_pPoints->setRawData(m_vecX.data(), m_vecY.data(), m_vecY.size());
#endif

	plot->setAxisTitle(QwtPlot::xBottom, "");
	plot->setAxisTitle(QwtPlot::yLeft, "");

	comboX->clear();
	comboY->clear();

	plot->replot();
}

void ScanViewerDlg::SelectDir()
{
	QString strCurDir = (m_strCurDir==""?".":m_strCurDir.c_str());
	QString strDir = QFileDialog::getExistingDirectory(this, "Select directory",
						strCurDir, QFileDialog::ShowDirsOnly);
	if(strDir != "")
	{
		editPath->setText(strDir);
		ChangedPath();
	}
}

void ScanViewerDlg::XAxisSelected(const QString& strLab)
{ PlotScan(); }
void ScanViewerDlg::YAxisSelected(const QString& strLab)
{ PlotScan(); }

void ScanViewerDlg::FileSelected(QListWidgetItem *pItem, QListWidgetItem *pItemPrev)
{
	//QListWidgetItem *pItem = listFiles->currentItem();
	if(!pItem) return;

	m_strCurFile = pItem->text().toStdString();


	ClearPlot();
	std::string strFile = m_strCurDir + m_strCurFile;
	m_pInstr = tl::FileInstr::LoadInstr(strFile.c_str());
	if(!m_pInstr) return;

	std::vector<std::string> vecScanVars = m_pInstr->GetScannedVars();
	std::string strCntVar = m_pInstr->GetCountVar();

	int iIdxX=-1, iIdxY=-1, iCurIdx=0;
	const tl::FileInstr::t_vecColNames& vecColNames = m_pInstr->GetColNames();
	for(const tl::FileInstr::t_vecColNames::value_type& strCol : vecColNames)
	{
		comboX->addItem(strCol.c_str());
		comboY->addItem(strCol.c_str());

		if(vecScanVars.size() && vecScanVars[0]==strCol)
			iIdxX = iCurIdx;
		if(strCntVar==strCol)
			iIdxY = iCurIdx;

		++iCurIdx;
	}

	m_bDoUpdate = 0;
	comboX->setCurrentIndex(iIdxX);
	comboY->setCurrentIndex(iIdxY);
	m_bDoUpdate = 1;

	PlotScan();
}

void ScanViewerDlg::PlotScan()
{
	if(m_pInstr==nullptr || !m_bDoUpdate)
		return;

	std::string strX = comboX->currentText().toStdString();
	std::string strY = comboY->currentText().toStdString();

	m_vecX = m_pInstr->GetCol(strX.c_str());
	m_vecY = m_pInstr->GetCol(strY.c_str());

	if(m_vecX.size()==0 || m_vecY.size()==0)
		return;

#if QWT_VER==6
	m_pCurve->setRawSamples(m_vecX.data(), m_vecY.data(), m_vecY.size());
	m_pPoints->setRawSamples(m_vecX.data(), m_vecY.data(), m_vecY.size());
#elif QWT_VER==5
	m_pCurve->setRawData(m_vecX.data(), m_vecY.data(), m_vecY.size());
	m_pPoints->setRawData(m_vecX.data(), m_vecY.data(), m_vecY.size());
#endif

	plot->setAxisTitle(QwtPlot::xBottom, strX.c_str());
	plot->setAxisTitle(QwtPlot::yLeft, strY.c_str());

	plot->replot();
}

void ScanViewerDlg::ChangedPath()
{
	listFiles->clear();

	std::string strPath = editPath->text().toStdString();
	fs::path dir(strPath);
	if(fs::exists(dir) && fs::is_directory(dir))
	{
		m_strCurDir = dir.native();
		tl::trim(m_strCurDir);
		if(*(m_strCurDir.begin()+m_strCurDir.length()-1) != fs::path::preferred_separator)
			m_strCurDir += fs::path::preferred_separator;
		UpdateFileList();

		m_settings.setValue("last_dir", QString(m_strCurDir.c_str()));
	}
}

void ScanViewerDlg::UpdateFileList()
{
	listFiles->clear();

	try
	{
		fs::path dir(m_strCurDir);
		fs::directory_iterator dir_begin(dir), dir_end;

		std::list<fs::path> lst;
		std::copy_if(dir_begin, dir_end, std::back_insert_iterator<decltype(lst)>(lst),
			[this](const fs::path& p) -> bool
			{
				std::string strExt = p.extension().native();
				if(strExt == ".bz2" || strExt == ".gz" || strExt == ".z")
					strExt = "." + tl::get_fileext2(p.filename().native());
				
				if(this->m_vecExts.size() == 0)
					return true;
				return std::find(this->m_vecExts.begin(), this->m_vecExts.end(),
					strExt) != this->m_vecExts.end();
			});

		for(const fs::path& d : lst)
			listFiles->addItem(d.filename().native().c_str());
	}
	catch(const std::exception& ex)
	{}
}

#include "scanviewer.moc"
