/*
 * Scattering Triangle tool
 * @author tweber
 * @date feb-2014
 */

#ifndef __TAZ_H__
#define __TAZ_H__

#include <QtGui/QDialog>

#include "ui/ui_taz.h"
#include "scattering_triangle.h"

class TazDlg : public QDialog, Ui::TazDlg
{ Q_OBJECT
	protected:
		ScatteringTriangleScene m_sceneRecip;

	public:
		TazDlg(QWidget *pParent);
		TazDlg() { TazDlg(0); }
		virtual ~TazDlg();
};

#endif
