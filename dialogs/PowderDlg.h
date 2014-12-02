/*
 * Powder Line Dialog
 * @author Tobias Weber
 * @date 2013, 2-dec-2014
 */
 
#ifndef __POWDER_DLG_H__
#define __POWDER_DLG_H__

#include <QtGui/QDialog>
#include <QtCore/QSettings>
#include "../ui/ui_powder.h"

#include "../helper/spacegroup.h"


class PowderDlg : public QDialog, Ui::PowderDlg
{ Q_OBJECT
	protected:
		QSettings *m_pSettings = 0;

		CrystalSystem m_crystalsys = CRYS_NOT_SET;
		const t_mapSpaceGroups* m_pmapSpaceGroups;

	public:	
		PowderDlg(QWidget* pParent=0, QSettings* pSett=0);
		virtual ~PowderDlg();
		
	protected slots:
		void CalcPeaks();
		
		void CheckCrystalType();
		void SpaceGroupChanged();
		void RepopulateSpaceGroups();
		
	protected:
		virtual void showEvent(QShowEvent *pEvt);
		virtual void accept();
		
		const SpaceGroup* GetCurSpaceGroup() const;
};

#endif
