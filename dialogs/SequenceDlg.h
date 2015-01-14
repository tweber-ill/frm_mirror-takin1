/*
 * Scan sequences
 * @author tweber
 * @date 24-dec-2014
 * @copyright GPLv2
 */

#ifndef __SEQ_DLG_H__
#define __SEQ_DLG_H__

#include <QtGui/QDialog>
#include <QtCore/QSettings>

#include "../ui/ui_sequences.h"


class SequenceDlg : public QDialog, Ui::SequenceDlg
{ Q_OBJECT
	protected:

	public:
		SequenceDlg(QWidget* pParent=0, QSettings* pSett=0);
		virtual ~SequenceDlg();
};

#endif
