/*
 * Scattering Triangle tool
 * @author tweber
 * @date feb-2014
 */

#ifndef __TAZ_H__
#define __TAZ_H__

//#include <QtGui/QDialog>
#include <QtGui/QMainWindow>
#include <QtCore/QSettings>
#include <QtCore/QVariant>

#include <string>
#include <vector>

#include "ui/ui_taz.h"
#include "scattering_triangle.h"
#include "tas_layout.h"
#include "recip3d.h"

#include "dialogs/RecipParamDlg.h"
#include "dialogs/RealParamDlg.h"
#include "dialogs/EllipseDlg.h"
#include "dialogs/EllipseDlg3D.h"
#include "../res/ResoDlg.h"
#include "dialogs/SpurionDlg.h"
#include "dialogs/NeutronDlg.h"

#include "helper/spacegroup.h"

class TazDlg : public QMainWindow, Ui::TazDlg
{ Q_OBJECT
	private:
		bool m_bUpdateRecipEdits = 1;

		QAction *m_pSmallq = 0;
		QAction *m_pSnapSmallq = 0;
		QAction *m_pBZ = 0;

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
		QLabel* m_pStatusMsg = 0;

		ScatteringTriangleView *m_pviewRecip = 0;
		ScatteringTriangleScene m_sceneRecip;

		TasLayoutView *m_pviewReal = 0;
		TasLayoutScene m_sceneReal;

		Recip3DDlg *m_pRecip3d = 0;

		std::string m_strCurFile;
		static const std::string s_strTitle;

		const t_mapSpaceGroups* m_pmapSpaceGroups;

		RecipParamDlg m_dlgRecipParam;
		RealParamDlg m_dlgRealParam;

		ResoDlg *m_pReso = 0;
		EllipseDlg *m_pEllipseDlg = 0;
		EllipseDlg3D *m_pEllipseDlg3D = 0;

		SpurionDlg *m_pSpuri = 0;
		NeutronDlg *m_pNeutronDlg = 0;

	protected:
		void InitReso();

	public:
		TazDlg(QWidget *pParent);
		TazDlg() : TazDlg(0) { }
		virtual ~TazDlg();

		bool Load(const char* pcFile);

	protected slots:
		void ChangedTolerance();
		void CalcPeaks();
		void CalcPeaksRecip();
		void UpdateDs();

		void UpdateSampleSense();
		void UpdateMonoSense();
		void UpdateAnaSense();
		void EnableSmallq(bool bEnable);
		void EnableBZ(bool bEnable);

		void Show3D();
		void ShowAbout();

		bool Save();
		bool SaveAs();
		bool Load();

		void ExportReal();
		void ExportRecip();

		void RepopulateSpaceGroups();

		void ShowRecipParams();
		void ShowRealParams();

		void ShowResoParams();
		void ShowResoEllipses();
		void ShowResoEllipses3D();

		void ShowNeutronDlg();

		void ShowSpurions();
		void spurionInfo(const ElasticSpurion& spuris,
				const std::vector<InelasticSpurion>& vecInelCKI,
				const std::vector<InelasticSpurion>& vecInelCKF);
		//void paramsChanged(const RecipParams& parms);

	protected:
		void ExportSceneSVG(QGraphicsScene& scene);

	signals:
		void ResoParamsChanged(const ResoParams& resoparams);
};

#endif
