/*
 * Form Factor & Scattering Length Dialog
 * @author tweber
 * @date nov-2015
 * @license GPLv2
 */

#ifndef __TAKIN_FF_DLG_H__
#define __TAKIN_FF_DLG_H__

#include <QDialog>
#include <QSettings>
#include <vector>
#include <memory>
#include "ui/ui_formfactors.h"
#include "libs/qthelper.h"


class FormfactorDlg : public QDialog, Ui::FormFactorDlg
{ Q_OBJECT
protected:
	QSettings *m_pSettings = nullptr;

	// form factors
	std::vector<double> m_vecQ, m_vecFF;
	std::unique_ptr<QwtPlotWrapper> m_plotwrap;

	// scattering lengths
	std::vector<double> m_vecElem, m_vecSc;
	std::unique_ptr<QwtPlotWrapper> m_plotwrapSc;


protected:
	virtual void closeEvent(QCloseEvent* pEvt) override;
	void SetupAtoms();

protected slots:
	void SearchAtom(const QString& qstr);
	void AtomSelected(QListWidgetItem *pItem, QListWidgetItem *pItemPrev);

	void PlotScatteringLengths();

	void cursorMoved(const QPointF& pt);

public:
	FormfactorDlg(QWidget* pParent = nullptr, QSettings *pSettings = nullptr);
	virtual ~FormfactorDlg();
};

#endif
