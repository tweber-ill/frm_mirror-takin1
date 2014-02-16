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


ScatteringTriangleNode::ScatteringTriangleNode(ScatteringTriangle* pSupItem) : m_pParentItem(pSupItem)
{
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setCacheMode(QGraphicsItem::DeviceCoordinateCache);
	setCursor(Qt::CrossCursor);
}

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


// --------------------------------------------------------------------------------


ScatteringTriangle::ScatteringTriangle(ScatteringTriangleScene& scene)
				: m_bReady(0), m_scene(scene)
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

	m_bReady = 1;
}

ScatteringTriangle::~ScatteringTriangle()
{
	delete m_pNodeKiQ;
	delete m_pNodeKiKf;
	delete m_pNodeKfQ;
}

void ScatteringTriangle::update(const QRectF& rect)
{
	QGraphicsItem::update();
	m_scene.updatedTriangle();
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
	QLineF lineKf2(ptKfQ, ptKiKf);
	QLineF lineQ2(ptKfQ, ptKiQ);

	const QLineF* pLines1[] = {&lineKi2, &lineKi, &lineKf};
	const QLineF* pLines2[] = {&lineQ, &lineKf, &lineQ2};
	const QPointF* pPoints[] = {&ptKiQ, &ptKiKf, &ptKfQ};

	const QLineF* pLines_arrow[] = {&lineKi, &lineKf2, &lineQ};
	const QPointF* pPoints_arrow[] = {&ptKiQ, &ptKfQ, &ptKiQ};

	for(unsigned int i=0; i<3; ++i)
	{
		// arrow heads
		double dAng = (pLines_arrow[i]->angle() + 90.) / 180. * M_PI;
		double dC = std::cos(dAng);
		double dS = std::sin(dAng);

		double dTriagX = 5., dTriagY = 10.;
		QPointF ptTriag1 = *pPoints_arrow[i] + QPointF(dTriagX*dC + dTriagY*dS, -dTriagX*dS + dTriagY*dC);
		QPointF ptTriag2 = *pPoints_arrow[i] + QPointF(-dTriagX*dC + dTriagY*dS, dTriagX*dS + dTriagY*dC);

		QPainterPath triag;
		triag.moveTo(*pPoints_arrow[i]);
		triag.lineTo(ptTriag1);
		triag.lineTo(ptTriag2);

		painter->setPen(Qt::black);
		painter->fillPath(triag, Qt::black);


		// angle arcs
		double dArcSize = (pLines1[i]->length() + pLines2[i]->length()) / 2. / 4.;
		double dBeginArcAngle = pLines1[i]->angle() + 180.;
		double dArcAngle = pLines1[i]->angleTo(*pLines2[i]) - 180.;

		painter->setPen(Qt::blue);
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

double ScatteringTriangle::GetTwoTheta() const
{
	QPointF ptKiQ = mapFromItem(m_pNodeKiQ, 0, 0);
	QPointF ptKfQ = mapFromItem(m_pNodeKfQ, 0, 0);
	QPointF ptKiKf = mapFromItem(m_pNodeKiKf, 0, 0);

	QLineF lineKi(ptKiKf, ptKiQ);
	QLineF lineKf(ptKiKf, ptKfQ);

	double dTT = lineKi.angleTo(lineKf) / 180. * M_PI;
	if(dTT > M_PI)
		dTT = -(2.*M_PI - dTT);

	return dTT;
}

// --------------------------------------------------------------------------------


ScatteringTriangleScene::ScatteringTriangleScene() : m_pTri(0)
{
	m_pTri = new ScatteringTriangle(*this);
	this->addItem(m_pTri);
}

ScatteringTriangleScene::~ScatteringTriangleScene()
{
	delete m_pTri;
}

void ScatteringTriangleScene::updatedTriangle()
{
	if(!m_pTri || !m_pTri->IsReady())
		return;

	TriangleOptions opts;
	opts.dTwoTheta = m_pTri->GetTwoTheta();

	emit triangleChanged(opts);
}

void ScatteringTriangleScene::wheelEvent(QGraphicsSceneWheelEvent *pEvt)
{
	//pEvt->ignore();
	QGraphicsScene::wheelEvent(pEvt);
}

void ScatteringTriangleScene::mousePressEvent(QGraphicsSceneMouseEvent *pEvt)
{
	//if(pEvt->buttons() & Qt::RightButton)
	//{}

	QGraphicsScene::mousePressEvent(pEvt);
}

void ScatteringTriangleScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *pEvt)
{
	//if((pEvt->buttons() & Qt::RightButton) == 0)
	//{}

	QGraphicsScene::mouseReleaseEvent(pEvt);
}

void ScatteringTriangleScene::mouseMoveEvent(QGraphicsSceneMouseEvent *pEvt)
{
	QGraphicsScene::mouseMoveEvent(pEvt);
}


#include "scattering_triangle.moc"
