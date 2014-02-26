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

	m_pSrc->setToolTip("Source");
	m_pMono->setToolTip("Monochromator");
	m_pSample->setToolTip("Sample");
	m_pAna->setToolTip("Analyser");
	m_pDet->setToolTip("Detector");

	m_pSrc->setPos(150., 150.);
	m_pMono->setPos(0., m_dLenMonoSample);
	m_pSample->setPos(0., 0.);
	m_pAna->setPos(-m_dLenSampleAna, 0.);
	m_pDet->setPos(-100., -m_dLenAnaDet);

	m_pSample->setFlag(QGraphicsItem::ItemIsMovable);
	m_pAna->setFlag(QGraphicsItem::ItemIsMovable);
	m_pDet->setFlag(QGraphicsItem::ItemIsMovable);

	scene.addItem(m_pSrc);
	scene.addItem(m_pMono);
	scene.addItem(m_pSample);
	scene.addItem(m_pAna);
	scene.addItem(m_pDet);

	setAcceptedMouseButtons(0);
	m_bReady = 1;
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
	static bool bAllowUpdate = 1;
	if(!bAllowUpdate) return;

	ublas::vector<double> vecSrc = qpoint_to_vec(mapFromItem(m_pSrc, 0, 0));
	ublas::vector<double> vecMono = qpoint_to_vec(mapFromItem(m_pMono, 0, 0));
	ublas::vector<double> vecSample = qpoint_to_vec(mapFromItem(m_pSample, 0, 0));
	ublas::vector<double> vecAna = qpoint_to_vec(mapFromItem(m_pAna, 0, 0));
	ublas::vector<double> vecDet = qpoint_to_vec(mapFromItem(m_pDet, 0, 0));

	bAllowUpdate = 0;
	if(pNode==m_pSample)
	{
		double dTwoTheta = m_dTwoTheta;
		double dAnaTwoTheta = m_dAnaTwoTheta;

		ublas::vector<double> vecSrcMono = vecMono-vecSrc;
		vecSrcMono /= ublas::norm_2(vecSrcMono);

		ublas::vector<double> vecMonoSample = vecSample-vecMono;
		vecSrcMono /= ublas::norm_2(vecMonoSample);

		m_dMonoTwoTheta = -(vec_angle_2(vecMonoSample) - vec_angle_2(vecSrcMono));
		if(m_dMonoTwoTheta < -M_PI) m_dMonoTwoTheta += 2.*M_PI;
		if(m_dMonoTwoTheta > M_PI) m_dMonoTwoTheta -= 2.*M_PI;

		//std::cout << m_dMonoTwoTheta/M_PI*180. << std::endl;


		ublas::vector<double> vecSampleAna = ublas::prod(rotation_matrix_2d(-dTwoTheta), vecMonoSample);
		vecSampleAna /= ublas::norm_2(vecSampleAna);
		vecSampleAna *= m_dLenSampleAna;

		m_pAna->setPos(vec_to_qpoint(vecSample + vecSampleAna));



		vecSampleAna /= ublas::norm_2(vecSampleAna);

		ublas::vector<double> vecAnaDet = ublas::prod(rotation_matrix_2d(-dAnaTwoTheta), vecSampleAna);
		vecAnaDet /= ublas::norm_2(vecAnaDet);
		vecAnaDet *= m_dLenAnaDet;

		m_pDet->setPos(vec_to_qpoint(vecAna+vecAnaDet));


		TriangleOptions opts;
		opts.bChangedMonoTwoTheta = 1;
		opts.bChangedTwoTheta = 1;
		opts.bChangedAnaTwoTheta = 1;
		opts.dMonoTwoTheta = m_dMonoTwoTheta;
		opts.dTwoTheta = dTwoTheta;
		opts.dAnaTwoTheta = dAnaTwoTheta;
		m_scene.emitUpdate(opts);
	}
	else if(pNode==m_pMono)
	{
		ublas::vector<double> vecSrcMono = vecMono-vecSrc;
		vecSrcMono /= ublas::norm_2(vecSrcMono);

		ublas::vector<double> vecMonoSample = ublas::prod(rotation_matrix_2d(-m_dMonoTwoTheta), vecSrcMono);
		vecMonoSample /= ublas::norm_2(vecMonoSample);
		vecMonoSample *= m_dLenMonoSample;

		m_pSample->setPos(vec_to_qpoint(vecMono+vecMonoSample));

		ublas::vector<double> vecSampleAna = vecAna - vecSample;
		vecSampleAna /= ublas::norm_2(vecSampleAna);
		vecSampleAna *= m_dLenSampleAna;

		m_pAna->setPos(vec_to_qpoint(vecSample + vecSampleAna));
	}
	else if(pNode==m_pDet)
	{
		ublas::vector<double> vecSampleAna = vecAna - vecSample;
		vecSampleAna /= ublas::norm_2(vecSampleAna);

		ublas::vector<double> vecAnaDet = vecDet-vecAna;
		vecAnaDet /= ublas::norm_2(vecAnaDet);

		m_dAnaTwoTheta = -(vec_angle_2(vecAnaDet) - vec_angle_2(vecSampleAna));
		if(m_dAnaTwoTheta < -M_PI) m_dAnaTwoTheta += 2.*M_PI;
		if(m_dAnaTwoTheta > M_PI) m_dAnaTwoTheta -= 2.*M_PI;

		//std::cout << m_dAnaTwoTheta/M_PI*180. << std::endl;

		TriangleOptions opts;
		opts.bChangedAnaTwoTheta = 1;
		opts.dAnaTwoTheta = m_dAnaTwoTheta;
		m_scene.emitUpdate(opts);
	}

	if(pNode==m_pMono || pNode==m_pAna)
	{
		ublas::vector<double> vecSampleAna = vecAna-vecSample;
		vecSampleAna /= ublas::norm_2(vecSampleAna);

		ublas::vector<double> vecAnaDet = ublas::prod(rotation_matrix_2d(-m_dAnaTwoTheta), vecSampleAna);
		vecAnaDet /= ublas::norm_2(vecAnaDet);
		vecAnaDet *= m_dLenAnaDet;

		m_pDet->setPos(vec_to_qpoint(vecAna+vecAnaDet));

		ublas::vector<double> vecMonoSample = vecSample - vecMono;
		vecMonoSample /= ublas::norm_2(vecMonoSample);

		m_dTwoTheta = -(vec_angle_2(vecSampleAna) - vec_angle_2(vecMonoSample));
		if(m_dTwoTheta < -M_PI) m_dTwoTheta += 2.*M_PI;
		if(m_dTwoTheta > M_PI) m_dTwoTheta -= 2.*M_PI;

		//std::cout << m_dTwoTheta/M_PI*180. << std::endl;

		TriangleOptions opts;
		opts.bChangedTwoTheta = 1;
		opts.dTwoTheta = m_dTwoTheta;
		m_scene.emitUpdate(opts);
	}

	bAllowUpdate = 1;
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



	// dashed extended lines
	QLineF lineSrcMono_ext(ptMono, ptMono + (ptMono-ptSrc)/2.);
	QLineF lineki_ext(ptSample, ptSample + (ptSample-ptMono)/2.);
	QLineF linekf_ext(ptAna, ptAna + (ptAna-ptSample)/2.);

	QPen penOrig = painter->pen();
	painter->setPen(Qt::DashLine);

	painter->drawLine(lineSrcMono_ext);
	painter->drawLine(lineki_ext);
	painter->drawLine(linekf_ext);

	painter->setPen(penOrig);



	// angle arcs
	const QLineF* pLines1[] = {&lineSrcMono, &lineKi, &lineKf};
	const QLineF* pLines2[] = {&lineKi, &lineKf, &lineAnaDet};
	const QPointF* pPoints[] = {&ptMono, &ptSample, &ptAna};
	const QPointF* pPoints_ext[] = {&ptSrc, &ptMono, &ptSample};
	const double dAngles[] = {m_dMonoTwoTheta, m_dTwoTheta, m_dAnaTwoTheta};

	for(unsigned int i=0; i<3; ++i)
	{
		double dArcSize = (pLines1[i]->length() + pLines2[i]->length()) / 2. / 4.;
		double dBeginArcAngle = pLines1[i]->angle();
		double dArcAngle = dAngles[i]/M_PI*180.;

		painter->setPen(Qt::blue);
		painter->drawArc(QRectF(pPoints[i]->x()-dArcSize/2., pPoints[i]->y()-dArcSize/2.,
								dArcSize, dArcSize),
								dBeginArcAngle*16., dArcAngle*16.);


		std::ostringstream ostrAngle;
		ostrAngle.precision(4);
		ostrAngle << std::fabs(dArcAngle) << "\xb0";

		QPointF ptDirOut = *pPoints[i] - *pPoints_ext[i];
		ptDirOut /= std::sqrt(ptDirOut.x()*ptDirOut.x() + ptDirOut.y()*ptDirOut.y());
		ptDirOut *= pLines1[i]->length()/4.;

		QPointF ptText = *pPoints[i] + ptDirOut;
		painter->drawText(ptText, ostrAngle.str().c_str());
	}

	painter->setPen(penOrig);


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
	ublas::vector<double> vecKf = ublas::prod(rotation_matrix_2d(-dAngle), vecKi);
	vecKf /= ublas::norm_2(vecKf);
	vecKf *= dLenKf;

	m_pAna->setPos(vec_to_qpoint(vecSample + vecKf));
	nodeMoved(m_pAna);
}

void TasLayout::SetMonoTwoTheta(double dAngle)
{
	m_dMonoTwoTheta = dAngle;
	nodeMoved(m_pMono);
}

void TasLayout::SetAnaTwoTheta(double dAngle)
{
	m_dAnaTwoTheta = dAngle;
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
	if(!m_pTas || !m_pTas->IsReady())
		return;

	m_bDontEmitChange = 1;

	m_pTas->SetMonoTwoTheta(opts.dMonoTwoTheta);
	m_pTas->SetSampleTwoTheta(opts.dTwoTheta);
	m_pTas->SetAnaTwoTheta(opts.dAnaTwoTheta);

	update();

	m_bDontEmitChange = 0;
}

void TasLayoutScene::emitUpdate(const TriangleOptions& opts)
{
	if(!m_pTas || !m_pTas->IsReady() || m_bDontEmitChange)
		return;

	emit tasChanged(opts);
}

#include "tas_layout.moc"
