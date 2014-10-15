/*
 * Goto Dialog
 * @author Tobias Weber
 * @date 15-oct-2014
 */

#ifndef __GOTO_DLG_H__
#define __GOTO_DLG_H__

#include <QtGui/QDialog>
#include "../ui/ui_goto.h"

#include "../helper/lattice.h"
#include "../helper/linalg.h"


class GotoDlg : public QDialog, Ui::GotoDlg
{ Q_OBJECT
	public:
		GotoDlg(QWidget* pParent=0);
		virtual ~GotoDlg();

	protected:
		ublas::vector<double> m_vec1, m_vec2;
		Lattice m_lattice;

		double m_dAna = 3.355;
		double m_dMono = 3.355;

		bool m_bSenseM=0, m_bSenseS=1, m_bSenseA=0;

	protected slots:
		void EditedKiKf();
		void EditedE();

	public slots:
		void CalcMonoAna();
		void CalcSample();

	public:
		void SetLattice(const Lattice& lattice) { m_lattice = lattice; }
		void SetScatteringPlane(const ublas::vector<double>& vec1, const ublas::vector<double>& vec2)
		{ m_vec1 = vec1; m_vec2 = vec2; }

		void SetD(double dMono, double dAna) { m_dMono = dMono; m_dAna = dAna; }
		void SetSenses(bool bM, bool bS, bool bA)
		{ m_bSenseM=bM; m_bSenseS=bS; m_bSenseA=bA; }

		void SetMonoSense(bool bSense) { m_bSenseM = bSense; }
		void SetSampleSense(bool bSense) { m_bSenseS = bSense; }
		void SetAnaSense(bool bSense) { m_bSenseA = bSense; }
};

#endif
