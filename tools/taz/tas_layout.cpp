/*
 * TAS layout
 * @author tweber
 * @date feb-2014
 */

#include "tas_layout.h"
#include "helper/spec_char.h"
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
	this->setFlag(QGraphicsItem::ItemIgnoresTransformations);

	m_pSrc = new TasLayoutNode(this);
	m_pMono = new TasLayoutNode(this);
	m_pSample = new TasLayoutNode(this);
	m_pAna = new TasLayoutNode(this);
	m_pDet = new TasLayoutNode(this);

	m_pSrc->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	m_pMono->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	m_pSample->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	m_pAna->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	m_pDet->setFlag(QGraphicsItem::ItemIgnoresTransformations);

	m_pSrc->setToolTip("Source");
	m_pMono->setToolTip("Monochromator");
	m_pSample->setToolTip("Sample");
	m_pAna->setToolTip("Analyser");
	m_pDet->setToolTip("Detector");

	m_pSrc->setPos(200., m_dLenMonoSample*m_dScaleFactor);
	m_pMono->setPos(0., m_dLenMonoSample*m_dScaleFactor);
	m_pSample->setPos(0., 0.);
	m_pAna->setPos(-m_dLenSampleAna*m_dScaleFactor, 0.);
	m_pDet->setPos(-100., -m_dLenAnaDet*m_dScaleFactor);

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
	m_bReady = 0;

	delete m_pSrc;
	delete m_pMono;
	delete m_pSample;
	delete m_pAna;
	delete m_pDet;
}

