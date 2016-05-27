/**
 * Scattering Triangle
 * @author tweber
 * @date feb-2014
 * @license GPLv2
 */

#ifndef __TAZ_SCATT_TRIAG_H__
#define __TAZ_SCATT_TRIAG_H__

#include "tlibs/math/linalg.h"
#include "tlibs/math/lattice.h"
#include "tlibs/math/powder.h"
#include "tlibs/math/bz.h"
#include "tlibs/math/neutrons.hpp"
#include "tlibs/math/kd.h"

#include "libs/globals.h"
#include "libs/globals_qt.h"
#include "libs/spacegroups/spacegroup.h"
#include "libs/formfactors/formfact.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsTextItem>
#include <QWheelEvent>
#if QT_VER>=5
	#include <QtWidgets>
#endif

#include "tasoptions.h"
#include "dialogs/RecipParamDlg.h"	// for RecipParams struct
#include "dialogs/AtomsDlg.h"


template<class t_real = t_real_glob>
struct RecipCommon
{
	using t_mat = ublas::matrix<t_real>;
	using t_vec = ublas::vector<t_real>;

	tl::Lattice<t_real> lattice, recip;
	tl::Plane<t_real> planeRLU;

	const SpaceGroup* pSpaceGroup = nullptr;
	const std::vector<AtomPos>* pvecAtomPos = nullptr;

	t_mat matPlane, matPlane_inv;

	std::vector<std::string> vecAllNames;
	std::vector<t_vec> vecAllAtoms, vecAllAtomsFrac;
	std::vector<std::size_t> vecAllAtomTypes;
	std::vector<std::complex<t_real>> vecScatlens;

	t_vec dir0RLU, dir1RLU;
	t_mat matA, matB;

	tl::Plane<t_real> plane;


	RecipCommon() = default;
	~RecipCommon() = default;

	bool Calc(const tl::Lattice<t_real>& lat, const tl::Lattice<t_real>& rec,
		const tl::Plane<t_real>& plRLU, const SpaceGroup *pSG,
		const std::vector<AtomPos>* pvecAt)
	{
		lattice = lat;
		recip = rec;
		planeRLU = plRLU;
		pSpaceGroup = pSG;
		pvecAtomPos = pvecAt;

		t_vec vecX0 = ublas::zero_vector<t_real>(3);
		t_vec vecPlaneX = planeRLU.GetDir0()[0] * recip.GetVec(0) +
			planeRLU.GetDir0()[1] * recip.GetVec(1) +
			planeRLU.GetDir0()[2] * recip.GetVec(2);
		t_vec vecPlaneY = planeRLU.GetDir1()[0] * recip.GetVec(0) +
			planeRLU.GetDir1()[1] * recip.GetVec(1) +
			planeRLU.GetDir1()[2] * recip.GetVec(2);
		plane = tl::Plane<t_real>(vecX0, vecPlaneX, vecPlaneY);

		matA = lattice.GetMetric();
		matB = recip.GetMetric();

		dir0RLU = planeRLU.GetDir0();
		dir1RLU = planeRLU.GetDir1();

		std::vector<t_vec> vecOrth =
			tl::gram_schmidt<t_vec>(
				{plane.GetDir0(), plane.GetDir1(), plane.GetNorm()}, 1);
		matPlane = tl::column_matrix(vecOrth);
		t_real dDir0Len = ublas::norm_2(vecOrth[0]), dDir1Len = ublas::norm_2(vecOrth[1]);

		if(tl::float_equal<t_real>(dDir0Len, 0.) || tl::float_equal<t_real>(dDir1Len, 0.)
			|| tl::is_nan_or_inf<t_real>(dDir0Len) || tl::is_nan_or_inf<t_real>(dDir1Len))
		{
			tl::log_err("Invalid scattering plane.");
			return false;
		}

		bool bInv = tl::inverse(matPlane, matPlane_inv);
		if(!bInv)
		{
			tl::log_err("Cannot invert scattering plane metric.");
			return false;
		}


		// --------------------------------------------------------------------
		// structure factors
		ScatlenList lstsl;
		//FormfactList lstff;

		const std::vector<t_mat>* pvecSymTrafos = nullptr;
		if(pSpaceGroup)
			pvecSymTrafos = &pSpaceGroup->GetTrafos();

		if(pvecSymTrafos && pvecSymTrafos->size() && /*g_bHasFormfacts &&*/
			g_bHasScatlens && pvecAtomPos && pvecAtomPos->size())
		{
			std::vector<t_vec> vecAtoms;
			std::vector<std::string> vecNames;
			for(std::size_t iAtom=0; iAtom<pvecAtomPos->size(); ++iAtom)
			{
				vecAtoms.push_back((*pvecAtomPos)[iAtom].vecPos);
				vecNames.push_back((*pvecAtomPos)[iAtom].strAtomName);
			}

			std::tie(vecAllNames, vecAllAtoms, vecAllAtomsFrac, vecAllAtomTypes) =
			tl::generate_all_atoms<t_mat, t_vec, std::vector>
				(*pvecSymTrafos, vecAtoms, &vecNames, matA,
				t_real(0), t_real(1), g_dEps);

			for(const std::string& strElem : vecAllNames)
			{
				const ScatlenList::elem_type* pElem = lstsl.Find(strElem);
				vecScatlens.push_back(pElem ? pElem->GetCoherent() : std::complex<t_real>(0.,0.));
				if(!pElem)
					tl::log_err("Element \"", strElem, "\" not found in scattering length table.",
						" Using b=0.");
			}
		}
		// --------------------------------------------------------------------

		return true;
	}


