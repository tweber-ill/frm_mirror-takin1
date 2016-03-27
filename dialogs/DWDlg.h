/*
 * Scattering factors dialog (e.g. Debye-Waller factor)
 * @author tweber
 * @date 2013, jan-2015
 * @license GPLv2
 */

#ifndef __DWDLG_H__
#define __DWDLG_H__

#include <QDialog>
#include <QSettings>
#include "ui/ui_dw.h"

#include <vector>
#include <memory>
#include "libs/qthelper.h"
#include <qwt_legend.h>


class DWDlg : public QDialog, Ui::DWDlg
{ Q_OBJECT
protected:
	QSettings *m_pSettings = nullptr;

	// dw stuff
	std::vector<double> m_vecQ, m_vecDeb;
	std::unique_ptr<QwtPlotWrapper> m_plotwrapDW;

	// ana stuff
	std::vector<double> m_veckf, m_vecInt;
	std::unique_ptr<QwtPlotWrapper> m_plotwrapAna;

	// bose stuff
	std::vector<double> m_vecBoseE, m_vecBoseIntPos, m_vecBoseIntNeg;
	std::unique_ptr<QwtPlotWrapper> m_plotwrapBose;
	QwtLegend *m_pLegendBose = nullptr;

	// lorentz stuff
	std::vector<double> m_vecLor2th, m_vecLor;
	std::unique_ptr<QwtPlotWrapper> m_plotwrapLor;

protected:
	virtual void showEvent(QShowEvent *pEvt) override;
	virtual void accept() override;

protected slots:
	void cursorMoved(const QPointF& pt);
	void CalcDW();
	void CalcAna();
	void CalcBose();
	void CalcLorentz();

public:
	DWDlg(QWidget* pParent = nullptr, QSettings *pSettings = nullptr);
	virtual ~DWDlg();
};

#endif
