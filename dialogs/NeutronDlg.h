/*
 * Neutron Properties Dialog (was: old formula dialog)
 * @author Tobias Weber
 * @date jul-2013, 28-may-2014
 */

#ifndef __NEUTRON_DLG_H__
#define __NEUTRON_DLG_H__

#include <QtGui/QDialog>
#include "../ui/ui_neutrons.h"


class NeutronDlg : public QDialog, Ui::NeutronDlg
{ Q_OBJECT
	protected:
		void setupConstants();

	public:
		NeutronDlg(QWidget* pParent=0);
		virtual ~NeutronDlg();

	protected slots:
		void CalcNeutronLam();
		void CalcNeutronk();
		void CalcNeutronv();
		void CalcNeutronE();
		void CalcNeutronOm();
		void CalcNeutronT();
};


#endif
