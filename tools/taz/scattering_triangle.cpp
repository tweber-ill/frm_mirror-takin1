/*
 * Scattering Triangle
 * @author tweber
 * @date feb-2014
 */

#include "helper/flags.h"
#include "helper/neutrons.hpp"
#include "helper/spec_char.h"
#include "helper/log.h"
#include "scattering_triangle.h"

#include <QtGui/QToolTip>
#include <iostream>
#include <cmath>
#include <sstream>


ScatteringTriangleNode::ScatteringTriangleNode(ScatteringTriangle* pSupItem)
	: m_pParentItem(pSupItem)
{
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setCacheMode(QGraphicsItem::DeviceCoordinateCache);
	setCursor(Qt::CrossCursor);

	setData(TRIANGLE_NODE_TYPE_KEY, NODE_OTHER);
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
	QVariant var = QGraphicsItem::itemChange(change, value);

	if(change == QGraphicsItem::ItemPositionHasChanged)
		m_pParentItem->nodeMoved(this);

	return var;
}


// --------------------------------------------------------------------------------

RecipPeak::RecipPeak()
{
	setCacheMode(QGraphicsItem::DeviceCoordinateCache);
	//setCursor(Qt::ArrowCursor);
	setFlag(QGraphicsItem::ItemIsMovable, false);
}

QRectF RecipPeak::boundingRect() const
{
	return QRectF(-3., -3., 64., 25.);
}

void RecipPeak::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	painter->setBrush(m_color);
	painter->drawEllipse(QRectF(-3., -3., 6., 6.));

	if(m_strLabel != "")
	{
		painter->setPen(m_color);
		painter->drawText(0., 14., m_strLabel);
	}
}

// --------------------------------------------------------------------------------


ScatteringTriangle::ScatteringTriangle(ScatteringTriangleScene& scene)
				: m_scene(scene)
{
	this->setFlag(QGraphicsItem::ItemIgnoresTransformations);

	m_pNodeKiQ = new ScatteringTriangleNode(this);
	m_pNodeKiKf = new ScatteringTriangleNode(this);
	m_pNodeKfQ = new ScatteringTriangleNode(this);
	m_pNodeGq = new ScatteringTriangleNode(this);

	m_pNodeKfQ->setData(TRIANGLE_NODE_TYPE_KEY, NODE_Q);
	m_pNodeGq->setData(TRIANGLE_NODE_TYPE_KEY, NODE_q);

	m_pNodeKiKf->setFlag(QGraphicsItem::ItemIsMovable);
	m_pNodeKfQ->setFlag(QGraphicsItem::ItemIsMovable);
	m_pNodeGq->setFlag(QGraphicsItem::ItemIsMovable);

	m_pNodeKiQ->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	m_pNodeKiKf->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	m_pNodeKfQ->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	m_pNodeGq->setFlag(QGraphicsItem::ItemIgnoresTransformations);

	m_pNodeKiQ->setPos(0., 0.);
	m_pNodeKiKf->setPos(50., -100.);
	m_pNodeKfQ->setPos(100., 0.);
	m_pNodeGq->setPos(100., 0.);

	m_scene.addItem(m_pNodeKiQ);
	m_scene.addItem(m_pNodeKiKf);
	m_scene.addItem(m_pNodeKfQ);
	m_scene.addItem(m_pNodeGq);

	setAcceptedMouseButtons(0);

	m_bReady = 1;
}

ScatteringTriangle::~ScatteringTriangle()
{
	m_bReady = 0;

	delete m_pNodeKiQ;
	delete m_pNodeKiKf;
	delete m_pNodeKfQ;
	delete m_pNodeGq;

	ClearPeaks();
}

void ScatteringTriangle::nodeMoved(const ScatteringTriangleNode* pNode)
{
	if(!m_bReady) return;

	if(m_scene.getSnapq() && pNode==GetNodeKfQ())
		m_scene.SnapToNearestPeak(GetNodeGq(), GetNodeKfQ());

	m_scene.emitUpdate();
	m_scene.emitAllParams();
	this->update();
}

QRectF ScatteringTriangle::boundingRect() const
{
	return QRectF(-1000.*m_dZoom, -1000.*m_dZoom,
					2000.*m_dZoom, 2000.*m_dZoom);
}

void ScatteringTriangle::SetZoom(double dZoom)
{
	m_dZoom = dZoom; m_scene.update();
}

void ScatteringTriangle::SetqVisible(bool bVisible)
{
	m_bqVisible = bVisible;
	this->m_pNodeGq->setVisible(bVisible);
	this->update();
}

void ScatteringTriangle::SetBZVisible(bool bVisible)
{
	m_bShowBZ = bVisible;
	this->update();
}

