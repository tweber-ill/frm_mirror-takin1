/*
 * mieze-tool
 * @author tweber
 * @date 01-may-2013
 */

#ifndef __RESO_DLG_H__
#define __RESO_DLG_H__

#include <QtGui/QDialog>
#include <QtGui/QLabel>
#include <vector>
#include <string>

#include "ui/ui_reso.h"
#include "cn.h"
#include "pop.h"
#include "helper/linalg.h"
#include "../../plot/plotgl.h"
#include "ellipse.h"

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

public:
	ResoDlg(QWidget* pParent);
	virtual ~ResoDlg();

protected slots:
	void UpdateUI();
	void Calc();

	void ButtonBoxClicked(QAbstractButton*);
	void hideEvent (QHideEvent *event);
	void showEvent(QShowEvent *event);

	void SaveFile();
	void LoadFile();
};

#endif
