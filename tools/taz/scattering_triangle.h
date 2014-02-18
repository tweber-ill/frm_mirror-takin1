/*
 * Scattering Triangle
 * @author tweber
 * @date feb-2014
 */

#ifndef __TAZ_SCATT_TRIAG_H__
#define __TAZ_SCATT_TRIAG_H__

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

class ScatteringTriangleScene;
class ScatteringTriangle : public QGraphicsItem
{
	private:
		bool m_bReady;

	protected:
		ScatteringTriangleScene &m_scene;

		ScatteringTriangleNode *m_pNodeKiQ;
		ScatteringTriangleNode *m_pNodeKiKf;
		ScatteringTriangleNode *m_pNodeKfQ;

		double m_dScaleFactor = 75.;	// pixels per A^-1

	protected:
		QRectF boundingRect() const;

	public:
		ScatteringTriangle(ScatteringTriangleScene& scene);
		virtual ~ScatteringTriangle();

		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
		void update(const QRectF& rect = QRectF());

		bool IsReady() const { return m_bReady; }
		double GetTwoTheta() const;
		double GetMonoTwoTheta(double dMonoD) const;
		double GetAnaTwoTheta(double dAnaD) const;
};


class ScatteringTriangleScene : public QGraphicsScene
{	Q_OBJECT
	protected:
		ScatteringTriangle *m_pTri;
		double m_dMonoD = 3.355;
		double m_dAnaD = 3.355;

	public:
		ScatteringTriangleScene();
		virtual ~ScatteringTriangleScene();

		void emitUpdate();
		void SetDs(double dMonoD, double dAnaD);

	public slots:
		void tasChanged(const TriangleOptions& opts);
	signals:
		void triangleChanged(const TriangleOptions& opts);
};


#endif
