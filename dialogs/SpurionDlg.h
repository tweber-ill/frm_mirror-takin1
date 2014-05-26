/*
 * Spurion Dialog
 * @author Tobias Weber
 * @date 26-may-2014
 */

#ifndef __SPURION_DLG_H__
#define __SPURION_DLG_H__

#include <QtGui/QDialog>
#include "../ui/ui_spurions.h"


class SpurionDlg : public QDialog, Ui::SpurionDlg
{ Q_OBJECT
	protected:

	public:
		SpurionDlg(QWidget* pParent=0);
		virtual ~SpurionDlg();

	protected slots:
		void ChangedKiKfMode();
		void Calc();
};


#endif
