/*
 * 3D Ellipsoid Dialog
 * @author Tobias Weber
 * @date may-2013, 29-apr-2014
 * @license GPLv2
 */

#ifndef __RESO_ELLI_DLG_3D__
#define __RESO_ELLI_DLG_3D__

#include <QDialog>
#include <QSettings>

#include <vector>

#include "helper/plotgl.h"
#include "tlibs/math/linalg.h"
#include "tools/res/ellipse.h"
#include "tools/res/pop.h"

class EllipseDlg3D : public QDialog
{Q_OBJECT
	protected:
		std::vector<PlotGl*> m_pPlots;
		std::vector<Ellipsoid> m_elliProj;
		std::vector<Ellipsoid> m_elliSlice;

		QSettings *m_pSettings = 0;

		ublas::vector<double>
		ProjRotatedVec(const ublas::matrix<double>& rot,
								const ublas::vector<double>& vec);

	public:
		EllipseDlg3D(QWidget* pParent, QSettings *pSett=0);
		virtual ~EllipseDlg3D();

	protected:
		void hideEvent(QHideEvent *event);
		void showEvent(QShowEvent *event);

	public slots:
		void SetParams(const ublas::matrix<double>& reso, const ublas::vector<double>& Q_avg);
};

#endif
