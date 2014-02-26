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
#include "tas_layout.h"

class TazDlg : public QDialog, Ui::TazDlg
{ Q_OBJECT
	private:
		bool m_bUpdateRecipEdits = 1;

	protected:
		ScatteringTriangleScene m_sceneRecip;
		TasLayoutScene m_sceneReal;

	public:
		TazDlg(QWidget *pParent);
		TazDlg() { TazDlg(0); }
		virtual ~TazDlg();

	protected slots:
		void CalcPeaks();
		void CalcPeaksRecip();
		void UpdateDs();

		void UpdateSampleSense();
		void UpdateMonoSense();
		void UpdateAnaSense();
};

#endif
