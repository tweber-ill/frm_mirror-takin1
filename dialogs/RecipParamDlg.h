/*
 * Reciprocal Space Parameters
 * @author tweber
 * @date 26-mar-2014
 */

#ifndef __RECIP_PARAMS_H__
#define __RECIP_PARAMS_H__

#include <QtGui/QDialog>
#include "../ui/ui_recip_params.h"


struct RecipParams
{
	double dki, dkf;
	double dE, dQ, dq;
	double d2Theta, dTheta, dKiQ, dKfQ;
};


class RecipParamDlg : public QDialog, Ui::RecipParamDlg
{ Q_OBJECT
	protected:

	public:
		RecipParamDlg(QWidget* pParent=0);
		virtual ~RecipParamDlg();

	public slots:
		void paramsChanged(const RecipParams& parms);

		void KiChanged();
		void KfChanged();
};

#endif
