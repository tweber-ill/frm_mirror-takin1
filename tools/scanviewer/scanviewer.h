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
	QwtPlotCurve *m_pCurve = nullptr, *m_pPoints = nullptr;
	std::vector<double> m_vecX, m_vecY;

protected:
	void ClearPlot();
	void PlotScan();
	void ShowProps();

protected slots:
	void UpdateFileList();
	void FileSelected(QListWidgetItem *pItem, QListWidgetItem *pItemPrev);
	void PropSelected(QTableWidgetItem *pItem, QTableWidgetItem *pItemPrev);
	void SelectDir();
	void ChangedPath();

	void XAxisSelected(const QString&);
	void YAxisSelected(const QString&);

public:
	ScanViewerDlg(QWidget* pParent = nullptr);
	virtual ~ScanViewerDlg();
};


#endif
