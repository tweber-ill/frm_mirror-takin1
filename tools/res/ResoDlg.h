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
#include "../../plot/plot.h"
#include "../../plot/plotgl.h"
#include "ellipse.h"


class EllipseDlg : public QDialog
{Q_OBJECT
protected:
	std::vector<Plot*> m_pPlots;
	std::vector<Ellipse> m_elliProj;
	std::vector<Ellipse> m_elliSlice;

	QLabel *m_pLabelStatus;

	virtual void paintEvent (QPaintEvent *pEvent);

public:
	EllipseDlg(QWidget* pParent);
	virtual ~EllipseDlg();

	void SetParams(const PopParams& pop, const CNResults& res);

protected slots:
	void hideEvent (QHideEvent *event);
	void showEvent(QShowEvent *event);

	void SetStatusMsg(const char* pcMsg, int iPos);
};

class EllipseDlg3D : public QDialog
{Q_OBJECT
protected:
	std::vector<PlotGl*> m_pPlots;
	std::vector<Ellipsoid> m_elliProj;
	std::vector<Ellipsoid> m_elliSlice;

	ublas::vector<double>
	ProjRotatedVec(const ublas::matrix<double>& rot,
							const ublas::vector<double>& vec);

public:
	EllipseDlg3D(QWidget* pParent);
	virtual ~EllipseDlg3D();

	void SetParams(const PopParams& pop, const CNResults& res);

protected slots:
	void hideEvent(QHideEvent *event);
	void showEvent(QShowEvent *event);
};


class InstLayoutDlg : public QDialog
{Q_OBJECT
protected:
	double m_dMonoTheta, m_dMono2Theta;
	double m_dSampleTheta, m_dSample2Theta;
	double m_dAnaTheta, m_dAna2Theta;

	double m_dDistMonoSample, m_dDistSampleAna, m_dDistAnaDet;

	double m_dPosMonoX, m_dPosMonoY;

	double m_dMonoW, m_dMonoD;
	double m_dAnaW, m_dAnaD;
	double m_dDetW, m_dDetD;
	double m_dSamp_Q, m_dSamp_perpQ;

	double m_dWidth, m_dHeight;

	virtual void paintEvent (QPaintEvent *pEvent);

public:
	InstLayoutDlg(QWidget* pParent);
	virtual ~InstLayoutDlg();

	void SetParams(const PopParams& pop, const CNResults& res);

protected slots:
	void hideEvent (QHideEvent *event);
	void showEvent(QShowEvent *event);
};


class ScatterTriagDlg : public QDialog
{Q_OBJECT
protected:
	ublas::vector<double> m_vec_ki, m_vec_kf, m_vec_Q;
	double m_d2Theta, m_dKiQ;

	virtual void paintEvent (QPaintEvent *pEvent);

public:
	ScatterTriagDlg(QWidget* pParent);
	virtual ~ScatterTriagDlg();

	void SetParams(const PopParams& pop, const CNResults& res);

protected slots:
	void hideEvent (QHideEvent *event);
	void showEvent(QShowEvent *event);
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

	EllipseDlg *m_pElliDlg;
	EllipseDlg3D *m_pElli3dDlg;
	InstLayoutDlg *m_pInstDlg;
	ScatterTriagDlg *m_pScatterDlg;

public:
	ResoDlg(QWidget* pParent);
	virtual ~ResoDlg();

protected slots:
	void UpdateUI();
	void Calc();

	void ShowEllipses();
	void ShowEllipses3d();
	void ShowInstrLayout();
	void ShowScatterTriag();

	void ButtonBoxClicked(QAbstractButton*);
	void hideEvent (QHideEvent *event);
	void showEvent(QShowEvent *event);

	void SaveFile();
	void LoadFile();
};

#endif
