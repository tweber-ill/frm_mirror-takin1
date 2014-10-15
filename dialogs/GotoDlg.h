/*
 * Goto Dialog
 * @author Tobias Weber
 * @date 15-oct-2014
 */

#ifndef __GOTO_DLG_H__
#define __GOTO_DLG_H__

#include <QtGui/QDialog>
#include "../ui/ui_goto.h"

class GotoDlg : public QDialog, Ui::GotoDlg
{ Q_OBJECT
	public:
		GotoDlg(QWidget* pParent=0);
		virtual ~GotoDlg();

	protected:

	protected slots:
		void EditedKiKf();
		void EditedE();

		void Calc();
};

#endif
