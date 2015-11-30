/*
 * resolution calculation
 * @author tweber
 * @date may-2013, apr-2014
 * @license GPLv2
 */

#ifndef __RESO_DLG_H__
#define __RESO_DLG_H__

#include <QSettings>
#include <QDialog>
#include <QLabel>

#include <vector>
#include <map>
#include <string>

#include "ui/ui_reso.h"
#include "cn.h"
#include "pop.h"
#include "eck.h"
#include "tlibs/math/linalg.h"
#include "tlibs/file/xml.h"
#ifndef NO_3D
	#include "helper/plotgl.h"
#endif
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

struct SampleParams
{
	double dLattice[3];
	double dAngles[3];
	double dPlane1[3], dPlane2[3];
};

class ResoDlg : public QDialog, Ui::ResoDlg
{Q_OBJECT
protected:
	std::vector<QDoubleSpinBox*> m_vecSpinBoxes;
	std::vector<std::string> m_vecSpinNames;

	std::vector<QCheckBox*> m_vecCheckBoxes;
	std::vector<std::string> m_vecCheckNames;

	std::vector<QLineEdit*> m_vecEditBoxes;
	std::vector<std::string> m_vecEditNames;

	std::vector<QRadioButton*> m_vecRadioPlus;
	std::vector<QRadioButton*> m_vecRadioMinus;
	std::vector<std::string> m_vecRadioNames;

	std::vector<QComboBox*> m_vecComboBoxes;
	std::vector<std::string> m_vecComboNames;

	void WriteLastConfig();
	void ReadLastConfig();


	// -------------------------------------------------------------------------
	ublas::vector<double> m_vecOrient1, m_vecOrient2;
	ublas::matrix<double> m_matU, m_matB, m_matUinv, m_matBinv;
	ublas::matrix<double> m_matUB, m_matUBinv;
	bool m_bHasUB = 0;
	double m_dAngleQVec0 = 0.;
	// -------------------------------------------------------------------------


	EckParams m_pop;
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
	void Calc();
	void AlgoChanged();

	void SaveRes();
	void LoadRes();

	void ButtonBoxClicked(QAbstractButton*);
	void hideEvent (QHideEvent *event);
	void showEvent(QShowEvent *event);

	void checkAutoCalcElli4dChanged();
	void CalcElli4d();
	void MCGenerate();

protected:
	void setupAlgos();
	void RefreshSimCmd();

public slots:
	void ResoParamsChanged(const ResoParams& params);
	void RecipParamsChanged(const RecipParams& parms);
	void RealParamsChanged(const RealParams& parms);
	void SampleParamsChanged(const SampleParams& parms);

public:
	void Load(tl::Xml& xml, const std::string& strXmlRoot);
	void Save(std::map<std::string, std::string>& mapConf, const std::string& strXmlRoot);

signals:
	void ResoResults(const ublas::matrix<double>& reso, const ublas::vector<double>& Q_avg);
};

#endif
