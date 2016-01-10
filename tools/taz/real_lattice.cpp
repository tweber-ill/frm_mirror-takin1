/*
 * Real crystal lattice
 * @author tweber
 * @date feb-2014
 * @license GPLv2
 */

#include "tlibs/helper/flags.h"
#include "tlibs/math/neutrons.hpp"
#include "tlibs/string/spec_char.h"
#include "tlibs/log/log.h"
#include "helper/globals.h"
#include "real_lattice.h"

#include <QToolTip>
#include <iostream>
#include <sstream>
#include <cmath>
#include <tuple>

namespace units = boost::units;
namespace co = boost::units::si::constants::codata;


LatticePoint::LatticePoint()
{
	setCacheMode(QGraphicsItem::DeviceCoordinateCache);
	//setCursor(Qt::ArrowCursor);
	setFlag(QGraphicsItem::ItemIsMovable, false);
}

QRectF LatticePoint::boundingRect() const
{
	return QRectF(-35., -10., 70., 50.);
}

void LatticePoint::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
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


RealLattice::RealLattice(LatticeScene& scene)
				: m_scene(scene)
{
	this->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	setAcceptedMouseButtons(0);
	m_bReady = 1;
}

RealLattice::~RealLattice()
{
	m_bReady = 0;
	ClearPeaks();
}

QRectF RealLattice::boundingRect() const
{
	return QRectF(-1000.*m_dZoom, -1000.*m_dZoom, 2000.*m_dZoom, 2000.*m_dZoom);
}

void RealLattice::SetZoom(double dZoom)
{
	m_dZoom = dZoom;
	m_scene.update();
}

void RealLattice::SetWSVisible(bool bVisible)
{
	m_bShowWS = bVisible;
	this->update();
}

