/*
 * Atom Positions Dialog
 * @author Tobias Weber
 * @date nov-2015
 * @license GPLv2
 */

#include "AtomsDlg.h"
#include "helper/globals.h"
#include <iostream>


AtomsDlg::AtomsDlg(QWidget* pParent, QSettings *pSettings)
	: QDialog(pParent), m_pSettings(pSettings)
{
	setupUi(this);
	tableAtoms->setColumnWidth(0, 75);
	btnAdd->setIcon(load_icon("res/list-add.svg"));
	btnDel->setIcon(load_icon("res/list-remove.svg"));

	QObject::connect(btnAdd, SIGNAL(clicked(bool)), this, SLOT(AddAtom()));
	QObject::connect(btnDel, SIGNAL(clicked(bool)), this, SLOT(RemoveAtom()));
	QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this,
		SLOT(ButtonBoxClicked(QAbstractButton*)));

	if(m_pSettings && m_pSettings->contains("atoms/geo"))
		restoreGeometry(m_pSettings->value("atoms/geo").toByteArray());
}

AtomsDlg::~AtomsDlg()
{
}


void AtomsDlg::RemoveAtom()
{
	bool bNothingRemoved = 1;

	// remove selected rows
	QList<QTableWidgetSelectionRange> lstSel = tableAtoms->selectedRanges();
	for(QTableWidgetSelectionRange& range : lstSel)
	{
		//std::cout << range.bottomRow() << " " << range.topRow() << std::endl;
		for(int iRow=range.bottomRow(); iRow>=range.topRow(); --iRow)
		{
			tableAtoms->removeRow(iRow);
			bNothingRemoved = 0;
		}
	}

	// remove last row if nothing is selected
	if(bNothingRemoved)
		tableAtoms->removeRow(tableAtoms->rowCount()-1);
}

void AtomsDlg::AddAtom()
{
	tableAtoms->insertRow(tableAtoms->rowCount());
}


void AtomsDlg::ButtonBoxClicked(QAbstractButton* pBtn)
{
	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::ApplyRole ||
	   buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{

	}
	else if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::RejectRole)
	{
		reject();
	}

	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		if(m_pSettings)
		{
			m_pSettings->setValue("atoms/geo", saveGeometry());
		}

		QDialog::accept();
	}
}

void AtomsDlg::closeEvent(QCloseEvent* pEvt)
{
	QDialog::closeEvent(pEvt);
}


#include "AtomsDlg.moc"
