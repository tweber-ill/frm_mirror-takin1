/*
 * Scattering Triangle
 * @author tweber
 * @date feb-2014
 * @license GPLv2
 */

#include "tlibs/helper/flags.h"
#include "tlibs/math/neutrons.hpp"
#include "tlibs/string/spec_char.h"
#include "tlibs/helper/log.h"
#include "helper/globals.h"
#include "helper/formfact.h"
#include "scattering_triangle.h"

#include <QToolTip>
#include <iostream>
#include <cmath>
#include <sstream>

namespace units = boost::units;
namespace co = boost::units::si::constants::codata;


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

void ScatteringTriangleNode::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
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

QVariant ScatteringTriangleNode::itemChange(GraphicsItemChange change, const QVariant &val)
{
	QVariant var = QGraphicsItem::itemChange(change, val);

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
	return QRectF(-35., -10., 70., 50.);
}

void RecipPeak::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	painter->setBrush(m_color);
	painter->drawEllipse(QRectF(-3., -3., 6., 6.));

	if(m_strLabel != "")
	{
		painter->setPen(m_color);
		QRectF rect = boundingRect();
		rect.setTop(rect.top()+16.5);
		//painter->drawRect(rect);
		painter->drawText(rect, Qt::AlignHCenter|Qt::AlignTop, m_strLabel);
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

	m_pNodeKiQ->setData(TRIANGLE_NODE_TYPE_KEY, NODE_KIQ);
	m_pNodeKiKf->setData(TRIANGLE_NODE_TYPE_KEY, NODE_KIKF);
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
	m_pNodeKiKf->setPos(80., -150.);
	m_pNodeKfQ->setPos(160., 0.);
	m_pNodeGq->setPos(160., 0.);

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
		SnapToNearestPeak(GetNodeGq(), GetNodeKfQ());

	update();
	m_scene.emitUpdate();
	m_scene.emitAllParams();
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

void ScatteringTriangle::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
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

			const tl::Brillouin2D<double>::t_vertices<double>& verts = m_bz.GetVertices();
			for(const tl::Brillouin2D<double>::t_vecpair<double>& vertpair : verts)
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



	// Powder lines
	{
		QPen penOrg = painter->pen();
		painter->setPen(Qt::red);

		const typename tl::Powder<int>::t_peaks_unique& powderpeaks = m_powder.GetUniquePeaks();
		for(const typename tl::Powder<int>::t_peak& powderpeak : powderpeaks)
		{
			const int ih = std::get<0>(powderpeak);
			const int ik = std::get<1>(powderpeak);
			const int il = std::get<2>(powderpeak);

			if(ih==0 && ik==0 && il==0) continue;

			std::ostringstream ostrPowderLine;
			ostrPowderLine << "(" << ih << " "<< ik << " " << il << ")";

			ublas::vector<double> vec = m_powder.GetRecipLatticePos(double(ih), double(ik), double(il));
			double drad = std::sqrt(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);
			drad *= m_dScaleFactor*m_dZoom;

			painter->drawEllipse(ptKiQ, drad, drad);
			painter->drawText(ptKiQ + QPointF(0., drad), ostrPowderLine.str().c_str());
		}
		painter->setPen(penOrg);
	}



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

	const std::wstring strAA = tl::get_spec_char_utf16("AA") + tl::get_spec_char_utf16("sup-") + tl::get_spec_char_utf16("sup1");
	const std::wstring& strDelta = tl::get_spec_char_utf16("Delta");

	std::wostringstream ostrQ, ostrKi, ostrKf, ostrE, ostrq, ostrG;
	ostrQ.precision(g_iPrecGfx); ostrE.precision(g_iPrecGfx);
	ostrKi.precision(g_iPrecGfx); ostrKf.precision(g_iPrecGfx);
	ostrG.precision(g_iPrecGfx); ostrq.precision(g_iPrecGfx);

	double dQ = GetQ();	tl::set_eps_0(dQ, g_dEpsGfx);
	double dKi = GetKi();	tl::set_eps_0(dKi, g_dEpsGfx);
	double dKf = GetKf();	tl::set_eps_0(dKf, g_dEpsGfx);
	double dE = GetE();	tl::set_eps_0(dE, g_dEpsGfx);

	ostrQ << L"Q = " << dQ << " " << strAA;
	ostrKi << L"ki = " << dKi << " " << strAA;
	ostrKf << L"kf = " << dKf << " " << strAA;
	ostrE << strDelta << "E = " << dE << " meV";
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

	std::vector<const QLineF*> vecLinesArrow = {&lineKi2, &lineKf, &lineQ2};
	std::vector<const QPointF*> vecPointsArrow = {&ptKiKf, &ptKiKf, &ptKfQ};

	std::vector<bool> vecDrawAngles = {1,1,1};
	std::vector<QColor> vecColor {Qt::black, Qt::black, Qt::red};

	QLineF lineG2(ptGq, ptKiQ);
	QLineF lineq2(ptGq, ptKfQ);

	if(m_bqVisible)
	{
		vecLinesArrow.push_back(&lineq);
		vecLinesArrow.push_back(&lineG2);

		vecPointsArrow.push_back(&ptKfQ);
		vecPointsArrow.push_back(&ptGq);

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

			const std::wstring& strDEG = tl::get_spec_char_utf16("deg");
			std::wostringstream ostrAngle;
			ostrAngle.precision(g_iPrecGfx);
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
	const double dE = tl::get_energy_transfer(dKi/tl::angstrom, dKf/tl::angstrom) / tl::one_eV * 1000.;
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
	vecQ = -vecQ;
	return tl::vec_angle(vecQ);
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
		double dAngle = tl::get_angle_ki_Q(GetKi()/tl::angstrom, GetKf()/tl::angstrom, GetQ()/tl::angstrom, bPosSense) / units::si::radians;
		//std::cout << "tt=" << dTT << ", kiQ="<<dAngle << std::endl;

		if(std::fabs(dTT) > M_PI)
			dAngle = -dAngle;

		return dAngle;
	}
	catch(const std::exception& ex)
	{
		tl::log_err(ex.what());
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
		double dAngle = M_PI-tl::get_angle_kf_Q(GetKi()/tl::angstrom, GetKf()/tl::angstrom, GetQ()/tl::angstrom, bPosSense) / units::si::radians;

		if(std::fabs(dTT) > M_PI)
			dAngle = -dAngle;

		return dAngle;
	}
	catch(const std::exception& ex)
	{
		tl::log_err(ex.what());
		return 0.;
	}
}

