/*
 * Scattering Triangle
 * @author tweber
 * @date feb-2014
 */

#include "helper/flags.h"
#include "helper/neutrons.hpp"
#include "helper/spec_char.h"
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

	m_scene.emitUpdate();
	m_scene.emitAllParams();
	this->update();
}

QRectF ScatteringTriangle::boundingRect() const
{
	return QRectF(-500.*m_dZoom, -500.*m_dZoom,
					1000.*m_dZoom, 1000.*m_dZoom);
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

void ScatteringTriangle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
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

	painter->drawLine(lineQ);
	painter->drawLine(lineKi);
	painter->drawLine(lineKf);

	if(m_bqVisible)
	{
		painter->drawLine(lineG);
		painter->drawLine(lineq);
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
	painter->drawText(QPointF(lineQ.length()/5.,12.), QString::fromWCharArray(ostrQ.str().c_str()));
	painter->rotate(lineQ.angle());

	painter->rotate(-lineKi.angle());
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
		painter->translate(ptKfQ);
		painter->rotate(-lineq.angle());
		painter->drawText(QPointF(lineq.length()/5.,-4.), QString::fromWCharArray(ostrq.str().c_str()));
		painter->rotate(lineq.angle());
		painter->translate(-ptKfQ);

		painter->rotate(-lineG.angle());
		painter->drawText(QPointF(lineG.length()/5.,-4.), QString::fromWCharArray(ostrG.str().c_str()));
		painter->rotate(lineG.angle());
	}

	painter->restore();


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
		double dArcSize = (pLines1[i]->length() + pLines2[i]->length()) / 2. / 3.;
		double dBeginArcAngle = pLines1[i]->angle() + 180.;
		double dArcAngle = pLines1[i]->angleTo(*pLines2[i]) - 180.;

		painter->setPen(Qt::blue);
		painter->drawArc(QRectF(pPoints[i]->x()-dArcSize/2., pPoints[i]->y()-dArcSize/2., dArcSize, dArcSize),
						dBeginArcAngle*16., dArcAngle*16.);

		const std::wstring& strDEG = ::get_spec_char_utf16("deg");
		std::wostringstream ostrAngle;
		ostrAngle.precision(4);
		ostrAngle << std::fabs(dArcAngle) << strDEG;

		QPointF ptDirOut = *pPoints[i] - ptMid;
		ptDirOut /= std::sqrt(ptDirOut.x()*ptDirOut.x() + ptDirOut.y()*ptDirOut.y());
		ptDirOut *= 15.;

		QPointF ptText = *pPoints[i] + ptDirOut;
		painter->drawText(ptText, QString::fromWCharArray(ostrAngle.str().c_str()));
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

double ScatteringTriangle::GetAngleKiQ() const
{
	ublas::vector<double> vecKi = qpoint_to_vec(mapFromItem(m_pNodeKiQ,0,0))
								- qpoint_to_vec(mapFromItem(m_pNodeKiKf,0,0));
	ublas::vector<double> vecQ = qpoint_to_vec(mapFromItem(m_pNodeKiQ,0,0))
								- qpoint_to_vec(mapFromItem(m_pNodeKfQ,0,0));

	const double dAngle = vec_angle(vecKi) - vec_angle(vecQ);
	return dAngle;
}

double ScatteringTriangle::GetAngleKfQ() const
{
	ublas::vector<double> vecKf = qpoint_to_vec(mapFromItem(m_pNodeKfQ,0,0))
								- qpoint_to_vec(mapFromItem(m_pNodeKiKf,0,0));
	ublas::vector<double> vecQ = qpoint_to_vec(mapFromItem(m_pNodeKiQ,0,0))
								- qpoint_to_vec(mapFromItem(m_pNodeKfQ,0,0));

	const double dAngle = vec_angle(vecKf) - vec_angle(vecQ);
	return dAngle;
}

double ScatteringTriangle::GetTheta(bool bPosSense) const
{
	ublas::vector<double> vecKi = qpoint_to_vec(mapFromItem(m_pNodeKiQ,0,0))
								- qpoint_to_vec(mapFromItem(m_pNodeKiKf,0,0));

	ublas::vector<double> vecSampleDirX(2);
	vecSampleDirX[0] = 1.;
	vecSampleDirX[1] = 0.;

	double dTh = vec_angle(vecKi) - vec_angle(vecSampleDirX) - M_PI/2.;
	dTh += m_dAngleRot;
	if(!bPosSense)
		dTh = -dTh;

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


void ScatteringTriangle::CalcPeaks(const Lattice& lattice,
								const Lattice& recip, const Lattice& recip_unrot,
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

	dir0 /= ublas::norm_2(dir0);
	dir1 /= ublas::norm_2(dir1);

	m_matPlane = column_matrix(
			std::vector<ublas::vector<double> >{dir0, dir1, plane.GetNorm()});
	bool bInv = ::inverse(m_matPlane, m_matPlane_inv);
	if(!bInv)
	{
		std::cerr << "Error: Cannot invert scattering plane metric." << std::endl;
		return;
	}

	ublas::vector<double> vecNorm = cross_3(m_recip_unrot.GetVec(0), m_recip_unrot.GetVec(1));
	try
	{
		m_dAngleRot = -vec_angle(m_recip.GetVec(0), m_recip_unrot.GetVec(0), &vecNorm);
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return;
	}
	//std::cout << m_dAngleRot/M_PI*180. << std::endl;

	for(double h=-m_dMaxPeaks; h<=m_dMaxPeaks; h+=1.)
		for(double k=-m_dMaxPeaks; k<=m_dMaxPeaks; k+=1.)
			for(double l=-m_dMaxPeaks; l<=m_dMaxPeaks; l+=1.)
			{
				if(pSpaceGroup)
				{
					int ih = int(h), ik = int(k), il = int(l);

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
					double dY = vecCoord[1];

					/*if(h==1 && k==0 && l==0)
					{
						std::cout << h << k << l << ": ";
						std::cout << "Lotfusspunkt: " << vecDropped << ", Distanz: " << dDist;
						std::cout << ", Ebene (x,y) = " << dX << ", " << dY << std::endl;
					}*/

					RecipPeak *pPeak = new RecipPeak();
					if(float_equal(h, 0.) && float_equal(k,0.) && float_equal(l,0.))
						pPeak->SetColor(Qt::green);
					pPeak->setFlag(QGraphicsItem::ItemIgnoresTransformations);
					pPeak->setPos(dX * m_dScaleFactor, -dY * m_dScaleFactor);
					pPeak->setData(TRIANGLE_NODE_TYPE_KEY, NODE_BRAGG);

					std::ostringstream ostrTip;
					ostrTip << "(" << int(h) << " " << int(k) << " " << int(l) << ")";

					if(h!=0. || k!=0. || l!=0.)
						pPeak->SetLabel(ostrTip.str().c_str());

					//std::string strAA = ::get_spec_char_utf8("AA")+::get_spec_char_utf8("sup-")+::get_spec_char_utf8("sup1");
					//ostrTip << "\ndistance to plane: " << dDist << " " << strAA;
					pPeak->setToolTip(QString::fromUtf8(ostrTip.str().c_str(), ostrTip.str().length()));

					m_vecPeaks.push_back(pPeak);
					m_scene.addItem(pPeak);
				}
			}

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
		vecQPlane = qpoint_to_vec(mapFromItem(m_pNodeKfQ,0,0))
						- qpoint_to_vec(mapFromItem(m_pNodeGq, 0, 0));
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
	parms.dKiQ = m_pTri->GetAngleKiQ();
	parms.dKfQ = m_pTri->GetAngleKfQ();

	ublas::vector<double> vecQ = m_pTri->GetQVec(0,0);
	ublas::vector<double> vecQrlu = m_pTri->GetQVec(0,1);
	ublas::vector<double> vecq = m_pTri->GetQVec(1,0);
	ublas::vector<double> vecqrlu = m_pTri->GetQVec(1,1);

	set_eps_0(vecQ); set_eps_0(vecQrlu);
	set_eps_0(vecq); set_eps_0(vecqrlu);

	for(unsigned int i=0; i<3; ++i)
	{
		parms.Q[i] = vecQ[i];
		parms.Q_rlu[i] = vecQrlu[i];
		parms.q[i] = vecq[i];
		parms.q_rlu[i] = vecqrlu[i];
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
	std::vector<InelasticSpurion> vecInelCKI = check_inelastic_spurions(0, Ei, Ef, E, 5);
	std::vector<InelasticSpurion> vecInelCKF = check_inelastic_spurions(1, Ei, Ef, E, 5);

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

	m_bDontEmitChange = 0;
}

void ScatteringTriangleScene::SetSampleSense(bool bPos)
{
	m_bSamplePosSense = bPos;
	emitUpdate();
}

void ScatteringTriangleScene::SetMonoSense(bool bPos)
{
	m_bMonoPosSense = bPos;
	emitUpdate();
}

void ScatteringTriangleScene::SetAnaSense(bool bPos)
{
	m_bAnaPosSense = bPos;
	emitUpdate();
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

void ScatteringTriangleScene::SnapToNearestPeak(ScatteringTriangleNode* pNode)
{
	if(!pNode)
		return;

	const QGraphicsItem *pNearestNode =
					get_nearest_node(pNode->pos(), pNode, items());

	if(pNearestNode)
		pNode->setPos(pNearestNode->pos());
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
			std::ostringstream ostrPos;
			ostrPos << "(" << vecHKL[0] << ", " << vecHKL[1] << ", " << vecHKL[2]  << ")";
			QToolTip::showText(pEvt->screenPos(), ostrPos.str().c_str());
		}
	}


	// node dragging
	if(m_bMousePressed)
	{
		QGraphicsItem* pCurItem = mouseGrabberItem();
		int iNodeType = NODE_OTHER;
		if(pCurItem)
			iNodeType = pCurItem->data(TRIANGLE_NODE_TYPE_KEY).toInt();

		if(pCurItem && (m_bSnap || iNodeType == NODE_q))
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
