/*
 * Goto Dialog
 * @author Tobias Weber
 * @date 15-oct-2014
 */

#ifndef __GOTO_DLG_H__
#define __GOTO_DLG_H__

#include <QtGui/QDialog>
#include <QtCore/QSettings>
#include "../ui/ui_goto.h"

#include "../tlibs/math/lattice.h"
#include "../tlibs/math/linalg.h"
#include "../tools/taz/tasoptions.h"
#include "RecipParamDlg.h"

class GotoDlg : public QDialog, Ui::GotoDlg
{ Q_OBJECT
	private:
		bool m_bMonoAnaOk = 0;
		bool m_bSampleOk = 0;

	public:
		GotoDlg(QWidget* pParent=0, QSettings* pSett=0);
		virtual ~GotoDlg();

	protected:
		QSettings *m_pSettings = 0;

		ublas::vector<double> m_vec1, m_vec2;
		tl::Lattice<double> m_lattice;
		RecipParams m_paramsRecip;
		bool m_bHasParamsRecip = 0;

		double m_dMono2Theta;
		double m_dSample2Theta;
		double m_dSampleTheta;
		double m_dAna2Theta;

		double m_dAna = 3.355;
		double m_dMono = 3.355;

		bool m_bSenseM=0, m_bSenseS=1, m_bSenseA=0;

	protected slots:
		void EditedKiKf();
		void EditedE();
		void EditedAngles();

		void GetCurPos();

	public slots:
		void CalcMonoAna();
		void CalcSample();

	public:
		void SetLattice(const tl::Lattice<double>& lattice) { m_lattice = lattice; }
		void SetScatteringPlane(const ublas::vector<double>& vec1, const ublas::vector<double>& vec2)
		{ m_vec1 = vec1; m_vec2 = vec2; }

		void SetD(double dMono, double dAna) { m_dMono = dMono; m_dAna = dAna; }
		void SetSenses(bool bM, bool bS, bool bA)
		{ m_bSenseM=bM; m_bSenseS=bS; m_bSenseA=bA; }

		void SetMonoSense(bool bSense) { m_bSenseM = bSense; }
		void SetSampleSense(bool bSense) { m_bSenseS = bSense; }
		void SetAnaSense(bool bSense) { m_bSenseA = bSense; }

	protected slots:
		void ButtonBoxClicked(QAbstractButton* pBtn);
	public slots:
		void RecipParamsChanged(const RecipParams&);

	signals:
		void vars_changed(const CrystalOptions& crys, const TriangleOptions& triag);

	protected:
		virtual void showEvent(QShowEvent *pEvt);
};

#endif
