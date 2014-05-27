/*
 * Spurion Dialog
 * @author Tobias Weber
 * @date 26-may-2014
 */

#ifndef __SPURION_DLG_H__
#define __SPURION_DLG_H__

#include <QtGui/QDialog>
#include "../ui/ui_spurions.h"
#include "RecipParamDlg.h"


class SpurionDlg : public QDialog, Ui::SpurionDlg
{ Q_OBJECT
	protected:
		double m_dEi=0., m_dEf=0.;

	public:
		SpurionDlg(QWidget* pParent=0);
		virtual ~SpurionDlg();

	protected slots:
		void ChangedKiKfMode();
		void Calc();

		void paramsChanged(const RecipParams& parms);
};


#endif
