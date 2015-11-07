/*
 * Atom Positions Dialog
 * @author Tobias Weber
 * @date nov-2015
 * @license GPLv2
 */

#ifndef __TAKIN_ATOMS_DLG_H__
#define __TAKIN_ATOMS_DLG_H__

#include <QDialog>
#include <QSettings>
#include "ui/ui_atoms.h"


class AtomsDlg : public QDialog, Ui::AtomsDlg
{ Q_OBJECT
protected:
	QSettings *m_pSettings = nullptr;

protected:
	virtual void closeEvent(QCloseEvent*) override;

protected slots:
	void ButtonBoxClicked(QAbstractButton* pBtn);
	void RemoveAtom();
	void AddAtom();

public:
	AtomsDlg(QWidget* pParent = nullptr, QSettings *pSettings = nullptr);
	virtual ~AtomsDlg();
};


#endif