void RealLattice::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	// Wigner-Seitz cell
	if(m_bShowWS && m_ws.IsValid())
	{
		QPen penOrg = painter->pen();
		painter->setPen(Qt::lightGray);

		const ublas::vector<double>& vecCentral = m_ws.GetCentralReflex() * m_dScaleFactor*m_dZoom;
		//std::cout << vecCentral << std::endl;
		for(const LatticePoint* pPeak : m_vecPeaks)
		{
			QPointF peakPos = pPeak->pos();
			peakPos *= m_dZoom;

			const tl::Brillouin2D<double>::t_vertices<double>& verts = m_ws.GetVertices();
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
}

void RealLattice::CalcPeaks(const tl::Lattice<double>& lattice, const tl::Plane<double>& plane)
{
	ClearPeaks();
	m_kdLattice.Unload();
	m_lattice = lattice;

	const ublas::matrix<double> matA = m_lattice.GetMetric();

	ublas::vector<double> dir0 = plane.GetDir0();
	ublas::vector<double> dir1 = tl::cross_3(plane.GetNorm(), dir0);

	double dDir0Len = ublas::norm_2(dir0);
	double dDir1Len = ublas::norm_2(dir1);

	if(tl::float_equal(dDir0Len, 0.) || tl::float_equal(dDir1Len, 0.)
		|| tl::is_nan_or_inf<double>(dDir0Len) || tl::is_nan_or_inf<double>(dDir1Len))
	{
		tl::log_err("Invalid plane in real lattice.");
		return;
	}


	// -------------------------------------------------------------------------
	// central peak for WS cell calculation
	ublas::vector<int> veciCent = tl::make_vec({0.,0.,0.});


	dir0 /= dDir0Len;
	dir1 /= dDir1Len;

	m_matPlane = tl::column_matrix({dir0, dir1, plane.GetNorm()});
	bool bInv = tl::inverse(m_matPlane, m_matPlane_inv);
	if(!bInv)
	{
		tl::log_err("Cannot invert plane metric in real lattice.");
		return;
	}


	std::list<std::vector<double>> lstPeaksForKd;

	for(int ih=-m_iMaxPeaks; ih<=m_iMaxPeaks; ++ih)
		for(int ik=-m_iMaxPeaks; ik<=m_iMaxPeaks; ++ik)
			for(int il=-m_iMaxPeaks; il<=m_iMaxPeaks; ++il)
			{
				const double h = double(ih);
				const double k = double(ik);
				const double l = double(il);

				const ublas::vector<double> vecPeak = m_lattice.GetPos(h,k,l);

				// add peak in A and in fractional units
				lstPeaksForKd.push_back(std::vector<double>{vecPeak[0],vecPeak[1],vecPeak[2], h,k,l});

				double dDist = 0.;
				ublas::vector<double> vecDropped = plane.GetDroppedPerp(vecPeak, &dDist);

				if(tl::float_equal(dDist, 0., m_dPlaneDistTolerance))
				{
					ublas::vector<double> vecCoord = ublas::prod(m_matPlane_inv, vecDropped);
					double dX = vecCoord[0];
					double dY = -vecCoord[1];

					LatticePoint *pPeak = new LatticePoint();
					if(ih==0 && ik==0 && il==0)
						pPeak->SetColor(Qt::green);
					pPeak->setFlag(QGraphicsItem::ItemIgnoresTransformations);
					pPeak->setPos(dX * m_dScaleFactor, dY * m_dScaleFactor);
					pPeak->setData(LATTICE_NODE_TYPE_KEY, NODE_LATTICE);

					std::ostringstream ostrTip;
					ostrTip << "(" << ih << " " << ik << " " << il << ")";
					pPeak->SetLabel(ostrTip.str().c_str());

					//std::string strAA = ::get_spec_char_utf8("AA")+::get_spec_char_utf8("sup-")+::get_spec_char_utf8("sup1");
					//ostrTip << "\ndistance to plane: " << dDist << " " << strAA;
					pPeak->setToolTip(QString::fromUtf8(ostrTip.str().c_str(), ostrTip.str().length()));

					m_vecPeaks.push_back(pPeak);
					m_scene.addItem(pPeak);


					// Wigner-Seitz cell
					if(ih==veciCent[0] && ik==veciCent[1] && il==veciCent[2])
					{
						ublas::vector<double> vecCentral = tl::make_vec({dX, dY});
						//log_debug("Central ", ih, ik, il, ": ", vecCentral);
						m_ws.SetCentralReflex(vecCentral);
					}
					// TODO: check if 2 next neighbours is sufficient for all space groups
					else if(std::abs(ih-veciCent[0])<=2
							&& std::abs(ik-veciCent[1])<=2
							&& std::abs(il-veciCent[2])<=2)
					{
						ublas::vector<double> vecN = tl::make_vec({dX, dY});
						//log_debug("Reflex: ", vecN);
						m_ws.AddReflex(vecN);
					}
				}
			}

	m_ws.CalcBZ();
	//for(LatticePoint* pPeak : m_vecPeaks)
	//	pPeak->SetBZ(&m_ws);

	m_kdLattice.Load(lstPeaksForKd, 3);

	this->update();
}

ublas::vector<double> RealLattice::GetHKLFromPlanePos(double x, double y) const
{
	if(!HasPeaks())
		return ublas::vector<double>();

	ublas::vector<double> vec = x*tl::get_column(m_matPlane, 0)
		+ y*tl::get_column(m_matPlane, 1);
	return m_lattice.GetHKL(vec);
}

void RealLattice::ClearPeaks()
{
	m_ws.Clear();

	for(LatticePoint*& pPeak : m_vecPeaks)
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


LatticeScene::LatticeScene() : m_pLatt(new RealLattice(*this))
{
	this->addItem(m_pLatt);
}

LatticeScene::~LatticeScene()
{
	delete m_pLatt;
}

void LatticeScene::scaleChanged(double dTotalScale)
{
	if(!m_pLatt) return;
	m_pLatt->SetZoom(dTotalScale);
}

void LatticeScene::mousePressEvent(QGraphicsSceneMouseEvent *pEvt)
{
	m_bMousePressed = 1;
	QGraphicsScene::mousePressEvent(pEvt);
}


#ifdef USE_GIL

#include "tlibs/gfx/gil.h"
namespace gil = boost::gil;

bool LatticeScene::ExportWSAccurate(const char* pcFile) const
{
	if(!m_pLatt) return false;

	const int iW = 720;
	const int iH = 720;

	gil::rgb8_view_t view;
	std::vector<gil::rgb8_pixel_t> vecPix;
	tl::create_imgview(iW, iH, vecPix, view);

	const int iMaxPeaks = m_pLatt->GetMaxPeaks();
	int iXMid = sceneRect().left() + (sceneRect().right()-sceneRect().left())/2;
	int iYMid = sceneRect().top() + (sceneRect().bottom()-sceneRect().top())/2;

	int _iY=0;
	for(int iY=iYMid-iH/2; iY<iYMid+iH/2; ++iY, ++_iY)
	{
		auto iterRow = view.row_begin(_iY);

		int _iX=0;
		double dY = iY;
		for(int iX=iXMid-iW/2; iX<iXMid+iW/2; ++iX, ++_iX)
		{
			double dX = iX;
			ublas::vector<double> vecHKL = m_pLatt->GetHKLFromPlanePos(dX, -dY);
			if(vecHKL.size()!=3) return false;
			vecHKL /= m_pLatt->GetScaleFactor();

			const std::vector<double>* pvecNearest = nullptr;
			const tl::Kd<double>& kd = m_pLatt->GetKdLattice();
			ublas::vector<double> vecHKLA = m_pLatt->GetRealLattice().GetPos(vecHKL[0], vecHKL[1], vecHKL[2]);

			if(kd.GetRootNode())
			{
				std::vector<double> stdvecHKL{vecHKLA[0], vecHKLA[1], vecHKLA[2]};
				pvecNearest = &kd.GetNearestNode(stdvecHKL);
			}

			if(!pvecNearest) return false;
			double dDist = ublas::norm_2(tl::make_vec({(*pvecNearest)[0], (*pvecNearest)[1], (*pvecNearest)[2]}) - vecHKLA);

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

			iR = tl::clamp(iR, 0, 255);
			iG = tl::clamp(iG, 0, 255);
			iB = tl::clamp(iB, 0, 255);

			iterRow[_iX] = gil::rgb8_pixel_t((unsigned char)iR, (unsigned char)iG, (unsigned char)iB);
		}

		//tl::log_info("BZ export: Line ", _iY+1, " of ", iH);
	}

	if(!tl::save_view(pcFile, &view))
	{
		tl::log_err("Cannot write image \"", pcFile, "\".");
		return false;
	}

	return true;
}

#else
void LatticeScene::ExportWSAccurate(const char* pcFile) const {}
#endif


void LatticeScene::drawBackground(QPainter* pPainter, const QRectF& rect)
{
	QGraphicsScene::drawBackground(pPainter, rect);
	// TODO: draw accurate WS cell
}

void LatticeScene::mouseMoveEvent(QGraphicsSceneMouseEvent *pEvt)
{
	bool bHandled = 0;
	bool bAllowed = 1;


	// tooltip
	if(m_pLatt)
	{
		const double dX = pEvt->scenePos().x()/m_pLatt->GetScaleFactor();
		const double dY = -pEvt->scenePos().y()/m_pLatt->GetScaleFactor();

		ublas::vector<double> vecHKL = m_pLatt->GetHKLFromPlanePos(dX, dY);
		tl::set_eps_0(vecHKL, g_dEps);

		if(vecHKL.size()==3)
		{
			//std::ostringstream ostrPos;
			//ostrPos << "(" << vecHKL[0] << ", " << vecHKL[1] << ", " << vecHKL[2]  << ")";
			//QToolTip::showText(pEvt->screenPos(), ostrPos.str().c_str());

			const std::vector<double>* pvecNearest = nullptr;

			const tl::Kd<double>& kd = m_pLatt->GetKdLattice();
			const tl::Lattice<double>& lattice = m_pLatt->GetRealLattice();
			ublas::vector<double> vecHKLA = lattice.GetPos(vecHKL[0], vecHKL[1], vecHKL[2]);

			if(kd.GetRootNode())
			{
				std::vector<double> stdvecHKL{vecHKLA[0], vecHKLA[1], vecHKLA[2]};
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
			const int iNodeType = pCurItem->data(LATTICE_NODE_TYPE_KEY).toInt();

			// nothing there yet...
		}
	}

	if(!bHandled && bAllowed)
		QGraphicsScene::mouseMoveEvent(pEvt);
}

void LatticeScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *pEvt)
{
	m_bMousePressed = 0;

	QGraphicsScene::mouseReleaseEvent(pEvt);
}

void LatticeScene::keyPressEvent(QKeyEvent *pEvt)
{
	if(pEvt->key() == Qt::Key_Control)
		m_bSnap = 1;

	QGraphicsScene::keyPressEvent(pEvt);
}

void LatticeScene::keyReleaseEvent(QKeyEvent *pEvt)
{
	if(pEvt->key() == Qt::Key_Control)
		m_bSnap = 0;

	QGraphicsScene::keyReleaseEvent(pEvt);
}


// --------------------------------------------------------------------------------


LatticeView::LatticeView(QWidget* pParent)
	: QGraphicsView(pParent)
{
	setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
		QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
	setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	setDragMode(QGraphicsView::ScrollHandDrag);
	setMouseTracking(1);
}

LatticeView::~LatticeView()
{}

void LatticeView::wheelEvent(QWheelEvent *pEvt)
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


#include "real_lattice.moc"