void ScatteringTriangle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	// Brillouin zone
	if(m_bShowBZ && m_bz.IsValid())
	{
		QPen penOrg = painter->pen();
		painter->setPen(Qt::lightGray);
		
		const ublas::vector<double>& vecCentral = m_bz.GetCentralReflex();

		for(const RecipPeak* pPeak : m_vecPeaks)
		{
			QPointF peakPos = pPeak->pos();
			peakPos *= m_dZoom;

			const Brillouin2D<double>::t_vertices<double>& verts = m_bz.GetVertices();
			for(const Brillouin2D<double>::t_vecpair<double>& vertpair : verts)
			{
				const ublas::vector<double>& vec1 = vertpair.first * m_dScaleFactor * m_dZoom;
				const ublas::vector<double>& vec2 = vertpair.second * m_dScaleFactor * m_dZoom;

				QPointF pt1 = vec_to_qpoint(vec1 - vecCentral) + peakPos;
				QPointF pt2 = vec_to_qpoint(vec2 - vecCentral) + peakPos;

				QLineF lineBZ(pt1, pt2);
				painter->drawLine(lineBZ);
			}
		}

		painter->setPen(penOrg);
	}




	QPointF ptKiQ = mapFromItem(m_pNodeKiQ, 0, 0) * m_dZoom;
	QPointF ptKfQ = mapFromItem(m_pNodeKfQ, 0, 0) * m_dZoom;
	QPointF ptKiKf = mapFromItem(m_pNodeKiKf, 0, 0) * m_dZoom;
	QPointF ptGq = mapFromItem(m_pNodeGq, 0, 0) * m_dZoom;

	QPointF ptQMid = ptKiQ + (ptKfQ - ptKiQ)/2.;
	QPointF ptKiMid = ptKiQ + (ptKiKf - ptKiQ)/2.;
	QPointF ptKfMid = ptKfQ + (ptKiKf - ptKfQ)/2.;
	QPointF ptqMid = ptKfQ + (ptGq - ptKfQ)/2.;
	QPointF ptGMid = ptKiQ + (ptGq - ptKiQ)/2.;

	QPointF ptMid = ptKiQ + (ptKfQ - ptKiQ)/2.;
	ptMid = ptMid + (ptKiKf - ptMid)/2.;

	QLineF lineQ(ptKiQ, ptKfQ);
	QLineF lineKi(ptKiQ, ptKiKf);
	QLineF lineKf(ptKiKf, ptKfQ);
	QLineF lineG(ptKiQ, ptGq);
	QLineF lineq(ptKfQ, ptGq);

	painter->setPen(Qt::red);
	painter->drawLine(lineQ);
	painter->setPen(Qt::black);
	painter->drawLine(lineKi);
	painter->drawLine(lineKf);

	if(m_bqVisible)
	{
		QPen penOrg = painter->pen();
		painter->setPen(Qt::darkGreen);

		painter->drawLine(lineG);
		painter->drawLine(lineq);

		painter->setPen(penOrg);
	}

	const double dG = lineG.length()/m_dScaleFactor/m_dZoom;

	const std::wstring& strAA = ::get_spec_char_utf16("AA") + ::get_spec_char_utf16("sup-") + ::get_spec_char_utf16("sup1");
	const std::wstring& strDelta = ::get_spec_char_utf16("Delta");

	std::wostringstream ostrQ, ostrKi, ostrKf, ostrE, ostrq, ostrG;
	ostrQ.precision(3); ostrE.precision(3);
	ostrKi.precision(3); ostrKf.precision(3);
	ostrG.precision(3); ostrq.precision(3);

	ostrQ << L"Q = " << GetQ() << " " << strAA;
	ostrKi << L"ki = " << GetKi() << " " << strAA;
	ostrKf << L"kf = " << GetKf() << " " << strAA;
	ostrE << strDelta << "E = " << GetE() << " meV";
	if(m_bqVisible)
	{
		ostrq << L"q = " << Getq() << " " << strAA;
		ostrG << L"G = " << dG << " " << strAA;
	}

	painter->save();

	painter->rotate(-lineQ.angle());
	painter->setPen(Qt::red);
	painter->drawText(QPointF(lineQ.length()/5.,12.), QString::fromWCharArray(ostrQ.str().c_str()));
	painter->rotate(lineQ.angle());

	painter->rotate(-lineKi.angle());
	painter->setPen(Qt::black);
	painter->drawText(QPointF(lineKi.length()/5.,-4.), QString::fromWCharArray(ostrKi.str().c_str()));
	painter->rotate(lineKi.angle());

	painter->translate(ptKiKf);
	painter->rotate(-lineKf.angle());
	painter->drawText(QPointF(lineKf.length()/5.,-4.), QString::fromWCharArray(ostrKf.str().c_str()));
	painter->drawText(QPointF(lineKf.length()/5.,12.), QString::fromWCharArray(ostrE.str().c_str()));
	painter->rotate(lineKf.angle());
	painter->translate(-ptKiKf);

	if(m_bqVisible)
	{
		QPen penOrg = painter->pen();
		painter->setPen(Qt::darkGreen);

		painter->translate(ptKfQ);
		painter->rotate(-lineq.angle());
		painter->drawText(QPointF(lineq.length()/5.,-4.), QString::fromWCharArray(ostrq.str().c_str()));
		painter->rotate(lineq.angle());
		painter->translate(-ptKfQ);

		painter->rotate(-lineG.angle());
		painter->drawText(QPointF(lineG.length()/5.,-4.), QString::fromWCharArray(ostrG.str().c_str()));
		painter->rotate(lineG.angle());

		painter->setPen(penOrg);
	}

	painter->restore();


	QLineF lineKi2(ptKiKf, ptKiQ);
	QLineF lineKf2(ptKfQ, ptKiKf);
	QLineF lineQ2(ptKfQ, ptKiQ);

	std::vector<const QLineF*> vecLines1 = {&lineKi2, &lineKi, &lineKf};
	std::vector<const QLineF*> vecLines2 = {&lineQ, &lineKf, &lineQ2};
	std::vector<const QPointF*> vecPoints = {&ptKiQ, &ptKiKf, &ptKfQ};

	std::vector<const QLineF*> vecLinesArrow = {&lineKi, &lineKf2, &lineQ};
	std::vector<const QPointF*> vecPointsArrow = {&ptKiQ, &ptKfQ, &ptKiQ};

	std::vector<bool> vecDrawAngles = {1,1,1};
	std::vector<QColor> vecColor {Qt::black, Qt::black, Qt::red};

	QLineF lineG2(ptGq, ptKiQ);
	QLineF lineq2(ptGq, ptKfQ);

	if(m_bqVisible)
	{
		vecLinesArrow.push_back(&lineq2);
		vecLinesArrow.push_back(&lineG);

		vecPointsArrow.push_back(&ptGq);
		vecPointsArrow.push_back(&ptKiQ);

		vecDrawAngles.push_back(0);
		vecDrawAngles.push_back(0);

		vecColor.push_back(Qt::darkGreen);
		vecColor.push_back(Qt::darkGreen);
	}

	for(unsigned int i=0; i<vecDrawAngles.size(); ++i)
	{
		// arrow heads
		double dAng = (vecLinesArrow[i]->angle() + 90.) / 180. * M_PI;
		double dC = std::cos(dAng);
		double dS = std::sin(dAng);

		double dTriagX = 5., dTriagY = 10.;
		QPointF ptTriag1 = *vecPointsArrow[i] + QPointF(dTriagX*dC + dTriagY*dS, -dTriagX*dS + dTriagY*dC);
		QPointF ptTriag2 = *vecPointsArrow[i] + QPointF(-dTriagX*dC + dTriagY*dS, dTriagX*dS + dTriagY*dC);

		QPainterPath triag;
		triag.moveTo(*vecPointsArrow[i]);
		triag.lineTo(ptTriag1);
		triag.lineTo(ptTriag2);

		painter->setPen(vecColor[i]);
		painter->fillPath(triag, vecColor[i]);

		if(vecDrawAngles[i])
		{
			// angle arcs
			double dArcSize = (vecLines1[i]->length() + vecLines2[i]->length()) / 2. / 3.;
			double dBeginArcAngle = vecLines1[i]->angle() + 180.;
			double dArcAngle = vecLines1[i]->angleTo(*vecLines2[i]) - 180.;

			painter->setPen(Qt::blue);
			painter->drawArc(QRectF(vecPoints[i]->x()-dArcSize/2., vecPoints[i]->y()-dArcSize/2., dArcSize, dArcSize),
							dBeginArcAngle*16., dArcAngle*16.);

			const std::wstring& strDEG = ::get_spec_char_utf16("deg");
			std::wostringstream ostrAngle;
			ostrAngle.precision(4);
			ostrAngle << std::fabs(dArcAngle) << strDEG;

			QPointF ptDirOut = *vecPoints[i] - ptMid;
			ptDirOut /= std::sqrt(ptDirOut.x()*ptDirOut.x() + ptDirOut.y()*ptDirOut.y());
			ptDirOut *= 15.;

			QPointF ptText = *vecPoints[i] + ptDirOut;
			painter->drawText(ptText, QString::fromWCharArray(ostrAngle.str().c_str()));
		}
	}
}