	bool CanCalcStructFact() const { return vecScatlens.size() != 0; }

	std::tuple<std::complex<t_real>, t_real, t_real> GetStructFact(const t_vec& vecPeak) const
	{
		std::complex<t_real> cF =
			tl::structfact<t_real, std::complex<t_real>, t_vec, std::vector>
				(vecAllAtoms, vecPeak, vecScatlens);
		t_real dFsq = (std::conj(cF)*cF).real();
		t_real dF = std::sqrt(dFsq);

		return std::make_tuple(cF, dF, dFsq);
	}
};


#define TRIANGLE_NODE_TYPE_KEY	0

enum ScatteringTriangleNodeType
{
	NODE_Q,
	NODE_q,

	NODE_BRAGG,
	NODE_KIQ,
	NODE_KIKF,

	NODE_OTHER
};

class ScatteringTriangle;
class ScatteringTriangleNode : public QGraphicsItem
{
	protected:
		ScatteringTriangle *m_pParentItem;

	protected:
		virtual QRectF boundingRect() const override;
		virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;

		virtual void mousePressEvent(QGraphicsSceneMouseEvent *pEvt) override;
		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *pEvt) override;
		virtual QVariant itemChange(GraphicsItemChange change, const QVariant &val) override;

	public:
		ScatteringTriangleNode(ScatteringTriangle* pSupItem);
};

class RecipPeak : public QGraphicsItem
{
	protected:
		QColor m_color = Qt::red;
		QString m_strLabel;
		//const Brillouin2D<t_real_glob>* m_pBZ = 0;
		t_real_glob m_dRadius = 3.;

	protected:
		virtual QRectF boundingRect() const override;
		virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;

	public:
		RecipPeak();

		void SetLabel(const QString& str) { m_strLabel = str; }
		void SetColor(const QColor& col) { m_color = col; }

		void SetRadius(t_real_glob dRad) { m_dRadius = dRad; }
		t_real_glob GetRadius() const { return m_dRadius; }
		//void SetBZ(const Brillouin2D<t_real_glob>* pBZ) { this->m_pBZ = pBZ; }
};

class ScatteringTriangleScene;
class ScatteringTriangle : public QGraphicsItem
{
	protected:
		bool m_bReady = 0;

		ScatteringTriangleScene &m_scene;

		ScatteringTriangleNode *m_pNodeKiQ = 0;
		ScatteringTriangleNode *m_pNodeKiKf = 0;
		ScatteringTriangleNode *m_pNodeKfQ = 0;
		ScatteringTriangleNode *m_pNodeGq = 0;

