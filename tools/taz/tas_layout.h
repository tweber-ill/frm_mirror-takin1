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

class TasLayout;
class TasLayoutNode : public QGraphicsItem
{
	protected:
		TasLayout *m_pParentItem;

	protected:
		QRectF boundingRect() const;
		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

		QVariant itemChange(GraphicsItemChange change, const QVariant &value);

	public:
		TasLayoutNode(TasLayout* pSupItem);
};


class TasLayoutScene;
class TasLayout : public QGraphicsItem
{
	protected:
		TasLayoutScene& m_scene;

		bool m_bNoUpdate = 0;

		TasLayoutNode *m_pSrc;
		TasLayoutNode *m_pMono;
		TasLayoutNode *m_pSample;
		TasLayoutNode *m_pAna;
		TasLayoutNode *m_pDet;

		double m_dMonoTwoTheta = 3.1415/2.;
		double m_dAnaTwoTheta = 3.1415/2.;
		double m_dTwoTheta = 3.1415/2.;

		double m_dLenMonoSample = 150.;
		double m_dLenSampleAna = 100.;
		double m_dLenAnaDet = 50.;

	protected:
		QRectF boundingRect() const;

	public:
		TasLayout(TasLayoutScene& scene);
		virtual ~TasLayout();

		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

		void SetSampleTwoTheta(double dTT);
		void SetMonoTwoTheta(double dTT);
		void SetAnaTwoTheta(double dTT);

		void nodeMoved(const TasLayoutNode* pNode=0);
};


class TasLayoutScene : public QGraphicsScene
{	Q_OBJECT
	protected:
		TasLayout *m_pTas;

	public:
		TasLayoutScene();
		virtual ~TasLayoutScene();

		void emitUpdate(const TriangleOptions& opts);

	public slots:
		void triangleChanged(const TriangleOptions& opts);
	signals:
		void tasChanged(const TriangleOptions& opts);
};

#endif
