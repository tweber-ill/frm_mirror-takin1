/*
 * S(q,w) parameters dialog
 * @author tweber
 * @date aug-2015
 * @license GPLv2
 */

#include "SqwParamDlg.h"


SqwParamDlg::SqwParamDlg(QWidget* pParent, QSettings* pSett)
	: QDialog(pParent), m_pSett(pSett)
{
	setupUi(this);
}

SqwParamDlg::~SqwParamDlg()
{}

void SqwParamDlg::accept()
{
	QDialog::accept();
}


#include "SqwParamDlg.moc"
