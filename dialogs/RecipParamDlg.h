/*
 * Reciprocal Space Parameters
 * @author tweber
 * @date 26-mar-2014
 * @copyright GPLv2
 */

#ifndef __RECIP_PARAMS_H__
#define __RECIP_PARAMS_H__

#include <QtGui/QDialog>
#include <QtCore/QSettings>
#include "../ui/ui_recip_params.h"


struct RecipParams
{
	double dki, dkf;
	double dE, dQ, dq;
	double d2Theta, dTheta, dKiQ, dKfQ;
	double dAngleQVec0;

	double Q[3], Q_rlu[3];
	double G[3], G_rlu[3];
	double q[3], q_rlu[3];

	double orient_0[3], orient_1[3], orient_up[3];
};


class RecipParamDlg : public QDialog, Ui::RecipParamDlg
{ Q_OBJECT
	protected:
		QSettings *m_pSettings = 0;

	public:
		RecipParamDlg(QWidget* pParent=0, QSettings* pSett=0);
		virtual ~RecipParamDlg();

	public slots:
		void paramsChanged(const RecipParams& parms);

	protected slots:
		void KiChanged();
		void KfChanged();

	protected:
		virtual void closeEvent(QCloseEvent *pEvt);
		virtual void showEvent(QShowEvent *pEvt);
		virtual void accept();
};

#endif