		t_real_glob m_dScaleFactor = 150.;	// pixels per A^-1 for zoom == 1.
		t_real_glob m_dZoom = 1.;
		t_real_glob m_dPlaneDistTolerance = 0.01;
		int m_iMaxPeaks = 7;

		tl::Lattice<t_real_glob> m_lattice, m_recip;
		ublas::matrix<t_real_glob> m_matPlane, m_matPlane_inv;
		std::vector<RecipPeak*> m_vecPeaks;

		tl::Powder<int, t_real_glob> m_powder;
		tl::Kd<t_real_glob> m_kdLattice;

		bool m_bShowBZ = 1;
		tl::Brillouin2D<t_real_glob> m_bz;

		//tl::Lattice<t_real_glob> m_recip_unrot;
		//t_real_glob m_dAngleRot = 0.;

		bool m_bqVisible = 0;
		bool m_bShowEwaldSphere = 1;

	protected:
		virtual QRectF boundingRect() const override;

	public:
		ScatteringTriangle(ScatteringTriangleScene& scene);
		virtual ~ScatteringTriangle();

		virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget *) override;

		void SetReady(bool bReady) { m_bReady = bReady; }
		void nodeMoved(const ScatteringTriangleNode* pNode=0);

		const ublas::matrix<t_real_glob>& GetPlane() const { return m_matPlane; }

		bool IsReady() const { return m_bReady; }
		t_real_glob GetTheta(bool bPosSense) const;
		t_real_glob GetTwoTheta(bool bPosSense) const;
		t_real_glob GetMonoTwoTheta(t_real_glob dMonoD, bool bPosSense) const;
		t_real_glob GetAnaTwoTheta(t_real_glob dAnaD, bool bPosSense) const;

		t_real_glob GetKi() const;
		t_real_glob GetKf() const;
		t_real_glob GetE() const;
		t_real_glob GetQ() const;
		t_real_glob Getq() const;

		t_real_glob GetAngleKiQ(bool bSense) const;
		t_real_glob GetAngleKfQ(bool bSense) const;
		t_real_glob GetAngleQVec0() const;

		void SetTwoTheta(t_real_glob dTT);
		void SetAnaTwoTheta(t_real_glob dTT, t_real_glob dAnaD);
		void SetMonoTwoTheta(t_real_glob dTT, t_real_glob dMonoD);

	public:
		bool HasPeaks() const { return m_vecPeaks.size()!=0 && m_recip.IsInited(); }
		void ClearPeaks();
		void CalcPeaks(const RecipCommon<t_real_glob>& recipcommon, bool bIsPowder=0);

		void SetPlaneDistTolerance(t_real_glob dTol) { m_dPlaneDistTolerance = dTol; }
		void SetMaxPeaks(int iMax) { m_iMaxPeaks = iMax; }
		unsigned int GetMaxPeaks() const { return m_iMaxPeaks; }
		void SetZoom(t_real_glob dZoom);
		t_real_glob GetZoom() const { return m_dZoom; }

		void SetqVisible(bool bVisible);
		void SetBZVisible(bool bVisible);
		void SetEwaldSphereVisible(bool bVisible);

		const tl::Powder<int,t_real_glob>& GetPowder() const { return m_powder; }
		const tl::Kd<t_real_glob>& GetKdLattice() const { return m_kdLattice; }

	public:
		std::vector<ScatteringTriangleNode*> GetNodes();
		std::vector<std::string> GetNodeNames() const;

		t_real_glob GetScaleFactor() const { return m_dScaleFactor; }
		void SetScaleFactor(t_real_glob dScale) { m_dScaleFactor = dScale; }

		ScatteringTriangleNode* GetNodeGq() { return m_pNodeGq; }
		ScatteringTriangleNode* GetNodeKiQ() { return m_pNodeKiQ; }
		ScatteringTriangleNode* GetNodeKfQ() { return m_pNodeKfQ; }
		ScatteringTriangleNode* GetNodeKiKf() { return m_pNodeKiKf; }

		ublas::vector<t_real_glob> GetHKLFromPlanePos(t_real_glob x, t_real_glob y) const;
		ublas::vector<t_real_glob> GetQVec(bool bSmallQ=0, bool bRLU=1) const;	// careful: check sign

		ublas::vector<t_real_glob> GetQVecPlane(bool bSmallQ=0) const;
		ublas::vector<t_real_glob> GetKiVecPlane() const;
		ublas::vector<t_real_glob> GetKfVecPlane() const;

		void RotateKiVec0To(bool bSense, t_real_glob dAngle);
		void SnapToNearestPeak(ScatteringTriangleNode* pNode,
						const ScatteringTriangleNode* pNodeOrg=0);
		bool KeepAbsKiKf(t_real_glob dQx, t_real_glob dQy);

		const tl::Lattice<t_real_glob>& GetRecipLattice() const { return m_recip; }
		QPointF GetGfxMid() const;

		void AllowMouseMove(bool bAllow);
};


