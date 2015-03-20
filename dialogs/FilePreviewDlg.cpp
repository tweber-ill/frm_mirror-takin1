/*
 * File Preview Dialog
 * @author Tobias Weber
 * @date feb-2015
 * @copyright GPLv2
 */

#include "FilePreviewDlg.h"
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include "tlibs/file/loadinstr.h"
#include <iostream>
#include <memory>


FilePreviewDlg::FilePreviewDlg(QWidget* pParent, const char* pcTitle)
	: QFileDialog(pParent, pcTitle)
{
	m_pPlot = new QwtPlot();
	m_pCurve = new QwtPlotCurve("Scan Curve");
	m_pPoints = new QwtPlotCurve("Scan Points");

	QPen penCurve;
	penCurve.setColor(QColor(0,0,0x99));
	penCurve.setWidth(2);
	m_pCurve->setPen(penCurve);
	m_pCurve->setStyle(QwtPlotCurve::CurveStyle::Lines);
	m_pCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	m_pCurve->attach(m_pPlot);

	QPen penPoints;
	penPoints.setColor(QColor(0xff,0,0));
	penPoints.setWidth(4);
	m_pPoints->setPen(penPoints);
	m_pPoints->setStyle(QwtPlotCurve::CurveStyle::Dots);
	m_pPoints->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	m_pPoints->attach(m_pPlot);

	QSizePolicy spol(QSizePolicy::Preferred, QSizePolicy::Preferred);
	spol.setHorizontalStretch(1);
	spol.setVerticalStretch(1);
	m_pPlot->setSizePolicy(spol);

	// depends on qt/src/gui/dialogs/qfiledialog.ui using QGridLayout
	QGridLayout *pLayout((QGridLayout*)layout());
	pLayout->addWidget(m_pPlot, pLayout->rowCount(), 0, 1, pLayout->columnCount());
	resize(size().width(), size().height()*1.25);

	QObject::connect(this, SIGNAL(currentChanged(const QString&)),
					this, SLOT(FileSelected(const QString&)));
}

FilePreviewDlg::~FilePreviewDlg()
{
	if(m_pPlot) { delete m_pPlot; m_pPlot = nullptr; }
}

void FilePreviewDlg::ClearPlot()
{
	m_vecCts.clear();
	m_vecScn.clear();
#ifdef USE_QWT6
	m_pCurve->setRawSamples(m_vecScn.data(), m_vecCts.data(), m_vecCts.size());
	m_pPoints->setRawSamples(m_vecScn.data(), m_vecCts.data(), m_vecCts.size());
#else
	m_pCurve->setRawData(m_vecScn.data(), m_vecCts.data(), m_vecCts.size());
	m_pPoints->setRawData(m_vecScn.data(), m_vecCts.data(), m_vecCts.size());
#endif

	m_pPlot->setAxisTitle(QwtPlot::xBottom, "");
	m_pPlot->setAxisTitle(QwtPlot::yLeft, "");

	m_pPlot->replot();
}

void FilePreviewDlg::FileSelected(const QString& qstrFile)
{
	ClearPlot();

	tl::FileInstr *pInstr = tl::FileInstr::LoadInstr(qstrFile.toStdString().c_str());
	if(!pInstr) return;

	std::unique_ptr<tl::FileInstr> _ptrInstr(pInstr);

	std::vector<std::string> vecScanVars = pInstr->GetScannedVars();
	if(vecScanVars.size() == 0) return;
	m_vecCts = pInstr->GetCol(pInstr->GetCountVar());
	m_vecScn = pInstr->GetCol(vecScanVars[0]);

	//std::copy(m_vecScn.begin(), m_vecScn.end(), std::ostream_iterator<double>(std::cout, " "));
	//std::cout << std::endl;

#ifdef USE_QWT6
	m_pCurve->setRawSamples(m_vecScn.data(), m_vecCts.data(), m_vecCts.size());
	m_pPoints->setRawSamples(m_vecScn.data(), m_vecCts.data(), m_vecCts.size());
#else
	m_pCurve->setRawData(m_vecScn.data(), m_vecCts.data(), m_vecCts.size());
	m_pPoints->setRawData(m_vecScn.data(), m_vecCts.data(), m_vecCts.size());
#endif

	m_pPlot->setAxisTitle(QwtPlot::xBottom, vecScanVars[0].c_str());
	m_pPlot->setAxisTitle(QwtPlot::yLeft, "Counts");

	m_pPlot->replot();
}


#include "FilePreviewDlg.moc"
