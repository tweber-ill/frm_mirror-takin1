/*
 * File Preview Dialog
 * @author Tobias Weber
 * @date feb-2015
 * @license GPLv2
 */

#ifndef __FILE_PREV_DLG__
#define __FILE_PREV_DLG__

#include <QFileDialog>
#include <QString>
#include <QSettings>
#include <vector>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>


class FilePreviewDlg : public QFileDialog
{ Q_OBJECT
	protected:
		QSettings *m_pSettings = nullptr;
	
		QwtPlot *m_pPlot = nullptr;
		QwtPlotCurve *m_pCurve = nullptr;
		QwtPlotCurve *m_pPoints = nullptr;

		std::vector<double> m_vecScn, m_vecCts;

	protected:
		void ClearPlot();

	public:
		FilePreviewDlg(QWidget* pParent, const char* pcTitle, QSettings* pSett=0);
		virtual ~FilePreviewDlg();

	protected slots:
		void FileSelected(const QString& strFile);
};

#endif