class ScatteringTriangleScene : public QGraphicsScene
{	Q_OBJECT
	protected:
		ScatteringTriangle *m_pTri;
		t_real_glob m_dMonoD = 3.355;
		t_real_glob m_dAnaD = 3.355;

		bool m_bSamplePosSense = 1;
		bool m_bAnaPosSense = 0;
		bool m_bMonoPosSense = 0;

		bool m_bDontEmitChange = 0;
		bool m_bSnap = 0;
		bool m_bSnapq = 1;
		bool m_bMousePressed = 0;

		bool m_bKeepAbsKiKf = 1;
		bool m_bSnapKiKfToElastic = 0;

	public:
		ScatteringTriangleScene();
		virtual ~ScatteringTriangleScene();

		void SetEmitChanges(bool bEmit) { m_bDontEmitChange = !bEmit; }
		// emits triangleChanged
		void emitUpdate();
		// emits paramsChanged
		void emitAllParams();
		void SetDs(t_real_glob dMonoD, t_real_glob dAnaD);

		void SetSampleSense(bool bPos);
		void SetMonoSense(bool bPos);
		void SetAnaSense(bool bPos);

		const ScatteringTriangle* GetTriangle() const { return m_pTri; }
		ScatteringTriangle* GetTriangle() { return m_pTri; }

		void CheckForSpurions();

		bool ExportBZAccurate(const char* pcFile) const;

	public slots:
		void tasChanged(const TriangleOptions& opts);
		void scaleChanged(t_real_glob dTotalScale);

		void setSnapq(bool bSnap);
		bool getSnapq() const { return m_bSnapq; }

		void setKeepAbsKiKf(bool bKeep) { m_bKeepAbsKiKf = bKeep; }
		bool getKeepAbsKiKf() const { return m_bKeepAbsKiKf; }

	signals:
		// relevant parameters for instrument view
		void triangleChanged(const TriangleOptions& opts);
		// all parameters
		void paramsChanged(const RecipParams& parms);

		void spurionInfo(const tl::ElasticSpurion& spuris,
			const std::vector<tl::InelasticSpurion<t_real_glob>>& vecInelCKI,
			const std::vector<tl::InelasticSpurion<t_real_glob>>& vecInelCKF);

		void coordsChanged(t_real_glob dh, t_real_glob dk, t_real_glob dl,
			bool bHasNearest,
			t_real_glob dNearestH, t_real_glob dNearestK, t_real_glob dNearestL);

		void nodeEvent(bool bStarted);

	protected:
		virtual void mousePressEvent(QGraphicsSceneMouseEvent *pEvt) override;
		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *pEvt) override;
		virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *pEvt) override;

		virtual void keyPressEvent(QKeyEvent *pEvt) override;
		virtual void keyReleaseEvent(QKeyEvent *pEvt) override;

		virtual void drawBackground(QPainter*, const QRectF&) override;
};


class ScatteringTriangleView : public QGraphicsView
{
	Q_OBJECT
	protected:
		t_real_glob m_dTotalScale = 1.;
		virtual void wheelEvent(QWheelEvent* pEvt) override;

	public:
		ScatteringTriangleView(QWidget* pParent = 0);
		virtual ~ScatteringTriangleView();

	signals:
		void scaleChanged(t_real_glob dTotalScale);
};

#endif
