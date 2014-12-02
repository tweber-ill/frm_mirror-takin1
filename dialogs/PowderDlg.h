/*
 * Powder Line Dialog
 * @author Tobias Weber
 * @date 013, 2-dec-2014
 */
 
#ifndef __POWDER_DLG_H__
#define __POWDER_DLG_H__

#include <QtGui/QDialog>
#include <QtCore/QSettings>
#include "../ui/ui_powder.h"

class PowderDlg : public QDialog, Ui::PowderDlg
{ Q_OBJECT
	protected:
		QSettings *m_pSettings = 0;

	public:	
		PowderDlg(QWidget* pParent=0, QSettings* pSett=0);
		virtual ~PowderDlg();
};

#endif
