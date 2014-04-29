/*
 * mieze-tool
 * @author tweber
 * @date 01-may-2013
 */

#ifndef __RESO_DLG_H__
#define __RESO_DLG_H__

#include <QtCore/QSettings>
#include <QtGui/QDialog>
#include <QtGui/QLabel>

#include <vector>
#include <string>

#include "ui/ui_reso.h"
#include "cn.h"
#include "pop.h"
#include "helper/linalg.h"
#include "plot/plotgl.h"
#include "ellipse.h"
#include "dialogs/RecipParamDlg.h"
#include "dialogs/RealParamDlg.h"

// parameters that are not already in RealParams or RecipParams
struct ResoParams
{
	bool bMonoDChanged = 0, bAnaDChanged = 0;
	bool bSensesChanged[3] = {0,0,0};

	double dMonoD, dAnaD;
	bool bScatterSenses[3];
};

class ResoDlg : public QDialog, Ui::ResoDlg
{Q_OBJECT
protected:
	std::vector<QDoubleSpinBox*> m_vecSpinBoxes;
	std::vector<std::string> m_vecSpinNames;

	std::vector<QCheckBox*> m_vecCheckBoxes;
	std::vector<std::string> m_vecCheckNames;

	std::vector<QRadioButton*> m_vecRadioPlus;
	std::vector<QRadioButton*> m_vecRadioMinus;
	std::vector<std::string> m_vecRadioNames;

	void WriteLastConfig();
	void ReadLastConfig();

	PopParams m_pop;
	CNResults m_res;
	bool m_bDontCalc;

	QSettings* m_pSettings = 0;

public:
	ResoDlg(QWidget* pParent, QSettings* pSettings=0);
	virtual ~ResoDlg();

	void EmitResults();

protected slots:
	void UpdateUI();
	void Calc();

	void ButtonBoxClicked(QAbstractButton*);
	void hideEvent (QHideEvent *event);
	void showEvent(QShowEvent *event);

	void SaveFile();
	void LoadFile();

public slots:
	void ResoParamsChanged(const ResoParams& params);
	void RecipParamsChanged(const RecipParams& parms);
	void RealParamsChanged(const RealParams& parms);

signals:
	void ResoResults(const PopParams& pop, const CNResults& res);
};

#endif
