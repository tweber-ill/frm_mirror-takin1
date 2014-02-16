/*
 * TAS layout
 * @author tweber
 * @date feb-2014
 */

#ifndef __TAS_LAYOUT_H__
#define __TAS_LAYOUT_H__

#include "helper/flags.h"
#include <cmath>

#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsItem>
#include <QtGui/QGraphicsSceneDragDropEvent>
#include <QtGui/QGraphicsTextItem>
#include <QtGui/QWheelEvent>

#include "tasoptions.h"

class TasLayoutNode : public QGraphicsItem
{
	protected:
		QGraphicsItem *m_pParentItem;

	protected:
		QRectF boundingRect() const;
		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

		QVariant itemChange(GraphicsItemChange change, const QVariant &value);

	public:
		TasLayoutNode(QGraphicsItem* pSupItem);
};


class TasLayout : public QGraphicsItem
{
	protected:
		TasLayoutNode *m_pSrc;
		TasLayoutNode *m_pMono;
		TasLayoutNode *m_pSample;
		TasLayoutNode *m_pAna;
		TasLayoutNode *m_pDet;

		double m_dAngleMonoTT = M_PI/2.;
		double m_dAngleSampleTT = M_PI/2.;
		double m_dAngleAnaTT = M_PI/2.;

	protected:
		QRectF boundingRect() const;

	public:
		TasLayout(QGraphicsScene& scene);
		virtual ~TasLayout();

		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

class TasLayoutScene : public QGraphicsScene
{	Q_OBJECT
	protected:
		TasLayout *m_pTas;

	public:
		TasLayoutScene();
		virtual ~TasLayoutScene();

		/*void mousePressEvent(QGraphicsSceneMouseEvent *pEvt);
		void mouseReleaseEvent(QGraphicsSceneMouseEvent *pEvt);
		void mouseMoveEvent(QGraphicsSceneMouseEvent *pEvt);*/

	public slots:
		void triangleChanged(const TriangleOptions& opts);
};

#endif
