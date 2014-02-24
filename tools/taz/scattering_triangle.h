/*
 * Scattering Triangle
 * @author tweber
 * @date feb-2014
 */

#ifndef __TAZ_SCATT_TRIAG_H__
#define __TAZ_SCATT_TRIAG_H__

#include "helper/linalg.h"
#include "helper/lattice.h"

#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsItem>
#include <QtGui/QGraphicsSceneDragDropEvent>
#include <QtGui/QGraphicsTextItem>
#include <QtGui/QWheelEvent>

#include "tasoptions.h"

class ScatteringTriangle;
class ScatteringTriangleNode : public QGraphicsItem
{
	protected:
		ScatteringTriangle *m_pParentItem;

	protected:
		QRectF boundingRect() const;
		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

		void mousePressEvent(QGraphicsSceneMouseEvent *pEvt);
		void mouseReleaseEvent(QGraphicsSceneMouseEvent *pEvt);
		QVariant itemChange(GraphicsItemChange change, const QVariant &value);

	public:
		ScatteringTriangleNode(ScatteringTriangle* pSupItem);
};

class RecipPeak : public QGraphicsItem
{
	protected:
		QRectF boundingRect() const;
		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

	public:
		RecipPeak();
};

class ScatteringTriangleScene;
class ScatteringTriangle : public QGraphicsItem
{
	private:
		bool m_bReady = 0;

	protected:
		ScatteringTriangleScene &m_scene;

		ScatteringTriangleNode *m_pNodeKiQ;
		ScatteringTriangleNode *m_pNodeKiKf;
		ScatteringTriangleNode *m_pNodeKfQ;

		double m_dScaleFactor = 75.;	// pixels per A^-1

		std::vector<RecipPeak*> m_vecPeaks;
		void ClearPeaks();

	protected:
		QRectF boundingRect() const;

	public:
		ScatteringTriangle(ScatteringTriangleScene& scene);
		virtual ~ScatteringTriangle();

		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
		void nodeMoved(const ScatteringTriangleNode* pNode=0);

		bool IsReady() const { return m_bReady; }
		double GetTwoTheta() const;
		double GetMonoTwoTheta(double dMonoD) const;
		double GetAnaTwoTheta(double dAnaD) const;

		void SetTwoTheta(double dTT);

	public:
		void CalcPeaks(const Lattice& lattice, const Plane<double>& plane);
};


class ScatteringTriangleScene : public QGraphicsScene
{	Q_OBJECT
	protected:
		ScatteringTriangle *m_pTri;
		double m_dMonoD = 3.355;
		double m_dAnaD = 3.355;

		bool m_bDontEmitChange = 0;

	public:
		ScatteringTriangleScene();
		virtual ~ScatteringTriangleScene();

		void emitUpdate();
		void SetDs(double dMonoD, double dAnaD);

		void CalcPeaks(const Lattice& lattice, const Plane<double>& plane);

	public slots:
		void tasChanged(const TriangleOptions& opts);
	signals:
		void triangleChanged(const TriangleOptions& opts);
};


#endif
