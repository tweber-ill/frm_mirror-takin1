/*
 * Scattering Triangle
 * @author tweber
 * @date feb-2014
 */

#include "scattering_triangle.h"
#include <iostream>


QRectF ScatteringTriangleNode::boundingRect() const
{
	return QRectF(-5., -5., 10., 10.);
}

void ScatteringTriangleNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	painter->drawEllipse(QRectF(-2., -2., 4., 4.));
}

void ScatteringTriangleNode::mousePressEvent(QGraphicsSceneMouseEvent *pEvt)
{
	//setCursor(Qt::ClosedHandCursor);
	QGraphicsItem::mousePressEvent(pEvt);
}

void ScatteringTriangleNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *pEvt)
{
	//setCursor(Qt::OpenHandCursor);
	QGraphicsItem::mouseReleaseEvent(pEvt);
}

QVariant ScatteringTriangleNode::itemChange(GraphicsItemChange change, const QVariant &value)
{
	m_pParentItem->update();
	return QGraphicsItem::itemChange(change, value);
}

ScatteringTriangleNode::ScatteringTriangleNode(QGraphicsItem* pSupItem) : m_pParentItem(pSupItem)
{
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setCacheMode(QGraphicsItem::DeviceCoordinateCache);

	setCursor(Qt::CrossCursor);
}


// --------------------------------------------------------------------------------


ScatteringTriangle::ScatteringTriangle(QGraphicsScene& scene) : m_dPixelsPerInvA(100.)
{
	m_pNodeKiQ = new ScatteringTriangleNode(this);
	m_pNodeKiKf = new ScatteringTriangleNode(this);
	m_pNodeKfQ = new ScatteringTriangleNode(this);

	m_pNodeKiQ->setPos(-200., 0.);
	m_pNodeKiKf->setPos(0., -200.);
	m_pNodeKfQ->setPos(200., 200.);

	scene.addItem(m_pNodeKiQ);
	scene.addItem(m_pNodeKiKf);
	scene.addItem(m_pNodeKfQ);

	setAcceptedMouseButtons(0);
}

QRectF ScatteringTriangle::boundingRect() const
{
	return QRectF(-500., -500., 1000., 1000.);
}

void ScatteringTriangle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	QPointF ptKiQ = mapFromItem(m_pNodeKiQ, 0, 0);
	QPointF ptKfQ = mapFromItem(m_pNodeKfQ, 0, 0);
	QPointF ptKiKf = mapFromItem(m_pNodeKiKf, 0, 0);

	QPointF ptQMid = ptKiQ + (ptKfQ - ptKiQ)/2.;
	QPointF ptKiMid = ptKiQ + (ptKiKf - ptKiQ)/2.;
	QPointF ptKfMid = ptKfQ + (ptKiKf - ptKfQ)/2.;

	QLineF lineQ(ptKiQ, ptKfQ);
	QLineF lineKi(ptKiQ, ptKiKf);
	QLineF lineKf(ptKiKf, ptKfQ);

	painter->drawLine(lineQ);
	painter->drawLine(lineKi);
	painter->drawLine(lineKf);

	painter->drawText(ptQMid, "Q");
	painter->drawText(ptKiMid, "ki");
	painter->drawText(ptKfMid, "kf");
}


// --------------------------------------------------------------------------------


ScatteringTriangleScene::ScatteringTriangleScene()
	: m_bMouseScale(0)
{
	m_pTri = new ScatteringTriangle(*this);
	this->addItem(m_pTri);
}

ScatteringTriangleScene::~ScatteringTriangleScene()
{
	delete m_pTri;
}

void ScatteringTriangleScene::wheelEvent(QWheelEvent *pEvt)
{
	pEvt->ignore();
}

void ScatteringTriangleScene::mousePressEvent(QGraphicsSceneMouseEvent *pEvt)
{
	if(pEvt->buttons() & Qt::RightButton)
	{
		m_dMouseScaleBegin = pEvt->pos().y();

		m_bMouseScale = 0;
	}

	QGraphicsScene::mousePressEvent(pEvt);
}

void ScatteringTriangleScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *pEvt)
{
	if((pEvt->buttons() & Qt::RightButton) == 0)
	{
		m_bMouseScale = 0;
	}

	QGraphicsScene::mouseReleaseEvent(pEvt);
}

void ScatteringTriangleScene::mouseMoveEvent(QGraphicsSceneMouseEvent *pEvt)
{
	if(m_bMouseScale)
	{
		double dNewY = pEvt->pos().y();

		double dy = dNewY - m_dMouseScaleBegin;

		if(dy != 0.)
		{
			m_dMouseScaleBegin = dNewY;
		}
	}

	QGraphicsScene::mouseMoveEvent(pEvt);
}