double ScatteringTriangle::GetKi() const
{
	QPointF ptKiQ = mapFromItem(m_pNodeKiQ, 0, 0);
	QPointF ptKiKf = mapFromItem(m_pNodeKiKf, 0, 0);

	QLineF lineKi(ptKiQ, ptKiKf);
	const double dKi = lineKi.length()/m_dScaleFactor;
	return dKi;
}

double ScatteringTriangle::GetKf() const
{
	QPointF ptKfQ = mapFromItem(m_pNodeKfQ, 0, 0);
	QPointF ptKiKf = mapFromItem(m_pNodeKiKf, 0, 0);

	QLineF lineKf(ptKiKf, ptKfQ);
	const double dKf = lineKf.length()/m_dScaleFactor;
	return dKf;
}

double ScatteringTriangle::GetE() const
{
	const double dKi = GetKi();
	double dKf = GetKf();
	const double dE = get_energy_transfer(dKi/angstrom, dKf/angstrom) / one_eV * 1000.;
	return dE;
}

double ScatteringTriangle::GetQ() const
{
  	QPointF ptKiQ = mapFromItem(m_pNodeKiQ, 0, 0) * m_dZoom;
	QPointF ptKfQ = mapFromItem(m_pNodeKfQ, 0, 0) * m_dZoom;

	QLineF lineQ(ptKiQ, ptKfQ);
	const double dQ = lineQ.length()/m_dScaleFactor/m_dZoom;
	return dQ;
}

double ScatteringTriangle::Getq() const
{
	QPointF ptKfQ = mapFromItem(m_pNodeKfQ, 0, 0) * m_dZoom;
	QPointF ptGq = mapFromItem(m_pNodeGq, 0, 0) * m_dZoom;

	QLineF lineq(ptKfQ, ptGq);
	const double dq = lineq.length()/m_dScaleFactor/m_dZoom;
	return dq;
}

double ScatteringTriangle::GetAngleQVec0() const
{
	ublas::vector<double> vecQ = qpoint_to_vec(mapFromItem(m_pNodeKiQ,0,0))
								- qpoint_to_vec(mapFromItem(m_pNodeKfQ,0,0));
	vecQ /= ublas::norm_2(vecQ);
	return vec_angle(vecQ);
}

