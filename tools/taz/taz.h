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
#include "recip3d.h"

class TazDlg : public QDialog, Ui::TazDlg
{ Q_OBJECT
	private:
		bool m_bUpdateRecipEdits = 1;

	protected:
		ScatteringTriangleView *m_pviewRecip = 0;
		ScatteringTriangleScene m_sceneRecip;

		TasLayoutView *m_pviewReal = 0;
		TasLayoutScene m_sceneReal;

		Recip3DDlg *m_pRecip3d = 0;

	public:
		TazDlg(QWidget *pParent);
		TazDlg() { TazDlg(0); }
		virtual ~TazDlg();

	protected slots:
		void ChangedTolerance();
		void CalcPeaks();
		void CalcPeaksRecip();
		void UpdateDs();

		void UpdateSampleSense();
		void UpdateMonoSense();
		void UpdateAnaSense();
		void EnableSmallq(bool bEnable);

		void Show3D();
		void ShowAbout();
};

#endif
