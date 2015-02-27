/*
 * File Preview Dialog
 * @author Tobias Weber
 * @date feb-2015
 * @copyright GPLv2
 */

#ifndef __FILE_PREV_DLG__
#define __FILE_PREV_DLG__

#include <QtGui/QFileDialog>
#include <QtCore/QString>
#include <vector>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>


class FilePreviewDlg : public QFileDialog
{ Q_OBJECT
	protected:
		QwtPlot *m_pPlot = nullptr;
		QwtPlotCurve *m_pCurve = nullptr;
		QwtPlotCurve *m_pPoints = nullptr;

		std::vector<double> m_vecScn, m_vecCts;

	protected:
		void ClearPlot();

	public:
		FilePreviewDlg(QWidget* pParent, const char* pcTitle);
		virtual ~FilePreviewDlg();

	protected slots:
		void FileSelected(const QString& strFile);
};

#endif
