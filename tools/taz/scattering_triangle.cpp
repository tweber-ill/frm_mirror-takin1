/*
 * Scattering Triangle
 * @author tweber
 * @date feb-2014
 */

#include "helper/flags.h"
#include "helper/neutrons.hpp"
#include "scattering_triangle.h"
#include <iostream>
#include <cmath>
#include <sstream>


ScatteringTriangleNode::ScatteringTriangleNode(ScatteringTriangle* pSupItem)
	: m_pParentItem(pSupItem)
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
	painter->setBrush(Qt::red);
	painter->drawEllipse(QRectF(-3., -3., 6., 6.));
	painter->setPen(Qt::red);
	painter->drawText(0., 14., m_strLabel);
}

// --------------------------------------------------------------------------------


ScatteringTriangle::ScatteringTriangle(ScatteringTriangleScene& scene)
				: m_scene(scene)
{
	m_pNodeKiQ = new ScatteringTriangleNode(this);
	m_pNodeKiKf = new ScatteringTriangleNode(this);
	m_pNodeKfQ = new ScatteringTriangleNode(this);

	m_pNodeKiKf->setFlag(QGraphicsItem::ItemIsMovable);
	m_pNodeKfQ->setFlag(QGraphicsItem::ItemIsMovable);

	m_pNodeKiQ->setPos(0., 0.);
	m_pNodeKiKf->setPos(50., -100.);
	m_pNodeKfQ->setPos(100., 0.);

	m_scene.addItem(m_pNodeKiQ);
	m_scene.addItem(m_pNodeKiKf);
	m_scene.addItem(m_pNodeKfQ);

	setAcceptedMouseButtons(0);

	m_bReady = 1;
}

ScatteringTriangle::~ScatteringTriangle()
{
	m_bReady = 0;

	delete m_pNodeKiQ;
	delete m_pNodeKiKf;
	delete m_pNodeKfQ;

	ClearPeaks();
}

void ScatteringTriangle::nodeMoved(const ScatteringTriangleNode* pNode)
{
	if(!m_bReady) return;

	m_scene.emitUpdate();
	this->update();
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

	const double dQ = lineQ.length()/m_dScaleFactor;
	const double dKi = lineKi.length()/m_dScaleFactor;
	const double dKf = lineKf.length()/m_dScaleFactor;
	const double dE = get_energy_transfer(dKi/angstrom, dKf/angstrom) / one_eV * 1000.;

	std::wostringstream ostrQ, ostrKi, ostrKf, ostrE;
	ostrQ.precision(3); ostrKi.precision(3); ostrKf.precision(3); ostrE.precision(3);
	ostrQ << L"Q = " << dQ << L" 1/\x212B";
	ostrKi << L"ki = " << dKi << L" 1/\x212B";
	ostrKf << L"kf = " << dKf << L" 1/\x212B";
	ostrE << L"\x0394" << "E = " << dE << L" meV";

	painter->save();
	painter->rotate(-lineQ.angle());
	painter->drawText(QPointF(lineQ.length()/5.,12.), QString::fromWCharArray(ostrQ.str().c_str()));
	painter->drawText(QPointF(lineQ.length()/5.,26.), QString::fromWCharArray(ostrE.str().c_str()));
	painter->rotate(lineQ.angle());

	painter->rotate(-lineKi.angle());
	painter->drawText(QPointF(lineKi.length()/5.,-4.), QString::fromWCharArray(ostrKi.str().c_str()));
	painter->rotate(lineKi.angle());

	painter->translate(ptKiKf);
	painter->rotate(-lineKf.angle());
	painter->drawText(QPointF(lineKf.length()/5.,-4.), QString::fromWCharArray(ostrKf.str().c_str()));
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
								const Plane<double>& plane)
{
	ClearPeaks();

	ublas::vector<double> dir0 = plane.GetDir0();
	//ublas::vector<double> dir1 = plane.GetDir1();
	ublas::vector<double> dir1 = cross_3(plane.GetNorm(), dir0);

	dir0 /= ublas::norm_2(dir0);
	dir1 /= ublas::norm_2(dir1);

	ublas::matrix<double> matPlane = column_matrix(
			std::vector<ublas::vector<double> >{dir0, dir1, plane.GetNorm()});
	ublas::matrix<double> matPlaneinv;
	bool bInv = ::inverse(matPlane, matPlaneinv);
	if(!bInv)
	{
		std::cerr << "Error: Cannot invert scattering plane metric." << std::endl;
		return;
	}

	ublas::vector<double> vecNorm = cross_3(recip_unrot.GetVec(0), recip_unrot.GetVec(1));
	m_dAngleRot = -vec_angle(recip.GetVec(0), recip_unrot.GetVec(0), &vecNorm);
	//std::cout << m_dAngleRot/M_PI*180. << std::endl;

	const double dMaxPeaks = 5.;

	for(double h=-dMaxPeaks; h<=dMaxPeaks; h+=1.)
		for(double k=-dMaxPeaks; k<=dMaxPeaks; k+=1.)
			for(double l=-dMaxPeaks; l<=dMaxPeaks; l+=1.)
			{
				ublas::vector<double> vecPeak = recip.GetPos(h,k,l);
				double dDist = 0.;
				ublas::vector<double> vecDropped = plane.GetDroppedPerp(vecPeak, &dDist);

				if(::float_equal(dDist, 0., 0.01))
				{
					ublas::vector<double> vecCoord = ublas::prod(matPlaneinv, vecDropped);
					double dX = vecCoord[0];
					double dY = vecCoord[1];

					/*if(h==1 && k==0 && l==0)
					{
						std::cout << h << k << l << ": ";
						std::cout << "Lotfusspunkt: " << vecDropped << ", Distanz: " << dDist;
						std::cout << ", Ebene (x,y) = " << dX << ", " << dY << std::endl;
					}*/

					RecipPeak *pPeak = new RecipPeak();
					pPeak->setPos(dX * m_dScaleFactor, -dY * m_dScaleFactor);

					std::ostringstream ostrTip;
					ostrTip << "(" << int(h) << " " << int(k) << " " << int(l) << ")";
					pPeak->setToolTip(ostrTip.str().c_str());

					if(h!=0. || k!=0. || l!=0.)
						pPeak->SetLabel(ostrTip.str().c_str());

					//std::cout << ostrTip.str() << ": " << dX << ", " << dY << std::endl;

					m_vecPeaks.push_back(pPeak);
					m_scene.addItem(pPeak);
				}
			}

	this->update();
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

void ScatteringTriangleScene::CalcPeaks(const Lattice& lattice,
										const Lattice& recip, const Lattice& recip_unrot,
										const Plane<double>& plane)
{
	if(!m_pTri)
		return;

	m_pTri->CalcPeaks(lattice, recip, recip_unrot, plane);
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

#include "scattering_triangle.moc"