double ScatteringTriangle::GetAngleKiQ(bool bPosSense) const
{
	/*ublas::vector<double> vecKi = qpoint_to_vec(mapFromItem(m_pNodeKiQ,0,0))
								- qpoint_to_vec(mapFromItem(m_pNodeKiKf,0,0));
	vecKi /= ublas::norm_2(vecKi);

	const double dAngle = vec_angle(vecKi) - GetAngleQVec0();*/

	try
	{
		double dTT = GetTwoTheta(bPosSense);
		double dAngle = get_angle_ki_Q(GetKi()/angstrom, GetKf()/angstrom, GetQ()/angstrom, bPosSense) / units::si::radians;
		//std::cout << "tt=" << dTT << ", kiQ="<<dAngle << std::endl;

		if(std::fabs(dTT) > M_PI)
			dAngle = -dAngle;

		return dAngle;
	}
	catch(const std::exception& ex)
	{
		log_err(ex.what());
		return 0.;
	}
}

double ScatteringTriangle::GetAngleKfQ(bool bPosSense) const
{
	/*ublas::vector<double> vecKf = qpoint_to_vec(mapFromItem(m_pNodeKfQ,0,0))
								- qpoint_to_vec(mapFromItem(m_pNodeKiKf,0,0));
	vecKf /= ublas::norm_2(vecKf);

	const double dAngle = vec_angle(vecKf) - GetAngleQVec0();*/

	try
	{
		double dTT = GetTwoTheta(bPosSense);
		double dAngle = M_PI-get_angle_kf_Q(GetKi()/angstrom, GetKf()/angstrom, GetQ()/angstrom, bPosSense) / units::si::radians;

		if(std::fabs(dTT) > M_PI)
			dAngle = -dAngle;

		return dAngle;
	}
	catch(const std::exception& ex)
	{
		log_err(ex.what());
		return 0.;
	}
}

double ScatteringTriangle::GetTheta(bool bPosSense) const
{
	ublas::vector<double> vecKi = qpoint_to_vec(mapFromItem(m_pNodeKiQ,0,0))
						- qpoint_to_vec(mapFromItem(m_pNodeKiKf,0,0));
	vecKi /= ublas::norm_2(vecKi);

	double dTh = vec_angle(vecKi) - M_PI/2.;
	dTh += m_dAngleRot;
	if(!bPosSense)
		dTh = -dTh;

	//log_info("theta: ", dTh/M_PI*180.);
	return dTh;
}

double ScatteringTriangle::GetTwoTheta(bool bPosSense) const
{
	ublas::vector<double> vecKi = qpoint_to_vec(mapFromItem(m_pNodeKiQ,0,0))
								- qpoint_to_vec(mapFromItem(m_pNodeKiKf,0,0));
	ublas::vector<double> vecKf = qpoint_to_vec(mapFromItem(m_pNodeKfQ,0,0))
								- qpoint_to_vec(mapFromItem(m_pNodeKiKf,0,0));

	vecKi /= ublas::norm_2(vecKi);
	vecKf /= ublas::norm_2(vecKf);

	double dTT = vec_angle(vecKi) - vec_angle(vecKf);
	if(dTT < 0.)
		dTT += 2.*M_PI;
	dTT = std::fmod(dTT, 2.*M_PI);

	if(!bPosSense)
		dTT = -dTT;

	return dTT;
}

double ScatteringTriangle::GetMonoTwoTheta(double dMonoD, bool bPosSense) const
{
	ublas::vector<double> vecKi = qpoint_to_vec(mapFromItem(m_pNodeKiQ, 0, 0))
									- qpoint_to_vec(mapFromItem(m_pNodeKiKf, 0, 0));
	double dKi = ublas::norm_2(vecKi) / m_dScaleFactor;
	return get_mono_twotheta(dKi/angstrom, dMonoD*angstrom, bPosSense) / units::si::radians;
}

double ScatteringTriangle::GetAnaTwoTheta(double dAnaD, bool bPosSense) const
{
	ublas::vector<double> vecKf = qpoint_to_vec(mapFromItem(m_pNodeKfQ, 0, 0))
									- qpoint_to_vec(mapFromItem(m_pNodeKiKf, 0, 0));
	double dKf = ublas::norm_2(vecKf) / m_dScaleFactor;
	return get_mono_twotheta(dKf/angstrom, dAnaD*angstrom, bPosSense) / units::si::radians;
}

void ScatteringTriangle::SetAnaTwoTheta(double dTT, double dAnaD)
{
	dTT = std::fabs(dTT);
	double dKf  = M_PI / std::sin(dTT/2.) / dAnaD;
	dKf *= m_dScaleFactor;

	ublas::vector<double> vecNodeKiKf = qpoint_to_vec(mapFromItem(m_pNodeKiKf,0,0));
	ublas::vector<double> vecNodeKfQ = qpoint_to_vec(mapFromItem(m_pNodeKfQ,0,0));
	ublas::vector<double> vecKf = qpoint_to_vec(mapFromItem(m_pNodeKfQ,0,0))
								- vecNodeKiKf;

	vecKf /= ublas::norm_2(vecKf);
	ublas::vector<double> vecKf_new = vecKf * dKf;

	m_pNodeKfQ->setPos(vec_to_qpoint(vecNodeKiKf + vecKf_new));

	nodeMoved(m_pNodeKfQ);
}

void ScatteringTriangle::SetMonoTwoTheta(double dTT, double dMonoD)
{
	dTT = std::fabs(dTT);
	double dKi  = M_PI / std::sin(dTT/2.) / dMonoD;
	dKi *= m_dScaleFactor;

	ublas::vector<double> vecNodeKiKf = qpoint_to_vec(mapFromItem(m_pNodeKiKf,0,0));
	ublas::vector<double> vecNodeKiQ = qpoint_to_vec(mapFromItem(m_pNodeKiQ,0,0));
	ublas::vector<double> vecKi = qpoint_to_vec(mapFromItem(m_pNodeKiQ, 0, 0))
									- qpoint_to_vec(mapFromItem(m_pNodeKiKf, 0, 0));

	vecKi /= ublas::norm_2(vecKi);
	ublas::vector<double> vecKi_new = vecKi * dKi;

	m_pNodeKiKf->setPos(vec_to_qpoint(vecNodeKiQ - vecKi_new));
	nodeMoved(m_pNodeKiKf);
}

