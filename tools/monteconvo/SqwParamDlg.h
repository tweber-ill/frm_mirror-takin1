/*
 * S(q,w) parameters dialog
 * @author tweber
 * @date aug-2015
 * @license GPLv2
 */

#ifndef __SQW_DLG_H__
#define __SQW_DLG_H__

#include <QDialog>
#include <QSettings>

#include "ui/ui_sqwparams.h"

class SqwParamDlg : public QDialog, Ui::SqwParamDlg
{ Q_OBJECT
protected:
	QSettings *m_pSett = nullptr;

public:
	SqwParamDlg(QWidget* pParent=nullptr, QSettings* pSett=nullptr);
	virtual ~SqwParamDlg();

protected:
	virtual void accept();
};

#endif
