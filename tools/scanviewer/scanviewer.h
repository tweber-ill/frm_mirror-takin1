/*
 * Scan viewer
 * @author tweber
 * @date mar-2015
 * @copyright GPLv2
 */

#ifndef __TAZ_SCANVIEWER_H__
#define __TAZ_SCANVIEWER_H__

#include <QtGui/QDialog>
#include <QtCore/QSettings>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>

#include <string>
#include <vector>
#include "tlibs/file/loadinstr.h"
#include "ui/ui_scanviewer.h"


class ScanViewerDlg : public QDialog, Ui::ScanViewerDlg
{ Q_OBJECT
protected:
	QSettings m_settings;
	std::string m_strCurDir, m_strCurFile;
	std::string m_strSelectedKey;
	std::vector<std::string> m_vecExts;

	bool m_bDoUpdate = 0;
	tl::FileInstr *m_pInstr = nullptr;
	std::vector<double> m_vecX, m_vecY;

	QwtPlotCurve *m_pCurve = nullptr, *m_pPoints = nullptr;
	QwtPlotGrid* m_pGrid = nullptr;
	QwtPlotPicker* m_pPicker = nullptr;
	QwtPlotZoomer* m_pZoomer = nullptr;

	std::string m_strX, m_strY, m_strCmd;

protected:
	void ClearPlot();
	void PlotScan();
	void ShowProps();

	void GenerateForRoot();
	void GenerateForGnuplot();
	void GenerateForPython();
	void GenerateForHermelin();

protected slots:
	void GenerateExternal(int iLang=0);

	void UpdateFileList();
	void FileSelected(QListWidgetItem *pItem, QListWidgetItem *pItemPrev);
	void PropSelected(QTableWidgetItem *pItem, QTableWidgetItem *pItemPrev);
	void SelectDir();
	void ChangedPath();

	void XAxisSelected(const QString&);
	void YAxisSelected(const QString&);

	//void openExternally();

public:
	ScanViewerDlg(QWidget* pParent = nullptr);
	virtual ~ScanViewerDlg();
};


#endif
