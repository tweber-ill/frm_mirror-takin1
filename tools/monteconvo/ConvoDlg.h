/*
 * monte carlo convolution tool
 * @author tweber
 * @date aug-2015
 * @license GPLv2
 */

#ifndef __MCONVO_GUI_H__
#define __MCONVO_GUI_H__

#include <QDialog>
#include <QSettings>
#include <thread>
#include <atomic>
#include <memory>

#include "libs/qthelper.h"
#include "SqwParamDlg.h"
#include "ui/ui_monteconvo.h"
#include "sqw.h"
#include "../res/defs.h"


class ConvoDlg : public QDialog, Ui::ConvoDlg
{ Q_OBJECT
protected:
	std::thread *m_pth = nullptr;
	std::atomic<bool> m_atStop;

	QSettings *m_pSett = nullptr;
	SqwParamDlg *m_pSqwParamDlg = nullptr;

	SqwBase *m_pSqw = nullptr;
	std::vector<t_real_reso> m_vecQ, m_vecS;
	std::unique_ptr<QwtPlotWrapper> m_plotwrap;

protected:
	void LoadSettings();
	virtual void showEvent(QShowEvent *pEvt) override;

protected slots:
	void showSqwParamDlg();

	void browseCrysFiles();
	void browseResoFiles();
	void browseSqwFiles();

	void SqwModelChanged(int);
	void createSqwModel(const QString& qstrFile);
	void SqwParamsChanged(const std::vector<SqwBase::t_var>&);

	void SaveResult();

	void Start();
	void Stop();

	void ButtonBoxClicked(QAbstractButton *pBtn);

public:
	ConvoDlg(QWidget* pParent=0, QSettings* pSett=0);
	virtual ~ConvoDlg();

signals:
	void SqwLoaded(const std::vector<SqwBase::t_var>&);
};

#endif
