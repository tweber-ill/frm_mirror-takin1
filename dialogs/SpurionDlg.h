/*
 * Spurion Dialog
 * @author Tobias Weber
 * @date 26-may-2014
 * @license GPLv2
 */

#ifndef __SPURION_DLG_H__
#define __SPURION_DLG_H__

#include <QDialog>
#include <QSettings>
#include <vector>
#include <memory>
#include "ui/ui_spurions.h"
#include "RecipParamDlg.h"
#include "libs/qthelper.h"


class SpurionDlg : public QDialog, Ui::SpurionDlg
{ Q_OBJECT
	protected:
		QSettings *m_pSettings = 0;
		double m_dEi=0., m_dEf=0.;

		std::vector<double> m_vecQ, m_vecE;
		std::unique_ptr<QwtPlotWrapper> m_plotwrap;

	public:
		SpurionDlg(QWidget* pParent=0, QSettings *pSett=0);
		virtual ~SpurionDlg();

	protected slots:
		void ChangedKiKfMode();
		void Calc();

		void CalcInel();
		void CalcBragg();

		void cursorMoved(const QPointF& pt);
		void paramsChanged(const RecipParams& parms);

	protected:
		virtual void showEvent(QShowEvent *pEvt);
		virtual void accept();
};


#endif
