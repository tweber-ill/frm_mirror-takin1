/*
 * Spurion Dialog
 * @author Tobias Weber
 * @date 26-may-2014
 */

#ifndef __SPURION_DLG_H__
#define __SPURION_DLG_H__

#include <QtGui/QDialog>
#include <QtCore/QSettings>
#include "../ui/ui_spurions.h"
#include "RecipParamDlg.h"


class SpurionDlg : public QDialog, Ui::SpurionDlg
{ Q_OBJECT
	protected:
		QSettings *m_pSettings = 0;
		double m_dEi=0., m_dEf=0.;

	public:
		SpurionDlg(QWidget* pParent=0, QSettings *pSett=0);
		virtual ~SpurionDlg();

	protected slots:
		void ChangedKiKfMode();
		void Calc();

		void paramsChanged(const RecipParams& parms);

	protected:
		virtual void showEvent(QShowEvent *pEvt);
		virtual void accept();
};


#endif
