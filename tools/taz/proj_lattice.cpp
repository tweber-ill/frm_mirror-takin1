/**
 * Crystal lattice projections
 * @author tweber
 * @date may-2016
 * @license GPLv2
 */

#include "tlibs/helper/flags.h"
#include "tlibs/math/neutrons.hpp"
#include "tlibs/string/spec_char.h"
#include "tlibs/log/log.h"
#include "libs/globals.h"
#include "proj_lattice.h"

#include <QToolTip>
#include <iostream>
#include <sstream>
#include <cmath>
#include <tuple>


using t_real = t_real_glob;
using t_vec = ublas::vector<t_real>;
using t_mat = ublas::matrix<t_real>;


// symbol drawing sizes
#define DEF_PEAK_SIZE 3.
#define MIN_PEAK_SIZE 0.5
#define MAX_PEAK_SIZE 5.5


#define PROJ_LATTICE_NODE_TYPE_KEY	0

enum ProjLatticeNodeType
{
	NODE_PROJ_LATTICE,
	NODE_PROJ_LATTICE_OTHER
};



// --------------------------------------------------------------------------------

ProjLatticePoint::ProjLatticePoint()
{
	//setCursor(Qt::ArrowCursor);
	setFlag(QGraphicsItem::ItemIsMovable, false);
	setFlag(QGraphicsItem::ItemIgnoresTransformations);
}

QRectF ProjLatticePoint::boundingRect() const
{
	return QRectF(-5., -5., 10., 10.);
}

void ProjLatticePoint::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	painter->setFont(g_fontGfx);
	painter->setBrush(m_color);
	painter->drawEllipse(QRectF(-m_dRadius, -m_dRadius, m_dRadius*2., m_dRadius*2.));

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


ProjLattice::ProjLattice(ProjLatticeScene& scene)
	: m_scene(scene)
{
	setFlag(QGraphicsItem::ItemIgnoresTransformations);
	setAcceptedMouseButtons(0);
	m_bReady = 1;
}

ProjLattice::~ProjLattice()
{
	m_bReady = 0;
	ClearPeaks();
}

QRectF ProjLattice::boundingRect() const
{
	return QRectF(-1000.*m_dZoom, -1000.*m_dZoom, 2000.*m_dZoom, 2000.*m_dZoom);
}

void ProjLattice::SetZoom(t_real dZoom)
{
	m_dZoom = dZoom;
	m_scene.update();
}

void ProjLattice::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	painter->setFont(g_fontGfx);
}

template<class T = double>
void to_gnomonic(T x, T y, T z, T& xg, T& yg)
{
	T rho, phi, theta, phi_crys, theta_crys;
	std::tie(rho, phi, theta) = tl::cart_to_sph(x, y, z);
	std::tie(phi_crys, theta_crys) = tl::sph_to_crys(phi, theta);
	std::tie(xg, yg) = tl::gnomonic_proj(phi_crys, theta_crys);
}

void ProjLattice::CalcPeaks(const RecipCommon<t_real>& recipcommon, bool bIsRecip)
{
	ClearPeaks();

	if(bIsRecip)
	{
		m_lattice = recipcommon.recip;
		m_matPlane_inv = recipcommon.matPlane_inv;
	}
	else
	{
		m_lattice = recipcommon.lattice;
		// m_matPlane_inv = ! TODO !
	}


	const std::string strAA = tl::get_spec_char_utf8("AA");
	bool bModifiedRadii = 0;

	for(int ih=-m_iMaxPeaks; ih<=m_iMaxPeaks; ++ih)
		for(int ik=-m_iMaxPeaks; ik<=m_iMaxPeaks; ++ik)
			for(int il=-m_iMaxPeaks; il<=m_iMaxPeaks; ++il)
			{
				if(bIsRecip && recipcommon.pSpaceGroup &&
					!recipcommon.pSpaceGroup->HasReflection(ih, ik, il))
					continue;

				const t_real h = t_real(ih), k = t_real(ik), l = t_real(il);
				t_vec vecPeak = m_lattice.GetPos(h,k,l);

				t_vec vecCoord = ublas::prod(m_matPlane_inv, vecPeak);
				t_real dX = vecCoord[0], dY = vecCoord[1];
				to_gnomonic(vecCoord[0], vecCoord[2], vecCoord[1], dX, dY);
				dX *=m_dScaleFactor; dY = -dY*m_dScaleFactor;

				if(tl::is_nan_or_inf(dX) || tl::is_nan_or_inf(dY))
					continue;

				auto iterPeak = std::find_if(m_vecPeaks.begin(), m_vecPeaks.end(),
					[dX, dY](const ProjLatticePoint* pPeak) -> bool
					{
						return tl::float_equal(pPeak->x(), dX, g_dEpsGfx) &&
							tl::float_equal(pPeak->y(), dY, g_dEpsGfx);
					});

				ProjLatticePoint *pPeak = nullptr;
				if(iterPeak == m_vecPeaks.end())
				{
					pPeak = new ProjLatticePoint();
					if(ih==0 && ik==0 && il==0)
						pPeak->SetColor(Qt::green);
					pPeak->setPos(dX, dY);
					pPeak->setData(PROJ_LATTICE_NODE_TYPE_KEY, NODE_PROJ_LATTICE);

					m_vecPeaks.push_back(pPeak);
					m_scene.addItem(pPeak);
				}
				else
				{
					pPeak = *iterPeak;
				}

				if(bIsRecip && recipcommon.CanCalcStructFact())
				{
					t_real dF = 0.;
					std::tie(std::ignore, dF, std::ignore) =
						recipcommon.GetStructFact(vecPeak);

					pPeak->AddRadius(dF);
					bModifiedRadii = 1;
				}
				else
				{
					pPeak->SetRadius(DEF_PEAK_SIZE);
				}

				std::ostringstream ostrTip;
				ostrTip.precision(g_iPrecGfx);

				ostrTip << "(" << ih << " " << ik << " " << il << ")";
				pPeak->AddTooltip(QString::fromUtf8(ostrTip.str().c_str(), ostrTip.str().length()));
			}


	t_real dMinRad = std::numeric_limits<t_real>::max(), dMaxRad = -1.;

	if(bModifiedRadii)
	{
		for(ProjLatticePoint* pPeak : m_vecPeaks)
		{
			dMinRad = std::min(dMinRad, pPeak->GetRadius());
			dMaxRad = std::max(dMaxRad, pPeak->GetRadius());
		}
	}

	for(ProjLatticePoint* pPeak : m_vecPeaks)
	{
		if(bModifiedRadii)
		{
			if(!tl::float_equal(dMinRad, dMaxRad, g_dEpsGfx))
			{
				t_real dRadScale = (pPeak->GetRadius()-dMinRad) / (dMaxRad-dMinRad);
				pPeak->SetRadius(tl::lerp(MIN_PEAK_SIZE, MAX_PEAK_SIZE, dRadScale));
			}
			else
			{
				pPeak->SetRadius(DEF_PEAK_SIZE);
			}
		}

		pPeak->SetTooltip();
	}

	this->update();
}