void ScatteringTriangle::SetTwoTheta(double dTT)
{
	ublas::vector<double> vecNodeKiKf = qpoint_to_vec(mapFromItem(m_pNodeKiKf,0,0));

	ublas::vector<double> vecKi = qpoint_to_vec(mapFromItem(m_pNodeKiQ,0,0))
								- qpoint_to_vec(mapFromItem(m_pNodeKiKf,0,0));
	ublas::vector<double> vecKf = qpoint_to_vec(mapFromItem(m_pNodeKfQ,0,0))
								- vecNodeKiKf;

	ublas::vector<double> vecKf_new = ublas::prod(rotation_matrix_2d(-dTT), vecKi);

	vecKf_new /= ublas::norm_2(vecKf_new);
	vecKf_new *= ublas::norm_2(vecKf);

	m_pNodeKfQ->setPos(vec_to_qpoint(vecNodeKiKf + vecKf_new));
	nodeMoved(m_pNodeKfQ);
}

void ScatteringTriangle::RotateKiVec0To(bool bSense, double dAngle)
{
	double dAngleCorr = bSense ? -M_PI/2. : M_PI/2.;
	double dCurAngle = GetTheta(bSense) + dAngleCorr;
	if(bSense) dCurAngle = -dCurAngle;
	//std::cout << "old: " << dCurAngle/M_PI*180. << "new: " << dAngle/M_PI*180. << std::endl;

	ublas::vector<double> vecNodeKiKf = qpoint_to_vec(mapFromItem(m_pNodeKiKf,0,0));
	ublas::vector<double> vecNodeKfQ = qpoint_to_vec(mapFromItem(m_pNodeKfQ,0,0));

	ublas::matrix<double> matRot = ::rotation_matrix_2d(dCurAngle - dAngle);
	vecNodeKiKf = ublas::prod(matRot, vecNodeKiKf);
	vecNodeKfQ = ublas::prod(matRot, vecNodeKfQ);

	m_pNodeKiKf->setPos(vec_to_qpoint(vecNodeKiKf));
	m_pNodeKfQ->setPos(vec_to_qpoint(vecNodeKfQ));

	nodeMoved(m_pNodeKiKf);
	nodeMoved(m_pNodeKfQ);
}

void ScatteringTriangle::CalcPeaks(const Lattice<double>& lattice,
								const Lattice<double>& recip, const Lattice<double>& recip_unrot,
								const Plane<double>& plane,
								const SpaceGroup* pSpaceGroup)
{
	ClearPeaks();

	m_lattice = lattice;
	m_recip = recip;
	m_recip_unrot = recip_unrot;

	ublas::vector<double> dir0 = plane.GetDir0();
	//ublas::vector<double> dir1 = plane.GetDir1();
	ublas::vector<double> dir1 = cross_3(plane.GetNorm(), dir0);

	double dDir0Len = ublas::norm_2(dir0);
	double dDir1Len = ublas::norm_2(dir1);

	//std::cout << "dir 0 len: " << dDir0Len << std::endl;
	//std::cout << "dir 1 len: " << dDir1Len << std::endl;

	if(float_equal(dDir0Len, 0.) || float_equal(dDir1Len, 0.)
		|| ::isnan(dDir0Len) || ::isnan(dDir1Len))
	{
		log_err("Invalid scattering plane.");
		return;
	}

	dir0 /= dDir0Len;
	dir1 /= dDir1Len;

	m_matPlane = column_matrix({dir0, dir1, plane.GetNorm()});
	bool bInv = ::inverse(m_matPlane, m_matPlane_inv);
	if(!bInv)
	{
		log_err("Cannot invert scattering plane metric.");
		return;
	}


	// crystal rotation angle
	try
	{
		ublas::vector<double> vecUnrotX = plane.GetDroppedPerp(m_recip_unrot.GetVec(0));
		ublas::vector<double> vecRotX = plane.GetDroppedPerp(m_recip.GetVec(0));

		const ublas::vector<double> &vecNorm = plane.GetNorm();
		m_dAngleRot = -vec_angle(vecRotX, vecUnrotX, &vecNorm);

		//std::cout << "rotation angle: " << m_dAngleRot/M_PI*180. << std::endl;
	}
	catch(const std::exception& ex)
	{
		log_err(ex.what());
		return;
	}


	for(int ih=-m_iMaxPeaks; ih<=m_iMaxPeaks; ++ih)
		for(int ik=-m_iMaxPeaks; ik<=m_iMaxPeaks; ++ik)
			for(int il=-m_iMaxPeaks; il<=m_iMaxPeaks; ++il)
			{
				double h = double(ih);
				double k = double(ik);
				double l = double(il);

				if(pSpaceGroup)
				{
					if(!pSpaceGroup->HasReflection(ih, ik, il))
						continue;
				}

				ublas::vector<double> vecPeak = m_recip.GetPos(h,k,l);
				double dDist = 0.;
				ublas::vector<double> vecDropped = plane.GetDroppedPerp(vecPeak, &dDist);

				if(::float_equal(dDist, 0., m_dPlaneDistTolerance))
				{
					ublas::vector<double> vecCoord = ublas::prod(m_matPlane_inv, vecDropped);
					double dX = vecCoord[0];
					double dY = -vecCoord[1];

					/*if(ih==1 && ik==0 && il==0)
					{
						std::cout << h << k << l << ": ";
						std::cout << "Lotfusspunkt: " << vecDropped << ", Distanz: " << dDist;
						std::cout << ", Ebene (x,y) = " << dX << ", " << dY << std::endl;
					}*/

					RecipPeak *pPeak = new RecipPeak();
					if(ih==0 && ik==0 && il==0)
						pPeak->SetColor(Qt::green);
					pPeak->setFlag(QGraphicsItem::ItemIgnoresTransformations);
					pPeak->setPos(dX * m_dScaleFactor, dY * m_dScaleFactor);
					pPeak->setData(TRIANGLE_NODE_TYPE_KEY, NODE_BRAGG);

					std::ostringstream ostrTip;
					ostrTip << "(" << ih << " " << ik << " " << il << ")";

					if(ih!=0 || ik!=0 || il!=0)
						pPeak->SetLabel(ostrTip.str().c_str());

					//std::string strAA = ::get_spec_char_utf8("AA")+::get_spec_char_utf8("sup-")+::get_spec_char_utf8("sup1");
					//ostrTip << "\ndistance to plane: " << dDist << " " << strAA;
					pPeak->setToolTip(QString::fromUtf8(ostrTip.str().c_str(), ostrTip.str().length()));

					m_vecPeaks.push_back(pPeak);
					m_scene.addItem(pPeak);

					const int iCent[] = {0,0,0};

					// 1st BZ
					if(ih==iCent[0] && ik==iCent[1] && il==iCent[2])
					{
						ublas::vector<double> vecCentral(2);
						vecCentral[0] = dX;
						vecCentral[1] = dY;
						
						//log_debug("Central ", ih, ik, il, ": ", vecCentral);
						m_bz.SetCentralReflex(vecCentral);
					}
					// TODO: check if 2 next neighbours is sufficient for all space groups
					else if(std::abs(ih-iCent[0])<=2 
							&& std::abs(ik-iCent[1])<=2 
							&& std::abs(il-iCent[2])<=2)
					{
						ublas::vector<double> vecN(2);
						vecN[0] = dX;
						vecN[1] = dY;
						
						//log_debug("Reflex: ", vecN);
						m_bz.AddReflex(vecN);
					}
				}
			}

	m_bz.CalcBZ();

	//for(RecipPeak* pPeak : m_vecPeaks)
	//	pPeak->SetBZ(&m_bz);

	m_scene.emitAllParams();
	this->update();
}

