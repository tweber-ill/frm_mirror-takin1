/*
 * Neutron Properties Dialog (was: old formula dialog)
 * @author Tobias Weber
 * @date jul-2013, 28-may-2014
 * @copyright GPLv2
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
		
		void RealThetaEdited();
		void RecipThetaEdited();
		void RealTwoThetaEdited();
		void RecipTwoThetaEdited();
		void CalcBraggReal();
		void CalcBraggRecip();
		
		void EnableRealEdits();
		void EnableRecipEdits();

	protected:
		virtual void showEvent(QShowEvent *pEvt);
		virtual void accept();
		
		static void SetEditTT(QLineEdit *pEditT, QLineEdit *pEditTT);
		static void SetEditT(QLineEdit *pEditT, QLineEdit *pEditTT);
};


#endif
