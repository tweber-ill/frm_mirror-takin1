/*
 * Scan viewer
 * @author tweber
 * @date mar-2015
 * @copyright GPLv2
 */

#include "scanviewer.h"
#include <QtGui/QFileDialog>
#include <QtGui/QTableWidget>
#include <QtGui/QTableWidgetItem>
#include <iostream>
#include <set>
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
	
	m_pGrid = new QwtPlotGrid();
	QPen penGrid;
	penGrid.setColor(QColor(0x99,0x99,0x99));
	penGrid.setStyle(Qt::DashLine);
	m_pGrid->setPen(penGrid);
	m_pGrid->attach(plot);

	m_pZoomer = new QwtPlotZoomer(plot->canvas());
	m_pZoomer->setMaxStackDepth(-1);
	m_pZoomer->setEnabled(1);
	
	plot->canvas()->setMouseTracking(1);
	m_pPicker = new QwtPlotPicker(plot->xBottom, plot->yLeft,
#if QWT_VER<6
		QwtPlotPicker::PointSelection,
#endif
		QwtPlotPicker::NoRubberBand, QwtPlotPicker::AlwaysOn, plot->canvas());

	m_pPicker->setEnabled(1);
	// -------------------------------------------------------------------------


	// -------------------------------------------------------------------------
	// property map stuff
	tableProps->setColumnCount(2);
	tableProps->setColumnWidth(0, 150);
	tableProps->setColumnWidth(1, 350);
	//tableProps->sortByColumn(0);

	tableProps->setHorizontalHeaderItem(0, new QTableWidgetItem("Property"));
	tableProps->setHorizontalHeaderItem(1, new QTableWidgetItem("Value"));
	
	tableProps->verticalHeader()->setVisible(false);
	tableProps->verticalHeader()->setDefaultSectionSize(tableProps->verticalHeader()->minimumSectionSize()+4);
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
	QObject::connect(tableProps, SIGNAL(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)),
					this, SLOT(PropSelected(QTableWidgetItem*, QTableWidgetItem*)));

	QString strDir = m_settings.value("last_dir", fs::current_path().native().c_str()).toString();
	editPath->setText(strDir);

	m_bDoUpdate = 1;
	ChangedPath();
}

ScanViewerDlg::~ScanViewerDlg()
{
	ClearPlot();
	tableProps->setRowCount(0);
	
	if(m_pGrid) delete m_pGrid;
	
	if(m_pZoomer)
	{
		m_pZoomer->setEnabled(0);
		delete m_pZoomer;
	}
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

#if QWT_VER>=6
	m_pCurve->setRawSamples(m_vecX.data(), m_vecY.data(), m_vecY.size());
	m_pPoints->setRawSamples(m_vecX.data(), m_vecY.data(), m_vecY.size());
#elif QWT_VER<6
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

	ShowProps();
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

#if QWT_VER>=6
	m_pCurve->setRawSamples(m_vecX.data(), m_vecY.data(), m_vecY.size());
	m_pPoints->setRawSamples(m_vecX.data(), m_vecY.data(), m_vecY.size());
#elif QWT_VER<6
	m_pCurve->setRawData(m_vecX.data(), m_vecY.data(), m_vecY.size());
	m_pPoints->setRawData(m_vecX.data(), m_vecY.data(), m_vecY.size());
#endif

	plot->setAxisTitle(QwtPlot::xBottom, strX.c_str());
	plot->setAxisTitle(QwtPlot::yLeft, strY.c_str());

	auto minmaxX = std::minmax_element(m_vecX.begin(), m_vecX.end());
	auto minmaxY = std::minmax_element(m_vecY.begin(), m_vecY.end());

	QRectF rect;
	rect.setLeft(*minmaxX.first);
	rect.setRight(*minmaxX.second);
	rect.setBottom(*minmaxY.second);
	rect.setTop(*minmaxY.first);
	m_pZoomer->setZoomBase(rect);
	m_pZoomer->zoom(rect);
	
	plot->replot();
}

void ScanViewerDlg::PropSelected(QTableWidgetItem *pItem, QTableWidgetItem *pItemPrev)
{
	if(!pItem)
		m_strSelectedKey = "";

	for(int iItem=0; iItem<tableProps->rowCount(); ++iItem)
	{
		const QTableWidgetItem *pKey = tableProps->item(iItem, 0);
		const QTableWidgetItem *pVal = tableProps->item(iItem, 1);

		if(pKey==pItem || pVal==pItem)
		{
			m_strSelectedKey = pKey->text().toStdString();
			break;
		}
	}
}

void ScanViewerDlg::ShowProps()
{
	if(m_pInstr==nullptr || !m_bDoUpdate)
		return;

	const tl::FileInstr::t_mapParams& params = m_pInstr->GetAllParams();
	tableProps->setRowCount(params.size());

	unsigned int iItem = 0;
	for(const tl::FileInstr::t_mapParams::value_type& pair : params)
	{
		QTableWidgetItem *pItemKey = tableProps->item(iItem, 0);
		if(!pItemKey)
		{
			pItemKey = new QTableWidgetItem();
			tableProps->setItem(iItem, 0, pItemKey);
		}
		pItemKey->setText(pair.first.c_str());

		QTableWidgetItem* pItemVal = tableProps->item(iItem, 1);
		if(!pItemVal)
		{
			pItemVal = new QTableWidgetItem();
			tableProps->setItem(iItem, 1, pItemVal);
		}
		pItemVal->setText(pair.second.c_str());

		++iItem;
	}

	tableProps->sortItems(0, Qt::AscendingOrder);


	// retain previous selection
	bool bHasSelection = 0;
	for(int iItem=0; iItem<tableProps->rowCount(); ++iItem)
	{
		const QTableWidgetItem *pItem = tableProps->item(iItem, 0);
		if(!pItem) continue;

		if(pItem->text().toStdString() == m_strSelectedKey)
		{
			tableProps->selectRow(iItem);
			bHasSelection = 1;
			break;
		}
	}

	if(!bHasSelection)
		tableProps->selectRow(0);
}

void ScanViewerDlg::ChangedPath()
{
	listFiles->clear();
	ClearPlot();
	tableProps->setRowCount(0);

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

		std::set<fs::path> lst;
		std::copy_if(dir_begin, dir_end, std::insert_iterator<decltype(lst)>(lst, lst.end()),
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
