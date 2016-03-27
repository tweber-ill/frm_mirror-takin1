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
#include <vector>
#include <memory>

#include "libs/spacegroups/spacegroup.h"
#include "libs/qthelper.h"
#include "tlibs/file/prop.h"
#include "AtomsDlg.h"


struct PowderLine
{
	int h, k, l;

	double dAngle;
	std::string strAngle;

	double dQ;
	std::string strQ;

	std::string strPeaks;

	unsigned int iMult;
	double dFn, dFx;	// neutron/xray structure factors
	double dIn, dIx;	// neutron/xray intensities
};


class PowderDlg : public QDialog, Ui::PowderDlg
{ Q_OBJECT
	protected:
		std::vector<double> m_vecTT, m_vecInt;
		std::vector<double> m_vecTTx, m_vecIntx;

		std::unique_ptr<QwtPlotWrapper> m_plotwrapN;
		std::unique_ptr<QwtPlotWrapper> m_plotwrapX;

	protected:
		bool m_bDontCalc = 1;
		QSettings *m_pSettings = 0;

		CrystalSystem m_crystalsys = CRYS_NOT_SET;
		const t_mapSpaceGroups* m_pmapSpaceGroups;

		AtomsDlg *m_pAtomsDlg = nullptr;
		std::vector<AtomPos> m_vecAtoms;

	public:
		PowderDlg(QWidget* pParent=0, QSettings* pSett=0);
		virtual ~PowderDlg();

	protected:
		void PlotPowderLines(const std::vector<const PowderLine*>& vecLines);

	protected slots:
		void CalcPeaks();

		void CheckCrystalType();
		void SpaceGroupChanged();
		void RepopulateSpaceGroups();

		void SaveTable();
		void SavePowder();
		void LoadPowder();

		void ShowAtomDlg();
		void ApplyAtoms(const std::vector<AtomPos>&);

		void cursorMoved(const QPointF& pt);

	protected:
		virtual void showEvent(QShowEvent *pEvt);
		virtual void accept();

		const SpaceGroup* GetCurSpaceGroup() const;

		void Save(std::map<std::string, std::string>& mapConf, const std::string& strXmlRoot);
		void Load(tl::Prop<std::string>& xml, const std::string& strXmlRoot);
};

#endif
