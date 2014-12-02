/*
 * Powder Line Dialog
 * @author Tobias Weber
 * @date 013, 2-dec-2014
 */
 
#include "PowderDlg.h"

PowderDlg::PowderDlg(QWidget* pParent, QSettings* pSett) 
			: QDialog(pParent), m_pSettings(pSett)
{
	this->setupUi(this);
}

PowderDlg::~PowderDlg()
{}


#include "PowderDlg.moc"