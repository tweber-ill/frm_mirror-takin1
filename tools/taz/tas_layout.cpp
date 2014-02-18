/*
 * TAS layout
 * @author tweber
 * @date feb-2014
 */

#include "tas_layout.h"
#include <iostream>


TasLayoutNode::TasLayoutNode(TasLayout* pSupItem) : m_pParentItem(pSupItem)
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
	//std::cout << change << std::endl;
	QVariant var = QGraphicsItem::itemChange(change, value);

	if(change == QGraphicsItem::ItemPositionHasChanged)
		m_pParentItem->nodeMoved(this);

	return var;
}

// --------------------------------------------------------------------------------

TasLayout::TasLayout(TasLayoutScene& scene) : m_scene(scene)
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
	m_pDet->setPos(-100., -m_dLenAnaDet);

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

void TasLayout::nodeMoved(const TasLayoutNode *pNode)
{
	if(m_bNoUpdate)
		return;

	ublas::vector<double> vecMono = qpoint_to_vec(mapFromItem(m_pMono, 0, 0));
	ublas::vector<double> vecSample = qpoint_to_vec(mapFromItem(m_pSample, 0, 0));
	ublas::vector<double> vecAna = qpoint_to_vec(mapFromItem(m_pAna, 0, 0));
	ublas::vector<double> vecDet = qpoint_to_vec(mapFromItem(m_pDet, 0, 0));

	if(pNode == m_pAna)
	{
		ublas::vector<double> vecSampleAna = vecAna-vecSample;
		vecSampleAna /= ublas::norm_2(vecSampleAna);

		ublas::vector<double> vecAnaDet = ublas::prod(rotation_matrix_2d(m_dAnaTwoTheta), vecSampleAna);
		vecAnaDet /= ublas::norm_2(vecAnaDet);
		vecAnaDet *= m_dLenAnaDet;

		m_pDet->setPos(vec_to_qpoint(vecAna+vecAnaDet));


		ublas::vector<double> vecMonoSample = vecSample - vecMono;
		vecMonoSample /= ublas::norm_2(vecMonoSample);

		//m_dTwoTheta = std::acos(ublas::inner_prod(vecMonoSample, vecSampleAna));
		m_dTwoTheta = vec_angle_2(vecSampleAna) - vec_angle_2(vecMonoSample);
		if(m_dTwoTheta < -M_PI) m_dTwoTheta += 2.*M_PI;
		if(m_dTwoTheta > M_PI) m_dTwoTheta -= 2.*M_PI;

		//std::cout << m_dTwoTheta/M_PI*180. << std::endl;

		TriangleOptions opts;
		opts.dTwoTheta = m_dTwoTheta;
		m_scene.emitUpdate(opts);
	}

	this->update();
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

	// arrow heads
	const QLineF* pLines_arrow[] = {&lineKi, &lineKf};
	const QPointF* pPoints_arrow[] = {&ptSample, &ptAna};
	for(unsigned int i=0; i<2; ++i)
	{
		double dAng = (pLines_arrow[i]->angle() - 90.) / 180. * M_PI;
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
	}
}

double TasLayout::GetSampleTwoTheta() const
{
	return m_dTwoTheta;
}

void TasLayout::SetSampleTwoTheta(double dAngle)
{
	m_dTwoTheta = dAngle;
	//std::cout << m_dTwoTheta/M_PI*180. << std::endl;

	ublas::vector<double> vecMono = qpoint_to_vec(mapFromItem(m_pMono, 0, 0));
	ublas::vector<double> vecSample = qpoint_to_vec(mapFromItem(m_pSample, 0, 0));
	ublas::vector<double> vecAna = qpoint_to_vec(mapFromItem(m_pAna, 0, 0));

	ublas::vector<double> vecKi = vecSample - vecMono;
	vecKi /= ublas::norm_2(vecKi);
	double dLenKf = ublas::norm_2(vecAna-vecSample);

	//std::cout << dAngle/M_PI*180. << std::endl;
	ublas::vector<double> vecKf = ublas::prod(rotation_matrix_2d(dAngle), vecKi);
	vecKf /= ublas::norm_2(vecKf);
	vecKf *= dLenKf;

	m_bNoUpdate = 1;
	m_pAna->setPos(vec_to_qpoint(vecSample + vecKf));
	m_bNoUpdate = 0;
	nodeMoved(m_pAna);
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

void TasLayoutScene::triangleChanged(const TriangleOptions& opts)
{
	m_pTas->SetSampleTwoTheta(opts.dTwoTheta);

	update();
}

void TasLayoutScene::emitUpdate(const TriangleOptions& opts)
{
	emit tasChanged(opts);
}

#include "tas_layout.moc"
