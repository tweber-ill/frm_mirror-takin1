/*
 * Server Dialog
 * @author Tobias Weber
 * @date 27-aug-2014
 * @copyright GPLv2
 */

#ifndef __SRV_DLG_H__
#define __SRV_DLG_H__

#include <QtGui/QDialog>
#include <QtCore/QSettings>
#include "../ui/ui_connection.h"

class SrvDlg : public QDialog, Ui::SrvDlg
{ Q_OBJECT
	protected:
		QSettings *m_pSettings = 0;

	public:
		SrvDlg(QWidget* pParent=0, QSettings *pSett=0);
		virtual ~SrvDlg();

	signals:
		void ConnectTo(const QString& strHost, const QString& strPort);

	protected slots:

	protected:
		virtual void accept();
};

#endif
