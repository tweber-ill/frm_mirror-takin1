/*
 * resolution calculation
 * @author tweber
 * @date may-2013, apr-2014
 */

#ifndef __RESO_DLG_H__
#define __RESO_DLG_H__

#include <QtCore/QSettings>
#include <QtGui/QDialog>
#include <QtGui/QLabel>

#include <vector>
#include <map>
#include <string>

#include "ui/ui_reso.h"
#include "cn.h"
#include "pop.h"
#include "helper/linalg.h"
#include "helper/xml.h"
#include "helper/plotgl.h"
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

	bool m_bEll4dCurrent = 0;
	Ellipsoid4d m_ell4d;

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

	void checkAutoCalcElli4dChanged();
	void CalcElli4d();
	void MCGenerate();

public slots:
	void ResoParamsChanged(const ResoParams& params);
	void RecipParamsChanged(const RecipParams& parms);
	void RealParamsChanged(const RealParams& parms);

public:
	void Load(Xml& xml, const std::string& strXmlRoot);
	void Save(std::map<std::string, std::string>& mapConf, const std::string& strXmlRoot);

signals:
	void ResoResults(const PopParams& pop, const CNResults& res);
};

#endif
