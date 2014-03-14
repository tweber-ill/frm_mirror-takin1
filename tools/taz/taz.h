/*
 * Scattering Triangle tool
 * @author tweber
 * @date feb-2014
 */

#ifndef __TAZ_H__
#define __TAZ_H__

#include <QtGui/QDialog>
#include <QtCore/QSettings>
#include <QtCore/QVariant>

#include <string>
#include <vector>

#include "ui/ui_taz.h"
#include "scattering_triangle.h"
#include "tas_layout.h"
#include "recip3d.h"

class TazDlg : public QDialog, Ui::TazDlg
{ Q_OBJECT
	private:
		bool m_bUpdateRecipEdits = 1;

		std::vector<QLineEdit*> m_vecEdits_real;
		std::vector<QLineEdit*> m_vecEdits_recip;
		std::vector<QLineEdit*> m_vecEdits_plane;
		std::vector<QLineEdit*> m_vecEdits_monoana;

		std::vector<QDoubleSpinBox*> m_vecSpinBoxesSample;
		std::vector<QCheckBox*> m_vecCheckBoxesSenses;

		std::vector<std::string> m_vecEditNames_real;
		std::vector<std::string> m_vecEditNames_recip;
		std::vector<std::string> m_vecEditNames_plane;
		std::vector<std::string> m_vecEditNames_monoana;

		std::vector<std::string> m_vecSpinBoxNamesSample;
		std::vector<std::string> m_vecCheckBoxNamesSenses;

	protected:
		QSettings m_settings;

		ScatteringTriangleView *m_pviewRecip = 0;
		ScatteringTriangleScene m_sceneRecip;

		TasLayoutView *m_pviewReal = 0;
		TasLayoutScene m_sceneReal;

		Recip3DDlg *m_pRecip3d = 0;

		std::string m_strCurFile;
		static const std::string s_strTitle;

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

		void Save();
		void SaveAs();
		void Load();
};

#endif
