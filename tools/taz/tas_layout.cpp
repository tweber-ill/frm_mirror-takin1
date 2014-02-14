/*
 * TAS layout
 * @author tweber
 * @date feb-2014
 */

#include "tas_layout.h"

TasLayoutNode::TasLayoutNode(QGraphicsItem* pSupItem) : m_pParentItem(pSupItem)
{
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setCacheMode(QGraphicsItem::DeviceCoordinateCache);
	setCursor(Qt::CrossCursor);
}

QRectF TasLayoutNode::boundingRect() const
{
	return QRectF(-5., -5., 10., 10.);
}

void TasLayoutNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	painter->drawEllipse(QRectF(-2., -2., 4., 4.));
}

QVariant TasLayoutNode::itemChange(GraphicsItemChange change, const QVariant &value)
{
	m_pParentItem->update();
	return QGraphicsItem::itemChange(change, value);
}

// --------------------------------------------------------------------------------

TasLayout::TasLayout(QGraphicsScene& scene)
{
	m_pSrc = new TasLayoutNode(this);
	m_pMono = new TasLayoutNode(this);
	m_pSample = new TasLayoutNode(this);
	m_pAna = new TasLayoutNode(this);
	m_pDet = new TasLayoutNode(this);

	m_pSrc->setPos(150., 150.);
	m_pMono->setPos(0., 150.);
	m_pSample->setPos(0., 0.);
	m_pAna->setPos(-100., 0.);
	m_pDet->setPos(-100., -50.);

	m_pAna->setFlag(QGraphicsItem::ItemIsMovable);

	scene.addItem(m_pSrc);
	scene.addItem(m_pMono);
	scene.addItem(m_pSample);
	scene.addItem(m_pAna);
	scene.addItem(m_pDet);

	setAcceptedMouseButtons(0);
}

TasLayout::~TasLayout()
{
	delete m_pSrc;
	delete m_pMono;
	delete m_pSample;
	delete m_pAna;
	delete m_pDet;
}

QRectF TasLayout::boundingRect() const
{
	return QRectF(-500., -500., 1000., 1000.);
}

void TasLayout::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	QPointF ptSrc = mapFromItem(m_pSrc, 0, 0);
	QPointF ptMono = mapFromItem(m_pMono, 0, 0);
	QPointF ptSample = mapFromItem(m_pSample, 0, 0);
	QPointF ptAna = mapFromItem(m_pAna, 0, 0);
	QPointF ptDet = mapFromItem(m_pDet, 0, 0);

	QLineF lineSrcMono(ptSrc, ptMono);
	QLineF lineKi(ptMono, ptSample);
	QLineF lineKf(ptSample, ptAna);
	QLineF lineAnaDet(ptAna, ptDet);

	painter->drawLine(lineSrcMono);
	painter->drawLine(lineKi);
	painter->drawLine(lineKf);
	painter->drawLine(lineAnaDet);
}


// --------------------------------------------------------------------------------


TasLayoutScene::TasLayoutScene()
{
	m_pTas = new TasLayout(*this);
	this->addItem(m_pTas);
}

TasLayoutScene::~TasLayoutScene()
{
	delete m_pTas;
}
