/*
 * Space Group List Dialog
 * @author tweber
 * @date oct-2015
 * @license GPLv2
 */

#ifndef __TAZ_SGLISTDLG_H__
#define __TAZ_SGLISTDLG_H__

#include <QDialog>
#include <QSettings>

#include "ui/ui_sglist.h"


class SgListDlg : public QDialog, Ui::SgListDlg
{ Q_OBJECT
protected:
	QSettings m_settings;
	
protected:
	void SetupSpacegroups();

protected slots:
	void SGSelected(QListWidgetItem *pItem, QListWidgetItem *pItemPrev);

public:
	SgListDlg(QWidget* pParent = nullptr);
	virtual ~SgListDlg();
};


#endif