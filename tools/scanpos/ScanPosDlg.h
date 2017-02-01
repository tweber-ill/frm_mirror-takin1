/**
 * Scan Position Plotter Dialog
 * @author Tobias Weber
 * @date Feb-2017
 * @license GPLv2
 */

#ifndef __SCANPOS_DLG_H__
#define __SCANPOS_DLG_H__

#include <QDialog>
#include <QSettings>
#include "ui/ui_scanpos.h"

class ScanPosDlg : public QDialog, Ui::ScanPosDlg
{ Q_OBJECT
	protected:
		QSettings *m_pSettings = 0;

		virtual void accept() override;

	public:
		ScanPosDlg(QWidget* pParent=0, QSettings *pSett=0);
		virtual ~ScanPosDlg() = default;

	protected:
		std::vector<std::string> GetFiles(bool bMultiple);

	protected slots:
		void LoadPlaneFromFile();
		void LoadPosFromFile();
		void UpdatePlot();
		void SavePlot();
		void ButtonBoxClicked(QAbstractButton* pBtn);
};

#endif