void TasLayout::nodeMoved(const TasLayoutNode *pNode)
{
	if(!m_bReady) return;

	static bool bAllowUpdate = 1;
	if(!bAllowUpdate) return;

	const ublas::vector<double> vecSrc = qpoint_to_vec(mapFromItem(m_pSrc, 0, 0));
	const ublas::vector<double> vecMono = qpoint_to_vec(mapFromItem(m_pMono, 0, 0));
	const ublas::vector<double> vecSample = qpoint_to_vec(mapFromItem(m_pSample, 0, 0));
	const ublas::vector<double> vecAna = qpoint_to_vec(mapFromItem(m_pAna, 0, 0));
	const ublas::vector<double> vecDet = qpoint_to_vec(mapFromItem(m_pDet, 0, 0));

	bAllowUpdate = 0;
	if(pNode==m_pSample)
	{
		double dTwoTheta = m_dTwoTheta;
		double dAnaTwoTheta = m_dAnaTwoTheta;

		ublas::vector<double> vecSrcMono = vecMono-vecSrc;
		vecSrcMono /= ublas::norm_2(vecSrcMono);

		ublas::vector<double> vecMonoSample = vecSample-vecMono;
		if(m_bAllowChanges)
			m_dLenMonoSample = ublas::norm_2(vecMonoSample)/m_dScaleFactor;
		vecMonoSample /= ublas::norm_2(vecMonoSample);

		if(m_bAllowChanges)
		{
			m_dMonoTwoTheta = -(vec_angle(vecMonoSample) - vec_angle(vecSrcMono));
			if(m_dMonoTwoTheta < -M_PI) m_dMonoTwoTheta += 2.*M_PI;
			if(m_dMonoTwoTheta > M_PI) m_dMonoTwoTheta -= 2.*M_PI;
		}

		//std::cout << m_dMonoTwoTheta/M_PI*180. << std::endl;


		ublas::vector<double> vecSampleAna = ublas::prod(rotation_matrix_2d(-dTwoTheta), vecMonoSample);
		vecSampleAna /= ublas::norm_2(vecSampleAna);
		vecSampleAna *= m_dLenSampleAna*m_dScaleFactor;

		m_pAna->setPos(vec_to_qpoint(vecSample + vecSampleAna));



		vecSampleAna /= ublas::norm_2(vecSampleAna);

		ublas::vector<double> vecAnaDet = ublas::prod(rotation_matrix_2d(-dAnaTwoTheta), vecSampleAna);
		vecAnaDet /= ublas::norm_2(vecAnaDet);
		vecAnaDet *= m_dLenAnaDet*m_dScaleFactor;

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
		vecMonoSample *= m_dLenMonoSample*m_dScaleFactor;

		m_pSample->setPos(vec_to_qpoint(vecMono + vecMonoSample));

		ublas::vector<double> vecSampleAna = vecAna - vecSample;
		vecSampleAna /= ublas::norm_2(vecSampleAna);
		vecSampleAna *= m_dLenSampleAna*m_dScaleFactor;

		m_pAna->setPos(vec_to_qpoint(vecSample + vecSampleAna));
	}
	else if(pNode==m_pDet)
	{
		ublas::vector<double> vecSampleAna = vecAna - vecSample;
		vecSampleAna /= ublas::norm_2(vecSampleAna);

		ublas::vector<double> vecAnaDet = vecDet-vecAna;
		if(m_bAllowChanges)
			m_dLenAnaDet = ublas::norm_2(vecAnaDet)/m_dScaleFactor;
		vecAnaDet /= ublas::norm_2(vecAnaDet);

		if(m_bAllowChanges)
		{
			m_dAnaTwoTheta = -(vec_angle(vecAnaDet) - vec_angle(vecSampleAna));
			if(m_dAnaTwoTheta < -M_PI) m_dAnaTwoTheta += 2.*M_PI;
			if(m_dAnaTwoTheta > M_PI) m_dAnaTwoTheta -= 2.*M_PI;
		}

		//std::cout << m_dAnaTwoTheta/M_PI*180. << std::endl;

		TriangleOptions opts;
		opts.bChangedAnaTwoTheta = 1;
		opts.dAnaTwoTheta = m_dAnaTwoTheta;
		m_scene.emitUpdate(opts);
	}

	if(pNode==m_pMono || pNode==m_pAna)
	{
		ublas::vector<double> vecSampleAna = vecAna-vecSample;
		if(pNode==m_pAna && m_bAllowChanges)
			m_dLenSampleAna = ublas::norm_2(vecSampleAna)/m_dScaleFactor;
		vecSampleAna /= ublas::norm_2(vecSampleAna);

		ublas::vector<double> vecAnaDet = ublas::prod(rotation_matrix_2d(-m_dAnaTwoTheta), vecSampleAna);
		vecAnaDet /= ublas::norm_2(vecAnaDet);
		vecAnaDet *= m_dLenAnaDet*m_dScaleFactor;

		m_pDet->setPos(vec_to_qpoint(vecAna+vecAnaDet));

		ublas::vector<double> vecMonoSample = vecSample - vecMono;
		vecMonoSample /= ublas::norm_2(vecMonoSample);

		if(m_bAllowChanges)
		{
			m_dTwoTheta = -(vec_angle(vecSampleAna) - vec_angle(vecMonoSample));
			if(m_dTwoTheta < -M_PI) m_dTwoTheta += 2.*M_PI;
			if(m_dTwoTheta > M_PI) m_dTwoTheta -= 2.*M_PI;
		}

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
	return QRectF(-500.*m_dZoom, -500.*m_dZoom,
				1000.*m_dZoom, 1000.*m_dZoom);
}

void TasLayout::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	QPointF ptSrc = mapFromItem(m_pSrc, 0, 0) * m_dZoom;
	QPointF ptMono = mapFromItem(m_pMono, 0, 0) * m_dZoom;
	QPointF ptSample = mapFromItem(m_pSample, 0, 0) * m_dZoom;
	QPointF ptAna = mapFromItem(m_pAna, 0, 0) * m_dZoom;
	QPointF ptDet = mapFromItem(m_pDet, 0, 0) * m_dZoom;

	QLineF lineSrcMono(ptSrc, ptMono);
	QLineF lineKi(ptMono, ptSample);
	QLineF lineKf(ptSample, ptAna);
	QLineF lineAnaDet(ptAna, ptDet);

	QPen penOrig = painter->pen();

	painter->drawLine(lineSrcMono);
	painter->drawLine(lineKi);
	painter->drawLine(lineKf);
	painter->drawLine(lineAnaDet);


	// write lengths
	QPointF ptMidKi = ptMono + (ptSample-ptMono)/2.;
	QPointF ptMidKf = ptSample + (ptAna-ptSample)/2.;
	QPointF ptMidAnaDet = ptAna + (ptDet-ptAna)/2.;

	std::ostringstream ostrLenKi, ostrLenKf, ostrLenAnaDet;
	ostrLenKi.precision(4); ostrLenKf.precision(4); ostrLenAnaDet.precision(4);

	ostrLenKi << m_dLenMonoSample << " cm";
	ostrLenKf << m_dLenSampleAna << " cm";
	ostrLenAnaDet << m_dLenAnaDet << " cm";

	painter->drawText(ptMidKi, ostrLenKi.str().c_str());
	painter->drawText(ptMidKf, ostrLenKf.str().c_str());
	painter->drawText(ptMidAnaDet, ostrLenAnaDet.str().c_str());



	ublas::vector<double> vecMono = qpoint_to_vec(ptMono);
	ublas::vector<double> vecSample = qpoint_to_vec(ptSample);
	ublas::vector<double> vecMonoSample = vecSample-vecMono;


	// sample theta rotation
	ublas::vector<double> vecSampleRotDir = ublas::prod(rotation_matrix_2d(-m_dTheta), vecMonoSample);
	vecSampleRotDir /= ublas::norm_2(vecSampleRotDir);
	vecSampleRotDir *= m_dLenSample*m_dScaleFactor;

	QPointF ptSampleThM =  vec_to_qpoint(vecSample-vecSampleRotDir);
	QPointF ptSampleThP =  vec_to_qpoint(vecSample+vecSampleRotDir);
	QLineF lineSampleRot(ptSampleThM, ptSampleThP);

	QPen penSample(Qt::red);
	penSample.setWidthF(1.5);
	painter->setPen(penSample);
	painter->drawLine(lineSampleRot);


	// dashed extended lines
	painter->setPen(penOrig);
	QLineF lineSrcMono_ext(ptMono, ptMono + (ptMono-ptSrc)/2.);
	QLineF lineki_ext(ptSample, ptSample + (ptSample-ptMono)/2.);
	QLineF linekf_ext(ptAna, ptAna + (ptAna-ptSample)/2.);

	painter->setPen(Qt::DashLine);

	painter->drawLine(lineSrcMono_ext);
	painter->drawLine(lineki_ext);
	painter->drawLine(linekf_ext);

	painter->setPen(penOrig);



	// angle arcs
	const QLineF* pLines1[] = {&lineSrcMono, &lineKi, &lineKf, &lineSampleRot};
	const QLineF* pLines2[] = {&lineKi, &lineKf, &lineAnaDet, &lineKi};
	const QPointF* pPoints[] = {&ptMono, &ptSample, &ptAna, &ptSample};
	const QPointF* pPoints_ext[] = {&ptSrc, &ptMono, &ptSample, &ptSampleThP};
	const double dAngles[] = {m_dMonoTwoTheta, m_dTwoTheta, m_dAnaTwoTheta, -m_dTheta};
	const double dAngleOffs[] = {0., 0., 0., 180.};

	QPen pen1(Qt::blue);
	QPen pen2(Qt::red);
	QPen* arcPens[] = {&pen1, &pen1, &pen1, &pen2};

	for(unsigned int i=0; i<4; ++i)
	{
		double dArcSize = (pLines1[i]->length() + pLines2[i]->length()) / 2. / 3.;
		double dBeginArcAngle = pLines1[i]->angle() + dAngleOffs[i];
		double dArcAngle = dAngles[i]/M_PI*180.;

		painter->setPen(*arcPens[i]);
		painter->drawArc(QRectF(pPoints[i]->x()-dArcSize/2., pPoints[i]->y()-dArcSize/2.,
								dArcSize, dArcSize),
								dBeginArcAngle*16., dArcAngle*16.);


		const std::wstring& strDEG = ::get_spec_char_utf16("DEG");
		std::wostringstream ostrAngle;
		ostrAngle.precision(4);
		if(!std::isnan(dArcAngle))
			ostrAngle << std::fabs(dArcAngle) << strDEG;
		else
			ostrAngle << "invalid";

		QPointF ptDirOut = *pPoints[i] - *pPoints_ext[i];
		ptDirOut /= std::sqrt(ptDirOut.x()*ptDirOut.x() + ptDirOut.y()*ptDirOut.y());
		ptDirOut *= pLines1[i]->length()/4.;

		QPointF ptText = *pPoints[i] + ptDirOut;
		painter->drawText(ptText, QString::fromWCharArray(ostrAngle.str().c_str()));
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
	nodeMoved(m_pSample);
	nodeMoved(m_pAna);
}

void TasLayout::SetSampleTheta(double dAngle)
{
	m_dTheta = dAngle;
	this->update();
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

void TasLayout::SetZoom(double dZoom)
{
	m_dZoom = dZoom;
	m_scene.update();
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
	m_pTas->AllowChanges(0);

	m_pTas->SetMonoTwoTheta(opts.dMonoTwoTheta);
	m_pTas->SetSampleTwoTheta(opts.dTwoTheta);
	m_pTas->SetSampleTheta(opts.dTheta);
	m_pTas->SetAnaTwoTheta(opts.dAnaTwoTheta);

	update();

	m_pTas->AllowChanges(1);
	m_bDontEmitChange = 0;
}

void TasLayoutScene::emitUpdate(const TriangleOptions& opts)
{
	if(!m_pTas || !m_pTas->IsReady() || m_bDontEmitChange)
		return;

	emit tasChanged(opts);
}

void TasLayoutScene::scaleChanged(double dTotalScale)
{
	if(!m_pTas)
		return;

	m_pTas->SetZoom(dTotalScale);
}



// --------------------------------------------------------------------------------


TasLayoutView::TasLayoutView(QWidget* pParent)
						: QGraphicsView(pParent)
{
	setRenderHints(QPainter::Antialiasing);
	setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	setDragMode(QGraphicsView::ScrollHandDrag);
}

TasLayoutView::~TasLayoutView()
{

}

void TasLayoutView::wheelEvent(QWheelEvent *pEvt)
{
	//this->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
	this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

	double dDelta = pEvt->delta()/100.;
	double dScale = dDelta;
	if(dDelta < 0.)
		dScale = -1./dDelta;

	this->scale(dScale, dScale);


	m_dTotalScale *= dScale;
	emit scaleChanged(m_dTotalScale);
}


#include "tas_layout.moc"
