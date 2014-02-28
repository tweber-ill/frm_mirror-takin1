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
	private:
		bool m_bReady = 0;

	protected:
		TasLayoutScene& m_scene;

		TasLayoutNode *m_pSrc = 0;
		TasLayoutNode *m_pMono = 0;
		TasLayoutNode *m_pSample = 0;
		TasLayoutNode *m_pAna = 0;
		TasLayoutNode *m_pDet = 0;

		double m_dMonoTwoTheta = 3.1415/2.;
		double m_dAnaTwoTheta = 3.1415/2.;
		double m_dTwoTheta = 3.1415/2.;
		double m_dTheta = 3.1415/4.;

		double m_dLenMonoSample = 150.;
		double m_dLenSampleAna = 100.;
		double m_dLenAnaDet = 50.;
		double m_dLenSample = 25.;

		bool m_bAllowLengthChanges = 1;

	protected:
		QRectF boundingRect() const;

	public:
		TasLayout(TasLayoutScene& scene);
		virtual ~TasLayout();

		bool IsReady() const { return m_bReady; }

		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

		void SetSampleTwoTheta(double dTT);
		void SetSampleTheta(double dT);
		void SetMonoTwoTheta(double dTT);
		void SetAnaTwoTheta(double dTT);

		void nodeMoved(const TasLayoutNode* pNode=0);

		void AllowLengthChanges(bool bAllow) { m_bAllowLengthChanges = bAllow; };
};


class TasLayoutScene : public QGraphicsScene
{	Q_OBJECT
	protected:
		TasLayout *m_pTas;

		bool m_bDontEmitChange = 1;

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
