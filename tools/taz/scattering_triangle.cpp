/*
 * Scattering Triangle
 * @author tweber
 * @date feb-2014
 */

#include "helper/flags.h"
#include "scattering_triangle.h"
#include <iostream>
#include <cmath>
#include <sstream>


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

	m_pNodeKiKf->setFlag(QGraphicsItem::ItemIsMovable);
	m_pNodeKfQ->setFlag(QGraphicsItem::ItemIsMovable);

	m_pNodeKiQ->setPos(0., 0.);
	m_pNodeKiKf->setPos(50., -100.);
	m_pNodeKfQ->setPos(100., 0.);

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

	QPointF ptMid = ptKiQ + (ptKfQ - ptKiQ)/2.;
	ptMid = ptMid + (ptKiKf - ptMid)/2.;

	QLineF lineQ(ptKiQ, ptKfQ);
	QLineF lineKi(ptKiQ, ptKiKf);
	QLineF lineKf(ptKiKf, ptKfQ);

	painter->drawLine(lineQ);
	painter->drawLine(lineKi);
	painter->drawLine(lineKf);

	painter->drawText(ptQMid, "Q");
	painter->drawText(ptKiMid, "ki");
	painter->drawText(ptKfMid, "kf");


	QLineF lineKi2(ptKiKf, ptKiQ);
	QLineF lineQ2(ptKfQ, ptKiQ);

	const QLineF* pLines1[] = {&lineKi2, &lineKi, &lineKf};
	const QLineF* pLines2[] = {&lineQ, &lineKf, &lineQ2};
	const QPointF* pPoints[] = {&ptKiQ, &ptKiKf, &ptKfQ};

	for(unsigned int i=0; i<3; ++i)
	{
		double dArcSize = (pLines1[i]->length() + pLines2[i]->length()) / 2. / 6.;
		double dBeginArcAngle = pLines1[i]->angle() + 180.;
		double dArcAngle = pLines1[i]->angleTo(*pLines2[i]) - 180.;

		painter->drawArc(QRectF(pPoints[i]->x()-dArcSize/2., pPoints[i]->y()-dArcSize/2., dArcSize, dArcSize),
						dBeginArcAngle*16., dArcAngle*16.);

		std::ostringstream ostrAngle;
		ostrAngle.precision(4);
		ostrAngle << std::fabs(dArcAngle) << "\xb0";

		QPointF ptDirOut = *pPoints[i] - ptMid;
		ptDirOut /= std::sqrt(ptDirOut.x()*ptDirOut.x() + ptDirOut.y()*ptDirOut.y());
		ptDirOut *= 15.;

		QPointF ptText = *pPoints[i] + ptDirOut;
		painter->drawText(ptText, ostrAngle.str().c_str());
	}
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