ublas::vector<double> ScatteringTriangle::GetHKLFromPlanePos(double x, double y) const
{
	if(!HasPeaks())
		return ublas::vector<double>();

	ublas::vector<double> vec = x*::get_column(m_matPlane, 0)
								+ y*::get_column(m_matPlane, 1);
	return m_recip.GetHKL(vec);
}

ublas::vector<double> ScatteringTriangle::GetQVecPlane(bool bSmallQ) const
{
	ublas::vector<double> vecQPlane;

	if(bSmallQ)
		vecQPlane = qpoint_to_vec(mapFromItem(m_pNodeGq,0,0))
						- qpoint_to_vec(mapFromItem(m_pNodeKfQ, 0, 0));
	else
		vecQPlane = qpoint_to_vec(mapFromItem(m_pNodeKiQ,0,0))
						- qpoint_to_vec(mapFromItem(m_pNodeKfQ,0,0));

	vecQPlane[1] = -vecQPlane[1];
	vecQPlane /= m_dScaleFactor;

	return vecQPlane;
}

ublas::vector<double> ScatteringTriangle::GetQVec(bool bSmallQ, bool bRLU) const
{
	ublas::vector<double> vecQPlane = GetQVecPlane(bSmallQ);

	ublas::vector<double> vecQ;
	if(bRLU)
		vecQ = GetHKLFromPlanePos(vecQPlane[0], vecQPlane[1]);
	else
		vecQ = vecQPlane[0]*::get_column(m_matPlane, 0)
				+ vecQPlane[1]*::get_column(m_matPlane, 1);

	return vecQ;
}

ublas::vector<double> ScatteringTriangle::GetKiVecPlane() const
{
	ublas::vector<double> vecKi = qpoint_to_vec(mapFromItem(m_pNodeKiQ,0,0))
								- qpoint_to_vec(mapFromItem(m_pNodeKiKf,0,0));
	vecKi[1] = -vecKi[1];
	vecKi /= m_dScaleFactor;
	return vecKi;
}

ublas::vector<double> ScatteringTriangle::GetKfVecPlane() const
{
	ublas::vector<double> vecKf = qpoint_to_vec(mapFromItem(m_pNodeKfQ,0,0))
								- qpoint_to_vec(mapFromItem(m_pNodeKiKf,0,0));
	vecKf[1] = -vecKf[1];
	vecKf /= m_dScaleFactor;
	return vecKf;
}

void ScatteringTriangle::ClearPeaks()
{
	m_bz.Clear();

	for(RecipPeak*& pPeak : m_vecPeaks)
	{
		if(pPeak)
		{
			m_scene.removeItem(pPeak);
			delete pPeak;
			pPeak = 0;
		}
	}
	m_vecPeaks.clear();
}


std::vector<ScatteringTriangleNode*> ScatteringTriangle::GetNodes()
{
	return std::vector<ScatteringTriangleNode*>
			{ m_pNodeKiQ, m_pNodeKiKf, m_pNodeKfQ, m_pNodeGq };
}

