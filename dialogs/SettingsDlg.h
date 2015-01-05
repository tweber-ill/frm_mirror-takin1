/*
 * Settings
 * @author tweber
 * @date 5-dec-2014
 */

#ifndef __TAZ_SETTINGS_H__
#define __TAZ_SETTINGS_H__

#include <QtGui/QDialog>
#include <QtCore/QSettings>

#include <tuple>
#include <vector>
#include <string>

#include "../ui/ui_settings.h"


class SettingsDlg : public QDialog, Ui::SettingsDlg
{ Q_OBJECT
	protected:
		QSettings *m_pSettings = 0;

		// key, default, lineedit
		typedef std::tuple<std::string, std::string, QLineEdit*> t_tupEdit;
		std::vector<t_tupEdit> m_vecEdits;

	public:
		SettingsDlg(QWidget* pParent=0, QSettings* pSett=0);
		virtual ~SettingsDlg();

	protected:
		void LoadSettings();
		void SaveSettings();

		void SetDefaults(bool bOverwrite=0);

	protected:
		virtual void showEvent(QShowEvent *pEvt);

	protected slots:
		void ButtonBoxClicked(QAbstractButton* pBtn);
		void SelectFont();
};

#endif
