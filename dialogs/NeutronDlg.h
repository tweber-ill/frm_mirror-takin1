/*
 * Neutron Properties Dialog (was: old formula dialog)
 * @author Tobias Weber
 * @date jul-2013, 28-may-2014
 */

#ifndef __NEUTRON_DLG_H__
#define __NEUTRON_DLG_H__

#include <QtGui/QDialog>
#include <QtCore/QSettings>
#include "../ui/ui_neutrons.h"


class NeutronDlg : public QDialog, Ui::NeutronDlg
{ Q_OBJECT
	protected:
		QSettings *m_pSettings = 0;

	protected:
		void setupConstants();

	public:
		NeutronDlg(QWidget* pParent=0, QSettings* pSett=0);
		virtual ~NeutronDlg();

	protected slots:
		void CalcNeutronLam();
		void CalcNeutronk();
		void CalcNeutronv();
		void CalcNeutronE();
		void CalcNeutronOm();
		void CalcNeutronF();
		void CalcNeutronT();

	protected:
		virtual void showEvent(QShowEvent *pEvt);
		virtual void accept();
};


#endif
