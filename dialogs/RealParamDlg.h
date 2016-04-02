/*
 * Real Space Parameters
 * @author tweber
 * @date 29-mar-2014
 * @license GPLv2
 */

#ifndef __REAL_PARAMS_H__
#define __REAL_PARAMS_H__

#include <QDialog>
#include <QSettings>
#include "ui/ui_real_params.h"
#include "libs/globals.h"


struct RealParams
{
	t_real_glob dMonoTT, dSampleTT, dAnaTT;
	t_real_glob dMonoT, dSampleT, dAnaT;
	t_real_glob dLenMonoSample, dLenSampleAna, dLenAnaDet;
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