double ScatteringTriangle::GetTheta(bool bPosSense) const
{
	ublas::vector<double> vecKi = qpoint_to_vec(mapFromItem(m_pNodeKiQ,0,0))
						- qpoint_to_vec(mapFromItem(m_pNodeKiKf,0,0));
	vecKi /= ublas::norm_2(vecKi);

	double dTh = tl::vec_angle(vecKi) - M_PI/2.;
	dTh += m_dAngleRot;
	if(!bPosSense)
		dTh = -dTh;

	//tl::log_info("theta: ", dTh/M_PI*180.);
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

	double dTT = tl::vec_angle(vecKi) - tl::vec_angle(vecKf);
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
	return tl::get_mono_twotheta(dKi/tl::angstrom, dMonoD*tl::angstrom, bPosSense) / units::si::radians;
}

double ScatteringTriangle::GetAnaTwoTheta(double dAnaD, bool bPosSense) const
{
	ublas::vector<double> vecKf = qpoint_to_vec(mapFromItem(m_pNodeKfQ, 0, 0))
									- qpoint_to_vec(mapFromItem(m_pNodeKiKf, 0, 0));
	double dKf = ublas::norm_2(vecKf) / m_dScaleFactor;
	return tl::get_mono_twotheta(dKf/tl::angstrom, dAnaD*tl::angstrom, bPosSense) / units::si::radians;
}

void ScatteringTriangle::SetAnaTwoTheta(double dTT, double dAnaD)
{
	dTT = std::fabs(dTT);
	double dKf  = M_PI / std::sin(dTT/2.) / dAnaD;
	dKf *= m_dScaleFactor;

	const ublas::vector<double> vecNodeKiKf = qpoint_to_vec(mapFromItem(m_pNodeKiKf,0,0));
	const ublas::vector<double> vecNodeKfQ = qpoint_to_vec(mapFromItem(m_pNodeKfQ,0,0));
	ublas::vector<double> vecKf = qpoint_to_vec(mapFromItem(m_pNodeKfQ,0,0))
		- vecNodeKiKf;

	vecKf /= ublas::norm_2(vecKf);
	ublas::vector<double> vecKf_new = vecKf * dKf;

	m_pNodeKfQ->setPos(vec_to_qpoint(vecNodeKiKf + vecKf_new));
	nodeMoved(m_pNodeKfQ);
}

void ScatteringTriangle::SetMonoTwoTheta(double dTT, double dMonoD)
{
	const double dSampleTT = GetTwoTheta(1);

	dTT = std::fabs(dTT);
	double dKi  = M_PI / std::sin(dTT/2.) / dMonoD;
	dKi *= m_dScaleFactor;

	const ublas::vector<double> vecNodeKiKf = qpoint_to_vec(mapFromItem(m_pNodeKiKf,0,0));
	const ublas::vector<double> vecNodeKiQ = qpoint_to_vec(mapFromItem(m_pNodeKiQ,0,0));
	const ublas::vector<double> vecKi_old = qpoint_to_vec(mapFromItem(m_pNodeKiQ, 0, 0))
		- qpoint_to_vec(mapFromItem(m_pNodeKiKf, 0, 0));

	ublas::vector<double> vecKi = vecKi_old;
	vecKi /= ublas::norm_2(vecKi);
	ublas::vector<double> vecKi_new = vecKi * dKi;

	m_pNodeKiKf->setPos(vec_to_qpoint(vecNodeKiQ - vecKi_new));
	nodeMoved(m_pNodeKiKf);

	SetTwoTheta(dSampleTT);		// m_pNodeKfQ also moved!
}

