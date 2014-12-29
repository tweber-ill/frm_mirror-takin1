/*
 * Scan sequences
 * @author tweber
 * @date 24-dec-2014
 */

#include "SequenceDlg.h"

SequenceDlg::SequenceDlg(QWidget* pParent, QSettings* pSett)
			: QDialog(pParent)
{
	setupUi(this);
}

SequenceDlg::~SequenceDlg()
{
}


#include "SequenceDlg.moc"
