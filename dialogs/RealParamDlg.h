/*
 * Real Space Parameters
 * @author tweber
 * @date 29-mar-2014
 * @copyright GPLv2
 */

#ifndef __REAL_PARAMS_H__
#define __REAL_PARAMS_H__

#include <QtGui/QDialog>
#include <QtCore/QSettings>
#include "../ui/ui_real_params.h"


struct RealParams
{
	double dMonoTT, dSampleTT, dAnaTT;
	double dMonoT, dSampleT, dAnaT;
	double dLenMonoSample, dLenSampleAna, dLenAnaDet;
};


class RealParamDlg : public QDialog, Ui::RealParamDlg
{ Q_OBJECT
	protected:
		QSettings *m_pSettings = 0;

	public:
		RealParamDlg(QWidget* pParent=0, QSettings* pSett=0);
		virtual ~RealParamDlg();

	public slots:
		void paramsChanged(const RealParams& parms);

	protected:
		virtual void closeEvent(QCloseEvent *pEvt);
		virtual void showEvent(QShowEvent *pEvt);
		virtual void accept();
};

#endif
