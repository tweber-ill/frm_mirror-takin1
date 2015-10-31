/*
 * Powder Line Dialog
 * @author Tobias Weber
 * @date 2013, 2-dec-2014
 * @license GPLv2
 */

#ifndef __POWDER_DLG_H__
#define __POWDER_DLG_H__

#include <QDialog>
#include <QSettings>
#include "ui/ui_powder.h"

#include <map>
#include <string>

#ifdef USE_CLP
	#include "helper/spacegroup_clp.h"
#else
	#include "helper/spacegroup.h"
#endif

#include "tlibs/file/xml.h"


class PowderDlg : public QDialog, Ui::PowderDlg
{ Q_OBJECT
	protected:
		bool m_bDontCalc = 1;
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

		void SavePowder();
		void LoadPowder();

	protected:
		virtual void showEvent(QShowEvent *pEvt);
		virtual void accept();

		const SpaceGroup* GetCurSpaceGroup() const;

		void Save(std::map<std::string, std::string>& mapConf, const std::string& strXmlRoot);
		void Load(tl::Xml& xml, const std::string& strXmlRoot);
};

#endif
