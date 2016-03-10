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

#include "helper/spacegroup.h"
#include "tlibs/file/prop.h"
#include "AtomsDlg.h"

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
//#include <qwt_legend.h>


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

		QwtPlotCurve *m_pCurve = nullptr;
		QwtPlotGrid *m_pGrid = nullptr;
		QwtPlotPicker *m_pPicker = nullptr;
		QwtPlotZoomer* m_pZoomer = nullptr;
		QwtPlotPanner* m_pPanner = nullptr;

		QwtPlotCurve *m_pCurveX = nullptr;
		QwtPlotGrid *m_pGridX = nullptr;
		QwtPlotPicker *m_pPickerX = nullptr;
		QwtPlotZoomer* m_pZoomerX = nullptr;
		QwtPlotPanner* m_pPannerX = nullptr;


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
