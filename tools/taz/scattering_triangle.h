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


class ScatteringTriangleNode : public QGraphicsItem
{
	protected:
		QGraphicsItem *m_pParentItem;

	protected:
		QRectF boundingRect() const;
		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

		void mousePressEvent(QGraphicsSceneMouseEvent *pEvt);
		void mouseReleaseEvent(QGraphicsSceneMouseEvent *pEvt);
		QVariant itemChange(GraphicsItemChange change, const QVariant &value);

	public:
		ScatteringTriangleNode(QGraphicsItem* pSupItem);
};

class ScatteringTriangle : public QGraphicsItem
{
	protected:
		ScatteringTriangleNode *m_pNodeKiQ;
		ScatteringTriangleNode *m_pNodeKiKf;
		ScatteringTriangleNode *m_pNodeKfQ;
		double m_dPixelsPerInvA;

	protected:
		QRectF boundingRect() const;

	public:
		ScatteringTriangle(QGraphicsScene& scene);

		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};


class ScatteringTriangleScene : public QGraphicsScene
{
	protected:
		ScatteringTriangle *m_pTri;
		bool m_bMouseScale;
		double m_dMouseScaleBegin = 0.;

	public:
		ScatteringTriangleScene();
		virtual ~ScatteringTriangleScene();

		void wheelEvent(QWheelEvent *pEvt);
		void mousePressEvent(QGraphicsSceneMouseEvent *pEvt);
		void mouseReleaseEvent(QGraphicsSceneMouseEvent *pEvt);
		void mouseMoveEvent(QGraphicsSceneMouseEvent *pEvt);
};


#endif
