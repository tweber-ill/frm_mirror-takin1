/*
 * Real crystal lattice
 * @author tweber
 * @date feb-2014
 * @license GPLv2
 */

#ifndef __TAZ_REAL_LATTICE_H__
#define __TAZ_REAL_LATTICE_H__

#include "tlibs/math/linalg.h"
#include "tlibs/math/lattice.h"
#include "tlibs/math/bz.h"
#include "tlibs/math/neutrons.hpp"
#include "tlibs/math/kd.h"
#include "tasoptions.h"
#include "dialogs/AtomsDlg.h"
#include "helper/spacegroup.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsTextItem>
#include <QWheelEvent>
#if QT_VER>=5
	#include <QtWidgets>
#endif

namespace ublas = boost::numeric::ublas;


class RealLattice;

class LatticePoint : public QGraphicsItem
{
	protected:
		QColor m_color = Qt::red;
		QRectF boundingRect() const;
		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

	protected:
		QString m_strLabel;

	public:
		LatticePoint();

		void SetLabel(const QString& str) { m_strLabel = str; }
		void SetColor(const QColor& col) { m_color = col; }
};


class LatticeAtom : public QGraphicsItem
{
	friend class RealLattice;

	protected:
		QColor m_color = Qt::cyan;
		QRectF boundingRect() const;
		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

	protected:
		std::string m_strElem;
		ublas::vector<double> m_vecPos;
		ublas::vector<double> m_vecProj;
		double m_dProjDist = 0.;

	public:
		LatticeAtom();
		void SetColor(const QColor& col) { m_color = col; }
};


class LatticeScene;
class RealLattice : public QGraphicsItem
{
	protected:
		bool m_bReady = 0;
		LatticeScene &m_scene;

		double m_dScaleFactor = 48.;	// pixels per A for zoom == 1.
		double m_dZoom = 1.;
		double m_dPlaneDistTolerance = 0.01;
		int m_iMaxPeaks = 7;

		tl::Lattice<double> m_lattice;
		ublas::matrix<double> m_matPlane, m_matPlane_inv;
		std::vector<LatticePoint*> m_vecPeaks;
		std::vector<LatticeAtom*> m_vecAtoms;

		tl::Kd<double> m_kdLattice;

		bool m_bShowWS = 1;
		tl::Brillouin2D<double> m_ws;	// Wigner-Seitz cell

	protected:
		QRectF boundingRect() const;

	public:
		RealLattice(LatticeScene& scene);
		virtual ~RealLattice();

		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

		void SetReady(bool bReady) { m_bReady = bReady; }
		bool IsReady() const { return m_bReady; }

		const ublas::matrix<double>& GetPlane() const { return m_matPlane; }

	public:
		bool HasPeaks() const { return m_vecPeaks.size()!=0 && m_lattice.IsInited(); }
		void ClearPeaks();
		void CalcPeaks(const tl::Lattice<double>& lattice, const tl::Plane<double>& planeFrac,
			const SpaceGroup* pSpaceGroup=nullptr, const std::vector<AtomPos>* pvecAtomPos=nullptr);

		void SetPlaneDistTolerance(double dTol) { m_dPlaneDistTolerance = dTol; }
		void SetMaxPeaks(int iMax) { m_iMaxPeaks = iMax; }
		unsigned int GetMaxPeaks() const { return m_iMaxPeaks; }
		void SetZoom(double dZoom);
		double GetZoom() const { return m_dZoom; }

		void SetWSVisible(bool bVisible);

		const tl::Kd<double>& GetKdLattice() const { return m_kdLattice; }

	public:
		double GetScaleFactor() const { return m_dScaleFactor; }
		void SetScaleFactor(double dScale) { m_dScaleFactor = dScale; }

		ublas::vector<double> GetHKLFromPlanePos(double x, double y) const;
		const tl::Lattice<double>& GetRealLattice() const { return m_lattice; }
};


class LatticeScene : public QGraphicsScene
{	Q_OBJECT
	protected:
		RealLattice *m_pLatt;

		bool m_bDontEmitChange = 0;
		bool m_bSnap = 0;
		bool m_bMousePressed = 0;

	public:
		LatticeScene();
		virtual ~LatticeScene();

		void SetEmitChanges(bool bEmit) { m_bDontEmitChange = !bEmit; }
		// emits triangleChanged
		void emitUpdate();
		// emits paramsChanged
		void emitAllParams();

		const RealLattice* GetLattice() const { return m_pLatt; }
		RealLattice* GetLattice() { return m_pLatt; }

		bool ExportWSAccurate(const char* pcFile) const;

	public slots:
		void scaleChanged(double dTotalScale);

	signals:
		void coordsChanged(double dh, double dk, double dl,
			bool bHasNearest,
			double dNearestH, double dNearestK, double dNearestL);

	protected:
		virtual void mousePressEvent(QGraphicsSceneMouseEvent *pEvt) override;
		virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *pEvt) override;
		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *pEvt) override;

		virtual void keyPressEvent(QKeyEvent *pEvt) override;
		virtual void keyReleaseEvent(QKeyEvent *pEvt) override;

		virtual void drawBackground(QPainter*, const QRectF&) override;
};


class LatticeView : public QGraphicsView
{
	Q_OBJECT
	protected:
		double m_dTotalScale = 1.;
		virtual void wheelEvent(QWheelEvent* pEvt);

	public:
		LatticeView(QWidget* pParent = 0);
		virtual ~LatticeView();

	signals:
		void scaleChanged(double dTotalScale);
};

#endif