void ProjLattice::ClearPeaks()
{
	for(ProjLatticePoint*& pPeak : m_vecPeaks)
	{
		if(pPeak)
		{
			m_scene.removeItem(pPeak);
			delete pPeak;
			pPeak = nullptr;
		}
	}
	m_vecPeaks.clear();
}

// --------------------------------------------------------------------------------


ProjLatticeScene::ProjLatticeScene() : m_pLatt(new ProjLattice(*this))
{
	this->addItem(m_pLatt);
}

ProjLatticeScene::~ProjLatticeScene()
{
	delete m_pLatt;
}

void ProjLatticeScene::scaleChanged(t_real dTotalScale)
{
	if(!m_pLatt) return;
	m_pLatt->SetZoom(dTotalScale);
}

void ProjLatticeScene::mousePressEvent(QGraphicsSceneMouseEvent *pEvt)
{
	m_bMousePressed = 1;
	QGraphicsScene::mousePressEvent(pEvt);
}


void ProjLatticeScene::drawBackground(QPainter* pPainter, const QRectF& rect)
{
	QGraphicsScene::drawBackground(pPainter, rect);
}

void ProjLatticeScene::mouseMoveEvent(QGraphicsSceneMouseEvent *pEvt)
{
	bool bHandled = 0;
	bool bAllowed = 1;

/*
	// TODO: inverse trafo
	if(m_pLatt)
	{
		const t_real dX = pEvt->scenePos().x()/m_pLatt->GetScaleFactor();
		const t_real dY = -pEvt->scenePos().y()/m_pLatt->GetScaleFactor();

		t_vec vecHKL = ...;
		tl::set_eps_0(vecHKL, g_dEps);

		if(vecHKL.size()==3)
		{
			emit coordsChanged(vecHKL[0], vecHKL[1], vecHKL[2], 0, 0., 0., 0.);
		}
	}
*/

	// node dragging
	if(m_bMousePressed)
	{
		QGraphicsItem* pCurItem = mouseGrabberItem();
		if(pCurItem)
		{
			const int iNodeType = pCurItem->data(PROJ_LATTICE_NODE_TYPE_KEY).toInt();

			// nothing there yet...
		}
	}

	if(!bHandled && bAllowed)
		QGraphicsScene::mouseMoveEvent(pEvt);
}

void ProjLatticeScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *pEvt)
{
	m_bMousePressed = 0;

	QGraphicsScene::mouseReleaseEvent(pEvt);
}

void ProjLatticeScene::keyPressEvent(QKeyEvent *pEvt)
{
	if(pEvt->key() == Qt::Key_Control)
		m_bSnap = 1;

	QGraphicsScene::keyPressEvent(pEvt);
}

void ProjLatticeScene::keyReleaseEvent(QKeyEvent *pEvt)
{
	if(pEvt->key() == Qt::Key_Control)
		m_bSnap = 0;

	QGraphicsScene::keyReleaseEvent(pEvt);
}


// --------------------------------------------------------------------------------


ProjLatticeView::ProjLatticeView(QWidget* pParent)
	: QGraphicsView(pParent)
{
	setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
		QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
	setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	setDragMode(QGraphicsView::ScrollHandDrag);
	setMouseTracking(1);
	setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}

ProjLatticeView::~ProjLatticeView()
{}

void ProjLatticeView::wheelEvent(QWheelEvent *pEvt)
{
#if QT_VER>=5
	const t_real dDelta = pEvt->angleDelta().y()/8. / 150.;
#else
	const t_real dDelta = pEvt->delta()/8. / 150.;
#endif

	t_real dScale = std::pow(2., dDelta);
	this->scale(dScale, dScale);
	m_dTotalScale *= dScale;
	emit scaleChanged(m_dTotalScale);
}


#include "proj_lattice.moc"