std::vector<std::string> ScatteringTriangle::GetNodeNames() const
{
	return std::vector<std::string>
		{ "kiQ", "kikf", "kfQ", "Gq" };
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

void ScatteringTriangleScene::SetDs(double dMonoD, double dAnaD)
{
	m_dMonoD = dMonoD;
	m_dAnaD = dAnaD;

	emitUpdate();
}

void ScatteringTriangleScene::emitUpdate()
{
	if(!m_pTri || !m_pTri->IsReady() || m_bDontEmitChange)
		return;

	TriangleOptions opts;
	opts.bChangedMonoTwoTheta = 1;
	opts.bChangedAnaTwoTheta = 1;
	opts.bChangedTwoTheta = 1;
	opts.bChangedTheta = 1;
	opts.dTwoTheta = m_pTri->GetTwoTheta(m_bSamplePosSense);
	opts.dTheta = m_pTri->GetTheta(m_bSamplePosSense);
	opts.dAnaTwoTheta = m_pTri->GetAnaTwoTheta(m_dAnaD, m_bAnaPosSense);
	opts.dMonoTwoTheta = m_pTri->GetMonoTwoTheta(m_dMonoD, m_bMonoPosSense);

	emit triangleChanged(opts);
}

void ScatteringTriangleScene::emitAllParams()
{
	if(!m_pTri || !m_pTri->IsReady())
		return;

	RecipParams parms;
	parms.d2Theta = m_pTri->GetTwoTheta(m_bSamplePosSense);
	parms.dTheta = m_pTri->GetTheta(m_bSamplePosSense);
	parms.dE = m_pTri->GetE();
	parms.dQ = m_pTri->GetQ();
	parms.dq = m_pTri->Getq();
	parms.dki = m_pTri->GetKi();
	parms.dkf = m_pTri->GetKf();
	parms.dKiQ = m_pTri->GetAngleKiQ(m_bSamplePosSense);
	parms.dKfQ = m_pTri->GetAngleKfQ(m_bSamplePosSense);
	parms.dAngleQVec0 = m_pTri->GetAngleQVec0();

	ublas::vector<double> vecQ = m_pTri->GetQVec(0,0);
	ublas::vector<double> vecQrlu = m_pTri->GetQVec(0,1);
	ublas::vector<double> vecq = m_pTri->GetQVec(1,0);
	ublas::vector<double> vecqrlu = m_pTri->GetQVec(1,1);
	ublas::vector<double> vecG = vecQ - vecq;
	ublas::vector<double> vecGrlu = vecQrlu - vecqrlu;

	const ublas::matrix<double>& matPlane = m_pTri->GetPlane();
	ublas::vector<double> vec0 = ::get_column(matPlane, 0);
	ublas::vector<double> vec1 = ::get_column(matPlane, 1);
	ublas::vector<double> vecUp = ::get_column(matPlane, 2);

	set_eps_0(vecQ); set_eps_0(vecQrlu);
	set_eps_0(vecq); set_eps_0(vecqrlu);
	set_eps_0(vecG); set_eps_0(vecGrlu);

	for(unsigned int i=0; i<3; ++i)
	{
		parms.Q[i] = vecQ[i];
		parms.Q_rlu[i] = vecQrlu[i];

		parms.q[i] = vecq[i];
		parms.q_rlu[i] = vecqrlu[i];

		parms.G[i] = vecG[i];
		parms.G_rlu[i] = vecGrlu[i];

		parms.orient_0[i] = vec0[i];
		parms.orient_1[i] = vec1[i];
		parms.orient_up[i] = vecUp[i];
	}

	CheckForSpurions();
	emit paramsChanged(parms);
}

// check for elastic spurions
void ScatteringTriangleScene::CheckForSpurions()
{
	typedef units::quantity<units::si::energy> energy;

	const ublas::vector<double> vecq = m_pTri->GetQVecPlane(1);
	const ublas::vector<double> vecKi = m_pTri->GetKiVecPlane();
	const ublas::vector<double> vecKf = m_pTri->GetKfVecPlane();
	energy E = m_pTri->GetE() * one_meV;
	energy Ei = k2E(m_pTri->GetKi()/angstrom);
	energy Ef = k2E(m_pTri->GetKf()/angstrom);

	// elastic ones
	ElasticSpurion spuris = check_elastic_spurion(vecKi, vecKf, vecq);

	// inelastic ones
	std::vector<InelasticSpurion> vecInelCKI = check_inelastic_spurions(1, Ei, Ef, E, 5);
	std::vector<InelasticSpurion> vecInelCKF = check_inelastic_spurions(0, Ei, Ef, E, 5);

	emit spurionInfo(spuris, vecInelCKI, vecInelCKF);
}

void ScatteringTriangleScene::tasChanged(const TriangleOptions& opts)
{
	if(!m_pTri || !m_pTri->IsReady())
		return;

	m_bDontEmitChange = 1;

	if(opts.bChangedMonoTwoTheta)
		m_pTri->SetMonoTwoTheta(opts.dMonoTwoTheta, m_dMonoD);
	if(opts.bChangedAnaTwoTheta)
		m_pTri->SetAnaTwoTheta(opts.dAnaTwoTheta, m_dAnaD);
	if(opts.bChangedTwoTheta)
		m_pTri->SetTwoTheta(m_bSamplePosSense?opts.dTwoTheta:-opts.dTwoTheta);
	if(opts.bChangedAngleKiVec0)
		m_pTri->RotateKiVec0To(m_bSamplePosSense, opts.dAngleKiVec0);

	m_bDontEmitChange = 0;
}

void ScatteringTriangleScene::SetSampleSense(bool bPos)
{
	m_bSamplePosSense = bPos;
	emitUpdate();
	emitAllParams();
}

void ScatteringTriangleScene::SetMonoSense(bool bPos)
{
	m_bMonoPosSense = bPos;
	emitUpdate();
	emitAllParams();
}

void ScatteringTriangleScene::SetAnaSense(bool bPos)
{
	m_bAnaPosSense = bPos;
	emitUpdate();
	emitAllParams();
}

void ScatteringTriangleScene::scaleChanged(double dTotalScale)
{
	if(!m_pTri)
		return;

	m_pTri->SetZoom(dTotalScale);
}

void ScatteringTriangleScene::mousePressEvent(QGraphicsSceneMouseEvent *pEvt)
{
	m_bMousePressed = 1;

	QGraphicsScene::mousePressEvent(pEvt);
}


static inline const QGraphicsItem* get_nearest_node(const QPointF& pt,
												const QGraphicsItem* pCurItem,
												const QList<QGraphicsItem*>& nodes)
{
	if(nodes.size()==0)
		return 0;

	double dMinLen = std::numeric_limits<double>::max();
	int iMinIdx = -1;

	for(int iNode=0; iNode<nodes.size(); ++iNode)
	{
		const QGraphicsItem *pNode = nodes[iNode];
		if(pNode == pCurItem || pNode->data(TRIANGLE_NODE_TYPE_KEY)!=NODE_BRAGG)
			continue;

		QLineF line(pt, pNode->scenePos());

		double dLen = line.length();
		if(dLen < dMinLen)
		{
			dMinLen = dLen;
			iMinIdx = iNode;
		}
	}

	if(iMinIdx < 0)
		return 0;
	return nodes[iMinIdx];
}

// snap pNode to a peak near pNodeOrg
void ScatteringTriangleScene::SnapToNearestPeak(ScatteringTriangleNode* pNode,
									const ScatteringTriangleNode* pNodeOrg)
{
	if(!pNode)
		return;
	if(!pNodeOrg)
		pNodeOrg = pNode;

	const QGraphicsItem *pNearestNode =
					get_nearest_node(pNodeOrg->pos(), pNode, items());

	if(pNearestNode)
		pNode->setPos(pNearestNode->pos());
}

void ScatteringTriangleScene::Snapq()
{
	if(m_bSnapq && m_pTri)
		SnapToNearestPeak(m_pTri->GetNodeGq(), m_pTri->GetNodeKfQ());
}

void ScatteringTriangleScene::setSnapq(bool bSnap)
{
	m_bSnapq = bSnap;
	Snapq();
}

void ScatteringTriangleScene::mouseMoveEvent(QGraphicsSceneMouseEvent *pEvt)
{
	bool bHandled = 0;


	// tooltip
	if(m_pTri)
	{
		const double dX = pEvt->scenePos().x()/m_pTri->GetScaleFactor();
		const double dY = -pEvt->scenePos().y()/m_pTri->GetScaleFactor();

		ublas::vector<double> vecHKL = m_pTri->GetHKLFromPlanePos(dX, dY);
		set_eps_0(vecHKL);

		if(vecHKL.size()==3)
		{
			//std::ostringstream ostrPos;
			//ostrPos << "(" << vecHKL[0] << ", " << vecHKL[1] << ", " << vecHKL[2]  << ")";
			//QToolTip::showText(pEvt->screenPos(), ostrPos.str().c_str());

			emit coordsChanged(vecHKL[0], vecHKL[1], vecHKL[2]);
		}
	}


	// node dragging
	if(m_bMousePressed)
	{
		QGraphicsItem* pCurItem = mouseGrabberItem();
		int iNodeType = NODE_OTHER;
		if(pCurItem)
			iNodeType = pCurItem->data(TRIANGLE_NODE_TYPE_KEY).toInt();

		if(pCurItem && (m_bSnap || (m_bSnapq && iNodeType == NODE_q)))
		{
			QList<QGraphicsItem*> nodes = items();
			const QGraphicsItem *pNearestNode =
							get_nearest_node(pEvt->scenePos(), pCurItem, nodes);

			if(pNearestNode)
			{
				pCurItem->setPos(pNearestNode->pos());
				bHandled = 1;
			}
		}
	}

	if(!bHandled)
		QGraphicsScene::mouseMoveEvent(pEvt);
}

void ScatteringTriangleScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *pEvt)
{
	m_bMousePressed = 0;

	QGraphicsScene::mouseReleaseEvent(pEvt);
}

void ScatteringTriangleScene::keyPressEvent(QKeyEvent *pEvt)
{
	if(pEvt->key() == Qt::Key_Control)
		m_bSnap = 1;

	QGraphicsScene::keyPressEvent(pEvt);
}

void ScatteringTriangleScene::keyReleaseEvent(QKeyEvent *pEvt)
{
	if(pEvt->key() == Qt::Key_Control)
		m_bSnap = 0;

	QGraphicsScene::keyReleaseEvent(pEvt);
}


// --------------------------------------------------------------------------------


ScatteringTriangleView::ScatteringTriangleView(QWidget* pParent)
						: QGraphicsView(pParent)
{
	setRenderHints(QPainter::Antialiasing);
	setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	setDragMode(QGraphicsView::ScrollHandDrag);
	setMouseTracking(1);
}

ScatteringTriangleView::~ScatteringTriangleView()
{}

void ScatteringTriangleView::wheelEvent(QWheelEvent *pEvt)
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

#include "scattering_triangle.moc"
