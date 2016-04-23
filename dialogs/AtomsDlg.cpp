/*
 * Atom Positions Dialog
 * @author Tobias Weber
 * @date nov-2015
 * @license GPLv2
 */

#include "AtomsDlg.h"
#include "tlibs/string/string.h"
#include "tlibs/math/linalg.h"

using t_real = t_real_glob;


AtomsDlg::AtomsDlg(QWidget* pParent, QSettings *pSettings)
	: QDialog(pParent), m_pSettings(pSettings)
{
	setupUi(this);
	if(m_pSettings)
	{
		QFont font;
		if(m_pSettings->contains("main/font_gen") && font.fromString(m_pSettings->value("main/font_gen", "").toString()))
			setFont(font);
	}

	tableAtoms->setColumnWidth(0, 75);
	btnAdd->setIcon(load_icon("res/list-add.svg"));
	btnDel->setIcon(load_icon("res/list-remove.svg"));

#if QT_VER >= 5
	QObject::connect(btnAdd, &QAbstractButton::clicked, this, &AtomsDlg::AddAtom);
	QObject::connect(btnDel, &QAbstractButton::clicked, this, &AtomsDlg::RemoveAtom);
	QObject::connect(buttonBox, &QDialogButtonBox::clicked, this, &AtomsDlg::ButtonBoxClicked);
#else
	QObject::connect(btnAdd, SIGNAL(clicked(bool)), this, SLOT(AddAtom()));
	QObject::connect(btnDel, SIGNAL(clicked(bool)), this, SLOT(RemoveAtom()));
	QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this,
		SLOT(ButtonBoxClicked(QAbstractButton*)));
#endif

	if(m_pSettings && m_pSettings->contains("atoms/geo"))
		restoreGeometry(m_pSettings->value("atoms/geo").toByteArray());
}

AtomsDlg::~AtomsDlg()
{}


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
	int iRow = tableAtoms->rowCount();
	tableAtoms->insertRow(iRow);
	tableAtoms->setItem(iRow, 0, new QTableWidgetItem("H"));
	for(unsigned int i=0; i<3; ++i)
		tableAtoms->setItem(iRow, i+1, new QTableWidgetItem("0"));
}


void AtomsDlg::SetAtoms(const std::vector<AtomPos>& vecAtoms)
{
	tableAtoms->setRowCount(vecAtoms.size());

	for(unsigned int iRow=0; iRow<vecAtoms.size(); ++iRow)
	{
		// add missing items
		for(int iCol=0; iCol<4; ++iCol)
			if(!tableAtoms->item(iRow, iCol))
				tableAtoms->setItem(iRow, iCol, new QTableWidgetItem(""));

		const AtomPos& atom = vecAtoms[iRow];
		tableAtoms->item(iRow, 0)->setText(atom.strAtomName.c_str());
		for(unsigned int i=0; i<3; ++i)
			tableAtoms->item(iRow, i+1)->setText(tl::var_to_str(atom.vecPos[i]).c_str());
	}
}

void AtomsDlg::SendApplyAtoms()
{
	std::vector<AtomPos> vecAtoms;
	vecAtoms.reserve(tableAtoms->rowCount());

	for(int iRow=0; iRow<tableAtoms->rowCount(); ++iRow)
	{
		AtomPos atom;
		atom.strAtomName = tableAtoms->item(iRow, 0)->text().toStdString();
		tl::trim(atom.strAtomName);
		t_real dX = tl::str_to_var<t_real>(tableAtoms->item(iRow, 1)->text().toStdString());
		t_real dY = tl::str_to_var<t_real>(tableAtoms->item(iRow, 2)->text().toStdString());
		t_real dZ = tl::str_to_var<t_real>(tableAtoms->item(iRow, 3)->text().toStdString());
		atom.vecPos = tl::make_vec({dX, dY, dZ});

		vecAtoms.push_back(std::move(atom));
	}

	emit ApplyAtoms(vecAtoms);
}

void AtomsDlg::ButtonBoxClicked(QAbstractButton* pBtn)
{
	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::ApplyRole ||
	   buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		SendApplyAtoms();
	}
	else if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::RejectRole)
	{
		reject();
	}

	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		if(m_pSettings)
			m_pSettings->setValue("atoms/geo", saveGeometry());

		QDialog::accept();
	}
}

void AtomsDlg::closeEvent(QCloseEvent* pEvt)
{
	QDialog::closeEvent(pEvt);
}


#include "AtomsDlg.moc"
