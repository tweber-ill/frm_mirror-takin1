/**
 * monte carlo convolution tool
 * @author tweber
 * @date aug-2015
 * @license GPLv2
 */

#ifndef __MCONVO_GUI_H__
#define __MCONVO_GUI_H__

#include <QDialog>
#include <QSettings>
#include <QMenuBar>

#include <thread>
#include <atomic>
#include <memory>

#include "ui/ui_monteconvo.h"

#include "libs/qthelper.h"
#include "libs/qwthelper.h"
#include "tlibs/file/prop.h"

#include "sqwfactory.h"

#include "tools/res/defs.h"
#include "tools/convofit/scan.h"

#include "dialogs/FavDlg.h"
#include "SqwParamDlg.h"
#include "TASReso.h"


class ConvoDlg : public QDialog, Ui::ConvoDlg
{ Q_OBJECT
protected:
	std::thread *m_pth = nullptr;
	std::atomic<bool> m_atStop;

	QSettings *m_pSett = nullptr;
	QMenuBar *m_pMenuBar = nullptr;
	SqwParamDlg *m_pSqwParamDlg = nullptr;
	FavDlg *m_pFavDlg = nullptr;

	bool m_bAllowSqwReinit = 1;
	std::shared_ptr<SqwBase> m_pSqw;
	std::vector<t_real_reso> m_vecQ, m_vecS, m_vecScaledS;
	std::vector<std::vector<t_real_reso>> m_vecE, m_vecW;
	std::unique_ptr<QwtPlotWrapper> m_plotwrap, m_plotwrap2d;

	bool m_bUseScan = 0;
	Scan m_scan;

protected:
	std::vector<QDoubleSpinBox*> m_vecSpinBoxes;
	std::vector<QSpinBox*> m_vecIntSpinBoxes;
	std::vector<QLineEdit*> m_vecEditBoxes;
	std::vector<QComboBox*> m_vecComboBoxes;
	std::vector<QCheckBox*> m_vecCheckBoxes;

	std::vector<std::string> m_vecSpinNames, m_vecIntSpinNames, m_vecEditNames,
		m_vecComboNames, m_vecCheckNames;

	QAction *m_pLiveResults = nullptr, *m_pLivePlots = nullptr;

protected:
	void LoadSettings();
	virtual void showEvent(QShowEvent *pEvt) override;

	ResoFocus GetFocus() const;

	void ClearPlot1D();
	void Start1D();
	void Start2D();

public:
	void Load(tl::Prop<std::string>& xml, const std::string& strXmlRoot);
	void Save(std::map<std::string, std::string>& mapConf, const std::string& strXmlRoot);

protected slots:
	void showSqwParamDlg();

	void browseCrysFiles();
	void browseResoFiles();
	void browseSqwFiles();
	void browseScanFiles();

	void SqwModelChanged(int);
	void createSqwModel(const QString& qstrFile);
	void SqwParamsChanged(const std::vector<SqwBase::t_var>&);

	void scanFileChanged(const QString& qstrFile);
	void scanCheckToggled(bool);
	void scaleChanged();

	void SaveResult();

	void Start();		// convolution
	void StartDisp();	// plot dispersion
	void Stop();		// stop convo

	void ChangeHK();
	void ChangeHL();
	void ChangeKL();

	void ShowFavourites();
	void UpdateCurFavPos();
	void ChangePos(const struct FavHklPos& pos);

	virtual void accept() override;

	void Load();
	void Save();

	void ShowAboutDlg();

public:
	ConvoDlg(QWidget* pParent=0, QSettings* pSett=0);
	virtual ~ConvoDlg();

signals:
	void SqwLoaded(const std::vector<SqwBase::t_var>&);
};

#endif