void ScatteringTriangle::SetTwoTheta(double dTT)
{
	const ublas::vector<double> vecNodeKiKf = qpoint_to_vec(mapFromItem(m_pNodeKiKf,0,0));
	const ublas::vector<double> vecKi = qpoint_to_vec(mapFromItem(m_pNodeKiQ,0,0))
		- qpoint_to_vec(mapFromItem(m_pNodeKiKf,0,0));
	const ublas::vector<double> vecKf = qpoint_to_vec(mapFromItem(m_pNodeKfQ,0,0))
		- vecNodeKiKf;

	ublas::vector<double> vecKf_new = ublas::prod(tl::rotation_matrix_2d(-dTT), vecKi);

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

	ublas::matrix<double> matRot = tl::rotation_matrix_2d(dCurAngle - dAngle);
	vecNodeKiKf = ublas::prod(matRot, vecNodeKiKf);
	vecNodeKfQ = ublas::prod(matRot, vecNodeKfQ);

	m_pNodeKiKf->setPos(vec_to_qpoint(vecNodeKiKf));
	m_pNodeKfQ->setPos(vec_to_qpoint(vecNodeKfQ));

	nodeMoved(m_pNodeKiKf);
	nodeMoved(m_pNodeKfQ);
}

void ScatteringTriangle::CalcPeaks(const tl::Lattice<double>& lattice,
	const tl::Lattice<double>& recip, const tl::Lattice<double>& recip_unrot,
	const tl::Plane<double>& plane, const SpaceGroup* pSpaceGroup,
	bool bIsPowder, const std::vector<AtomPos>* pvecAtomPos)
{
	ClearPeaks();
	m_powder.clear();
	m_kdLattice.Unload();

	m_lattice = lattice;
	m_recip = recip;
	m_recip_unrot = recip_unrot;

	m_powder.SetRecipLattice(&m_recip);

	const ublas::matrix<double> matA = m_lattice.GetMetric();
	const ublas::matrix<double> matB = m_recip.GetMetric();

	ublas::vector<double> dir0 = plane.GetDir0();
	//ublas::vector<double> dir1 = plane.GetDir1();
	ublas::vector<double> dir1 = tl::cross_3(plane.GetNorm(), dir0);

	double dDir0Len = ublas::norm_2(dir0);
	double dDir1Len = ublas::norm_2(dir1);

	if(tl::float_equal(dDir0Len, 0.) || tl::float_equal(dDir1Len, 0.)
		|| tl::is_nan_or_inf<double>(dDir0Len) || tl::is_nan_or_inf<double>(dDir1Len))
	{
		tl::log_err("Invalid scattering plane.");
		return;
	}

	dir0 /= dDir0Len;
	dir1 /= dDir1Len;

	m_matPlane = tl::column_matrix({dir0, dir1, plane.GetNorm()});
	bool bInv = tl::inverse(m_matPlane, m_matPlane_inv);
	if(!bInv)
	{
		tl::log_err("Cannot invert scattering plane metric.");
		return;
	}


	// crystal rotation angle
	try
	{
		ublas::vector<double> vecUnrotX = plane.GetDroppedPerp(m_recip_unrot.GetVec(0));
		ublas::vector<double> vecRotX = plane.GetDroppedPerp(m_recip.GetVec(0));

		const ublas::vector<double> &vecNorm = plane.GetNorm();
		m_dAngleRot = -tl::vec_angle(vecRotX, vecUnrotX, &vecNorm);

		//std::cout << "rotation angle: " << m_dAngleRot/M_PI*180. << std::endl;
	}
	catch(const std::exception& ex)
	{
		tl::log_err(ex.what());
		return;
	}



	//const ublas::matrix<double> matB = recip.GetMetric();

	// structure factors
#ifdef USE_CLP
	ScatlenList lstsl;
	//FormfactList lstff;

	std::vector<ublas::vector<double>> vecAllAtoms;
	std::vector<std::complex<double>> vecScatlens;

	std::vector<ublas::matrix<double>> vecSymTrafos;
	if(pSpaceGroup)
		pSpaceGroup->GetSymTrafos(vecSymTrafos);
	//if(pSpaceGroup) std::cout << pSpaceGroup->GetName() << std::endl;

	if(vecSymTrafos.size() && /*g_bHasFormfacts &&*/ g_bHasScatlens && pvecAtomPos && pvecAtomPos->size())
	{
		for(unsigned int iAtom=0; iAtom<pvecAtomPos->size(); ++iAtom)
		{
			ublas::vector<double> vecAtom = (*pvecAtomPos)[iAtom].vecPos;
			// homogeneous coordinates
			vecAtom.resize(4,1); vecAtom[3] = 1.;
			const std::string& strElem = (*pvecAtomPos)[iAtom].strAtomName;
			//std::cout << strElem << ": " << vecAtom << std::endl;

			std::vector<ublas::vector<double>> vecSymPos =
				tl::generate_atoms<ublas::matrix<double>, ublas::vector<double>, std::vector>
					(vecSymTrafos, vecAtom);

			const ScatlenList::elem_type* pElem = lstsl.Find(strElem);
			if(!pElem)
			{
				tl::log_err("Element \"", strElem, "\" not found in scattering length table.");
				vecAllAtoms.clear();
				vecScatlens.clear();
				break;
			}


			const std::complex<double> b = pElem->GetCoherent() / 10.;
			//std::cout << "b = " << b << std::endl;

			for(ublas::vector<double> vecThisAtom : vecSymPos)
			{
				vecThisAtom.resize(3,1);
				vecThisAtom = matA * vecThisAtom;
				vecAllAtoms.push_back(std::move(vecThisAtom));
				vecScatlens.push_back(b);
			}
		}

		//for(const ublas::vector<double>& vecAt : vecAllAtoms)
		//	std::cout << vecAt << std::endl;
		//for(const std::complex<double>& b : vecScatlens)
		//	std::cout << b << std::endl;
	}
#endif


	std::list<std::vector<double>> lstPeaksForKd;

	for(int ih=-m_iMaxPeaks; ih<=m_iMaxPeaks; ++ih)
		for(int ik=-m_iMaxPeaks; ik<=m_iMaxPeaks; ++ik)
			for(int il=-m_iMaxPeaks; il<=m_iMaxPeaks; ++il)
			{
				const double h = double(ih);
				const double k = double(ik);
				const double l = double(il);

				if(pSpaceGroup)
				{
					if(!pSpaceGroup->HasReflection(ih, ik, il))
						continue;
				}


				const ublas::vector<double> vecPeak = m_recip.GetPos(h,k,l);
				//ublas::vector<double> vecPeak = matB * tl::make_vec({h,k,l});
				//const double dG = ublas::norm_2(vecPeak);


				double dF = -1.;
				std::string strStructfact;
#ifdef USE_CLP
				if(vecScatlens.size())
				{
					std::complex<double> cF =
						tl::structfact<double, std::complex<double>, ublas::vector<double>, std::vector>
							(vecAllAtoms, vecPeak, vecScatlens);
					double dFsq = (std::conj(cF)*cF).real();
					dF = std::sqrt(dFsq);
					tl::set_eps_0(dF, g_dEpsGfx);

					std::ostringstream ostrStructfact;
					ostrStructfact.precision(g_iPrecGfx);
					ostrStructfact << "F = " << dF;
					strStructfact = ostrStructfact.str();
				}
#endif

				if(bIsPowder)
					m_powder.AddPeak(ih, ik, il);

				if(!bIsPowder || (ih==0 && ik==0 && il==0))		// (000), i.e. direct beam, also needed for powder
				{
					// add peak in 1/A and rlu units
					lstPeaksForKd.push_back(std::vector<double>{vecPeak[0],vecPeak[1],vecPeak[2], h,k,l, dF});

					double dDist = 0.;
					ublas::vector<double> vecDropped = plane.GetDroppedPerp(vecPeak, &dDist);

					if(tl::float_equal(dDist, 0., m_dPlaneDistTolerance))
					{
						ublas::vector<double> vecCoord = ublas::prod(m_matPlane_inv, vecDropped);
						double dX = vecCoord[0];
						double dY = -vecCoord[1];

						RecipPeak *pPeak = new RecipPeak();
						if(ih==0 && ik==0 && il==0)
							pPeak->SetColor(Qt::green);
						pPeak->setFlag(QGraphicsItem::ItemIgnoresTransformations);
						pPeak->setPos(dX * m_dScaleFactor, dY * m_dScaleFactor);
						pPeak->setData(TRIANGLE_NODE_TYPE_KEY, NODE_BRAGG);

						std::ostringstream ostrTip;
						ostrTip << "(" << ih << " " << ik << " " << il << ")";
						if(strStructfact.length())
							ostrTip << "\n" << strStructfact;

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
			}

	if(!bIsPowder)
	{
		m_bz.CalcBZ();
		//for(RecipPeak* pPeak : m_vecPeaks)
		//	pPeak->SetBZ(&m_bz);

		m_kdLattice.Load(lstPeaksForKd, 3);
	}

	m_scene.emitAllParams();
	this->update();
}

ublas::vector<double> ScatteringTriangle::GetHKLFromPlanePos(double x, double y) const
{
	if(!HasPeaks())
		return ublas::vector<double>();

	ublas::vector<double> vec = x*tl::get_column(m_matPlane, 0)
								+ y*tl::get_column(m_matPlane, 1);
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
		vecQ = vecQPlane[0]*tl::get_column(m_matPlane, 0)
				+ vecQPlane[1]*tl::get_column(m_matPlane, 1);

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
	return std::vector<std::string>{ "kiQ", "kikf", "kfQ", "Gq" };
}


static std::pair<bool, QPointF>
get_nearest_elastic_kikf_pos(const QPointF& ptKiKf, const QPointF& ptKiQ, const QPointF& ptKfQ)
{
	std::pair<bool, QPointF> pairRet;
	bool& bOk = pairRet.first;
	QPointF& ptRet = pairRet.second;

	ublas::vector<double> vecKiQ = qpoint_to_vec(ptKiQ);
	ublas::vector<double> vecKfQ = qpoint_to_vec(ptKfQ);
	ublas::vector<double> vecKiKf = qpoint_to_vec(ptKiKf);
	ublas::vector<double> vecQ = vecKfQ - vecKiQ;

	ublas::vector<double> vecQperp = tl::make_vec({vecQ[1], -vecQ[0]});
	ublas::vector<double> vecQMid = vecKiQ + vecQ*0.5;

	tl::Line<double> lineQMidPerp(vecQMid, vecQperp);
	ublas::vector<double> vecDrop = lineQMidPerp.GetDroppedPerp(vecKiKf);
	ptRet = vec_to_qpoint(vecDrop);

	bOk = 1;
	return pairRet;
}


static std::tuple<bool, double, QPointF>
get_nearest_node(const QPointF& pt,
	const QGraphicsItem* pCurItem, const QList<QGraphicsItem*>& nodes,
	double dFactor, const tl::Powder<int>* pPowder=nullptr)
{
	if(nodes.size()==0)
		return std::tuple<bool, double, QPointF>(0, 0., QPointF());

	double dMinLen = std::numeric_limits<double>::max();
	int iMinIdx = -1;
	bool bHasPowderPeak = 0;

	// Bragg peaks
	for(int iNode=0; iNode<nodes.size(); ++iNode)
	{
		const QGraphicsItem *pNode = nodes[iNode];
		if(pNode == pCurItem || pNode->data(TRIANGLE_NODE_TYPE_KEY)!=NODE_BRAGG)
			continue;

		double dLen = QLineF(pt, pNode->scenePos()).length();
		if(dLen < dMinLen)
		{
			dMinLen = dLen;
			iMinIdx = iNode;
		}
	}

	const QGraphicsItem* pNodeOrigin = nullptr;
	for(const QGraphicsItem* pNode : nodes)
		if(pNode->data(TRIANGLE_NODE_TYPE_KEY) == NODE_KIQ)
		{
			pNodeOrigin = pNode;
			break;
		}



	// Powder peaks
	QPointF ptPowder;
	if(pNodeOrigin && pPowder)
	{
		ublas::vector<double> vecOrigin = qpoint_to_vec(pNodeOrigin->scenePos());
		ublas::vector<double> vecPt = qpoint_to_vec(pt);
		ublas::vector<double> vecOriginPt = vecPt-vecOrigin;
		const double dDistToOrigin = ublas::norm_2(vecOriginPt);
		vecOriginPt /= dDistToOrigin;

		const typename tl::Powder<int>::t_peaks_unique& powderpeaks = pPowder->GetUniquePeaks();
		for(const typename tl::Powder<int>::t_peak& powderpeak : powderpeaks)
		{
			const int ih = std::get<0>(powderpeak);
			const int ik = std::get<1>(powderpeak);
			const int il = std::get<2>(powderpeak);
			if(ih==0 && ik==0 && il==0) continue;

			ublas::vector<double> vec = pPowder->GetRecipLatticePos(double(ih), double(ik), double(il));
			double drad = std::sqrt(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);
			drad *= dFactor;

			if(std::fabs(drad-dDistToOrigin) < dMinLen)
			{
				bHasPowderPeak = 1;
				dMinLen = std::fabs(drad-dDistToOrigin);

				ublas::vector<double> vecPowder = vecOrigin + vecOriginPt*drad;
				ptPowder = vec_to_qpoint(vecPowder);
			}
		}
	}



	if(bHasPowderPeak)
	{
		return std::tuple<bool, double, QPointF>(1, dMinLen, ptPowder);
	}
	else
	{
		if(iMinIdx < 0)
			return std::tuple<bool, double, QPointF>(0, 0., QPointF());

		return std::tuple<bool, double, QPointF>(1, dMinLen, nodes[iMinIdx]->pos());
	}
}

// snap pNode to a peak near pNodeOrg
void ScatteringTriangle::SnapToNearestPeak(ScatteringTriangleNode* pNode,
	const ScatteringTriangleNode* pNodeOrg)
{
	if(!pNode) return;
	if(!pNodeOrg) pNodeOrg = pNode;

	std::tuple<bool, double, QPointF> tupNearest =
		get_nearest_node(pNodeOrg->pos(), pNode, m_scene.items(),
			GetScaleFactor(), &GetPowder());

	if(std::get<0>(tupNearest))
		pNode->setPos(std::get<2>(tupNearest));
}

bool ScatteringTriangle::KeepAbsKiKf(double dQx, double dQy)
{
	try
	{
		ublas::vector<double> vecCurKfQ = tl::make_vec({
			m_pNodeKfQ->scenePos().x(),
			m_pNodeKfQ->scenePos().y() });

		ublas::vector<double> vecNewKfQ = tl::make_vec({
			m_pNodeKfQ->scenePos().x() + dQx,
			m_pNodeKfQ->scenePos().y() + dQy });

		ublas::vector<double> vecKiQ = qpoint_to_vec(mapFromItem(m_pNodeKiQ,0,0));
		ublas::vector<double> vecKi = vecKiQ
									- qpoint_to_vec(mapFromItem(m_pNodeKiKf,0,0));

		ublas::vector<double> vecKf = vecCurKfQ
									- qpoint_to_vec(mapFromItem(m_pNodeKiKf,0,0));
		ublas::vector<double> vecNewKf = vecNewKfQ
									- qpoint_to_vec(mapFromItem(m_pNodeKiKf,0,0));

		ublas::vector<double> vecCurQ = vecKiQ - vecCurKfQ;
		ublas::vector<double> vecNewQ = vecKiQ - vecNewKfQ;

		double dKi = ublas::norm_2(vecKi)/m_dScaleFactor;
		double dKf = ublas::norm_2(vecKf)/m_dScaleFactor;
		//double dCurQ = ublas::norm_2(vecCurQ)/m_dScaleFactor;
		double dNewQ = ublas::norm_2(vecNewQ)/m_dScaleFactor;

		//double dCurAngKiQ = tl::get_angle_ki_Q(dKi/tl::angstrom, dKf/tl::angstrom, dCurQ/tl::angstrom) / tl::radians;
		double dAngKiQ = tl::get_angle_ki_Q(dKi/tl::angstrom, dKf/tl::angstrom, dNewQ/tl::angstrom) / tl::radians;

		ublas::matrix<double> matRot = tl::rotation_matrix_2d(-dAngKiQ);
		vecKi = ublas::prod(matRot, vecNewQ) / ublas::norm_2(vecNewQ);
		vecKi *= dKi * m_dScaleFactor;
		ublas::vector<double> vecPtKiKf = vecKiQ - vecKi;

		/*ublas::vector<double> vecKfChk = vecNewKfQ - vecPtKiKf;
		double dKfChk = ublas::norm_2(vecKfChk) / m_dScaleFactor;
		if(!tl::float_equal(dKfChk, dKf))
			return 0;*/

		m_bReady = 0;
		m_pNodeKiKf->setPos(vec_to_qpoint(vecPtKiKf));
		m_bReady = 1;

		return 1;	// allowed
	}
	catch(const std::exception&)
	{
		return 0;	// not allowed
	}
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

	//tl::log_debug("triangle: triangleChanged");
	emit triangleChanged(opts);
}

void ScatteringTriangleScene::emitAllParams()
{
	if(!m_pTri || !m_pTri->IsReady() /*|| m_bDontEmitChange*/) // emit even with m_bDontEmitChange because of Q vector in real space
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
	ublas::vector<double> vec0 = tl::get_column(matPlane, 0);
	ublas::vector<double> vec1 = tl::get_column(matPlane, 1);
	ublas::vector<double> vecUp = tl::get_column(matPlane, 2);

	tl::set_eps_0(vecQ, g_dEpsGfx); tl::set_eps_0(vecQrlu, g_dEpsGfx);
	tl::set_eps_0(vecq, g_dEpsGfx); tl::set_eps_0(vecqrlu, g_dEpsGfx);
	tl::set_eps_0(vecG, g_dEpsGfx); tl::set_eps_0(vecGrlu, g_dEpsGfx);

	/*std::cout << "Q = " << vecQrlu << std::endl;
	std::cout << "q = " << vecqrlu << std::endl;
	std::cout << "G = " << vecGrlu << std::endl;*/

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


	// nearest node (exact G)
	parms.G_rlu_accurate[0] = parms.G_rlu_accurate[1] = parms.G_rlu_accurate[2] = 0.;
	const tl::Kd<double>& kd = m_pTri->GetKdLattice();
	ublas::vector<double> vecHKLinvA = m_pTri->GetRecipLattice().GetPos(-vecQrlu[0], -vecQrlu[1], -vecQrlu[2]);

	if(kd.GetRootNode())
	{
		std::vector<double> stdvecHKL{vecHKLinvA[0], vecHKLinvA[1], vecHKLinvA[2]};
		const std::vector<double>* pvecNearest = &kd.GetNearestNode(stdvecHKL);

		if(pvecNearest)
		{
			parms.G_rlu_accurate[0] = (*pvecNearest)[3];
			parms.G_rlu_accurate[1] = (*pvecNearest)[4];
			parms.G_rlu_accurate[2] = (*pvecNearest)[5];
		}
	}


	CheckForSpurions();

	//tl::log_debug("triangle: emitAllParams");
	emit paramsChanged(parms);
}

// check for elastic spurions
void ScatteringTriangleScene::CheckForSpurions()
{
	typedef units::quantity<units::si::energy> energy;

	const ublas::vector<double> vecq = m_pTri->GetQVecPlane(1);
	const ublas::vector<double> vecKi = m_pTri->GetKiVecPlane();
	const ublas::vector<double> vecKf = m_pTri->GetKfVecPlane();
	energy E = m_pTri->GetE() * tl::one_meV;
	energy Ei = tl::k2E(m_pTri->GetKi()/tl::angstrom);
	energy Ef = tl::k2E(m_pTri->GetKf()/tl::angstrom);

	// elastic ones
	tl::ElasticSpurion spuris = tl::check_elastic_spurion(vecKi, vecKf, vecq);

	// inelastic ones
	std::vector<tl::InelasticSpurion<double>> vecInelCKI = tl::check_inelastic_spurions(1, Ei, Ef, E, 5);
	std::vector<tl::InelasticSpurion<double>> vecInelCKF = tl::check_inelastic_spurions(0, Ei, Ef, E, 5);

	emit spurionInfo(spuris, vecInelCKI, vecInelCKF);
}

void ScatteringTriangleScene::tasChanged(const TriangleOptions& opts)
{
	if(!m_pTri || !m_pTri->IsReady())
		return;

	m_bDontEmitChange = 1;

	if(opts.bChangedMonoTwoTheta)
	{
		m_pTri->SetMonoTwoTheta(opts.dMonoTwoTheta, m_dMonoD);

		//log_info("triag, changed mono: ", opts.dMonoTwoTheta/M_PI*180.);
		//log_info("triag, mono now: ", m_pTri->GetMonoTwoTheta(3.355, 1)/M_PI*180.);
	}
	if(opts.bChangedAnaTwoTheta)
	{
		m_pTri->SetAnaTwoTheta(opts.dAnaTwoTheta, m_dAnaD);
		//log_info("triag, changed ana: ", opts.dAnaTwoTheta/M_PI*180.);
		//log_info("triag, ana now: ", m_pTri->GetAnaTwoTheta(3.355, 1)/M_PI*180.);
	}

	if(opts.bChangedTwoTheta)
	{
		m_pTri->SetTwoTheta(m_bSamplePosSense?opts.dTwoTheta:-opts.dTwoTheta);
		//log_info("triag, changed sample tt: ", opts.dTwoTheta/M_PI*180.);
		//log_info("triag, tt now: ", m_pTri->GetTwoTheta(1)/M_PI*180.);
	}
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
	if(!m_pTri) return;
	m_pTri->SetZoom(dTotalScale);
}

void ScatteringTriangleScene::mousePressEvent(QGraphicsSceneMouseEvent *pEvt)
{
	m_bMousePressed = 1;
	QGraphicsScene::mousePressEvent(pEvt);
}

void ScatteringTriangleScene::setSnapq(bool bSnap)
{
	m_bSnapq = bSnap;

	if(m_bSnapq && m_pTri)
		m_pTri->SnapToNearestPeak(m_pTri->GetNodeGq(), m_pTri->GetNodeKfQ());
}


#ifdef USE_GIL

#define int_p_NULL nullptr
#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/io/png_io.hpp>
namespace gil = boost::gil;

bool ScatteringTriangleScene::ExportBZAccurate(const char* pcFile) const
{
	if(!m_pTri) return false;

	const int iW = 720;
	const int iH = 720;

	std::unique_ptr<gil::rgb8_pixel_t[]> _ptrPix(new gil::rgb8_pixel_t[iH * iW]);
	gil::rgb8_view_t view = gil::interleaved_view(iW, iH, _ptrPix.get(), iW*sizeof(gil::rgb8_pixel_t));

	const int iMaxPeaks = m_pTri->GetMaxPeaks();
	int iXMid = sceneRect().left() + (sceneRect().right()-sceneRect().left())/2;
	int iYMid = sceneRect().top() + (sceneRect().bottom()-sceneRect().top())/2;

	int _iY=0;
	for(int iY=iYMid-iH/2; iY<iYMid+iH/2; ++iY, ++_iY)
	{
		int _iX=0;
		double dY = iY;
		for(int iX=iXMid-iW/2; iX<iXMid+iW/2; ++iX, ++_iX)
		{
			double dX = iX;
			ublas::vector<double> vecHKL = m_pTri->GetHKLFromPlanePos(dX, -dY);
			if(vecHKL.size()!=3) return false;
			vecHKL /= m_pTri->GetScaleFactor();

			const std::vector<double>* pvecNearest = nullptr;
			const tl::Kd<double>& kd = m_pTri->GetKdLattice();
			ublas::vector<double> vecHKLinvA = m_pTri->GetRecipLattice().GetPos(vecHKL[0], vecHKL[1], vecHKL[2]);

			if(kd.GetRootNode())
			{
				std::vector<double> stdvecHKL{vecHKLinvA[0], vecHKLinvA[1], vecHKLinvA[2]};
				pvecNearest = &kd.GetNearestNode(stdvecHKL);
			}

			if(!pvecNearest) return false;
			double dDist = ublas::norm_2(tl::make_vec({(*pvecNearest)[0], (*pvecNearest)[1], (*pvecNearest)[2]}) - vecHKLinvA);

			bool bIsDirectBeam = 0;
			if(tl::float_equal((*pvecNearest)[3], 0.) && tl::float_equal((*pvecNearest)[4], 0.) && tl::float_equal((*pvecNearest)[5], 0.))
				bIsDirectBeam = 1;

			int iR = ((*pvecNearest)[3]+iMaxPeaks) * 255 / (iMaxPeaks*2);
			int iG = ((*pvecNearest)[4]+iMaxPeaks) * 255 / (iMaxPeaks*2);
			int iB = ((*pvecNearest)[5]+iMaxPeaks) * 255 / (iMaxPeaks*2);
			double dBraggAmp = tl::gauss_model(dDist, 0., 0.01, 255., 0.);
			iR += bIsDirectBeam ? -(unsigned int)dBraggAmp : (unsigned int)dBraggAmp;
			iG += bIsDirectBeam ? -(unsigned int)dBraggAmp : (unsigned int)dBraggAmp;
			iB += bIsDirectBeam ? -(unsigned int)dBraggAmp : (unsigned int)dBraggAmp;

			if(iR > 255) iR = 255; if(iG > 255) iG = 255; if(iB > 255) iB = 255;
			if(iR < 0) iR = 0; if(iG < 0) iG = 0; if(iB < 0) iB = 0;

			decltype(view)::x_iterator iterX = view.row_begin(_iY);
			iterX[_iX] = gil::rgb8_pixel_t((unsigned char)iR, (unsigned char)iG, (unsigned char)iB);
		}

		//tl::log_info("BZ export: Line ", _iY+1, " of ", iH);
	}

	try
	{
		gil::png_write_view(pcFile, view);
	}
	catch(const std::ios_base::failure& ex)
	{
		tl::log_err(ex.what());
		return false;
	}
	return true;
}

#else
void ScatteringTriangleScene::ExportBZAccurate(const char* pcFile) const {}
#endif


void ScatteringTriangleScene::drawBackground(QPainter* pPainter, const QRectF& rect)
{
	QGraphicsScene::drawBackground(pPainter, rect);

	// TODO: draw accurate BZ
}

void ScatteringTriangleScene::mouseMoveEvent(QGraphicsSceneMouseEvent *pEvt)
{
	bool bHandled = 0;
	bool bAllowed = 1;


	// tooltip
	if(m_pTri)
	{
		const double dX = pEvt->scenePos().x()/m_pTri->GetScaleFactor();
		const double dY = -pEvt->scenePos().y()/m_pTri->GetScaleFactor();

		ublas::vector<double> vecHKL = m_pTri->GetHKLFromPlanePos(dX, dY);
		tl::set_eps_0(vecHKL, g_dEpsGfx);

		if(vecHKL.size()==3)
		{
			//std::ostringstream ostrPos;
			//ostrPos << "(" << vecHKL[0] << ", " << vecHKL[1] << ", " << vecHKL[2]  << ")";
			//QToolTip::showText(pEvt->screenPos(), ostrPos.str().c_str());

			const std::vector<double>* pvecNearest = nullptr;

			const tl::Kd<double>& kd = m_pTri->GetKdLattice();
			const tl::Lattice<double>& recip = m_pTri->GetRecipLattice();
			ublas::vector<double> vecHKLinvA = recip.GetPos(vecHKL[0], vecHKL[1], vecHKL[2]);

			if(kd.GetRootNode())
			{
				std::vector<double> stdvecHKL{vecHKLinvA[0], vecHKLinvA[1], vecHKLinvA[2]};
				pvecNearest = &kd.GetNearestNode(stdvecHKL);
			}

			emit coordsChanged(vecHKL[0], vecHKL[1], vecHKL[2],
				pvecNearest != nullptr,
				pvecNearest?(*pvecNearest)[3]:0.,
				pvecNearest?(*pvecNearest)[4]:0.,
				pvecNearest?(*pvecNearest)[5]:0.);
		}
	}


	// node dragging
	if(m_bMousePressed)
	{
		QGraphicsItem* pCurItem = mouseGrabberItem();
		if(pCurItem)
		{
			const int iNodeType = pCurItem->data(TRIANGLE_NODE_TYPE_KEY).toInt();

			if(m_bSnap || (m_bSnapq && iNodeType == NODE_q))
			{
				QList<QGraphicsItem*> nodes = items();
				std::tuple<bool, double, QPointF> tupNearest =
					get_nearest_node(pEvt->scenePos(), pCurItem, nodes,
						m_pTri->GetScaleFactor(), &m_pTri->GetPowder());

				if(std::get<0>(tupNearest))
				{
					pCurItem->setPos(std::get<2>(tupNearest));
					bHandled = 1;
				}
			}
			else if(iNodeType == NODE_KIKF && m_bSnapKiKfToElastic)
			{
				std::pair<bool, QPointF> pairNearest =
					get_nearest_elastic_kikf_pos(pEvt->scenePos() /*m_pTri->GetNodeKiKf()->scenePos()*/,
						m_pTri->GetNodeKiQ()->scenePos(),
						m_pTri->GetNodeKfQ()->scenePos());

				if(pairNearest.first)
				{
					pCurItem->setPos(pairNearest.second);
					bHandled = 1;
				}
			}

			// TODO: case for both snapping and abs ki kf fixed
			else if(iNodeType == NODE_Q && m_bKeepAbsKiKf)
			{
				double dX = pEvt->scenePos().x()-pEvt->lastScenePos().x();
				double dY = pEvt->scenePos().y()-pEvt->lastScenePos().y();

				bAllowed = m_pTri->KeepAbsKiKf(dX, dY);
			}
		}
	}

	if(!bHandled && bAllowed)
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
	if(pEvt->key() == Qt::Key_Shift)
		m_bSnapKiKfToElastic = 1;

	QGraphicsScene::keyPressEvent(pEvt);
}

void ScatteringTriangleScene::keyReleaseEvent(QKeyEvent *pEvt)
{
	if(pEvt->key() == Qt::Key_Control)
		m_bSnap = 0;
	if(pEvt->key() == Qt::Key_Shift)
		m_bSnapKiKfToElastic = 0;

	QGraphicsScene::keyReleaseEvent(pEvt);
}


// --------------------------------------------------------------------------------


ScatteringTriangleView::ScatteringTriangleView(QWidget* pParent)
	: QGraphicsView(pParent)
{
	setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
		QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
	setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	setDragMode(QGraphicsView::ScrollHandDrag);
	setMouseTracking(1);
}

ScatteringTriangleView::~ScatteringTriangleView()
{}

void ScatteringTriangleView::wheelEvent(QWheelEvent *pEvt)
{
	this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

	const double dDelta = pEvt->delta()/100.;

#if QT_VER>=5
	double dScale = dDelta>0. ? 1.1 : 1./1.1;
#else
	double dScale = dDelta;
	if(dDelta < 0.)
		dScale = -1./dDelta;
#endif

	this->scale(dScale, dScale);

	m_dTotalScale *= dScale;
	emit scaleChanged(m_dTotalScale);
}


#include "scattering_triangle.moc"
