/*
 * TAS layout
 * @author tweber
 * @date feb-2014
 * @copyright GPLv2
 */

#ifndef __TAS_LAYOUT_H__
#define __TAS_LAYOUT_H__

#include "tlibs/helper/flags.h"
#include <cmath>

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
#include "dialogs/RealParamDlg.h"	// for RealParams struct
#include "dialogs/RecipParamDlg.h"	// for RecipParams struct

class TasLayout;
class TasLayoutNode : public QGraphicsItem
{
	protected:
		TasLayout *m_pParentItem;

	protected:
		QRectF boundingRect() const;
		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

		QVariant itemChange(GraphicsItemChange change, const QVariant &value);

	public:
		TasLayoutNode(TasLayout* pSupItem);
};


class TasLayoutScene;
class TasLayout : public QGraphicsItem
{
	protected:
		bool m_bReady = 0;

		TasLayoutScene& m_scene;

		TasLayoutNode *m_pSrc = 0;
		TasLayoutNode *m_pMono = 0;
		TasLayoutNode *m_pSample = 0;
		TasLayoutNode *m_pAna = 0;
		TasLayoutNode *m_pDet = 0;

		double m_dMonoTwoTheta = 3.1415/2.;
		double m_dAnaTwoTheta = 3.1415/2.;
		double m_dTwoTheta = 3.1415/2.;
		double m_dTheta = 3.1415/4.;
		double m_dAngleKiQ = 3.1415/4.;

		double m_dLenMonoSample = 150.;
		double m_dLenSampleAna = 100.;
		double m_dLenAnaDet = 50.;
		double m_dLenSample = 25.;

		double m_dScaleFactor = 1.25; // pixels per cm for zoom == 1
		double m_dZoom = 1.;

		bool m_bRealQVisible = 1;
		bool m_bAllowChanges = 1;

	public:
		double GetMonoTwoTheta() const { return m_dMonoTwoTheta; }
		double GetMonoTheta() const { return m_dMonoTwoTheta/2.; }
		double GetAnaTwoTheta() const { return m_dAnaTwoTheta; }
		double GetAnaTheta() const { return m_dAnaTwoTheta/2.; }
		double GetSampleTwoTheta() const { return m_dTwoTheta; }
		double GetSampleTheta() const { return m_dTheta; }
		double GetLenMonoSample() const { return m_dLenMonoSample; }
		double GetLenSampleAna() const { return m_dLenSampleAna; }
		double GetLenAnaDet() const { return m_dLenAnaDet; }

	protected:
		QRectF boundingRect() const;

	public:
		TasLayout(TasLayoutScene& scene);
		virtual ~TasLayout();

		bool IsReady() const { return m_bReady; }

		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

		void SetSampleTwoTheta(double dTT);
		void SetSampleTheta(double dT);
		void SetMonoTwoTheta(double dTT);
		void SetAnaTwoTheta(double dTT);
		void SetAngleKiQ(double dKiQ);

		void SetReady(bool bReady) { m_bReady = bReady; }
		void nodeMoved(const TasLayoutNode* pNode=0);

		void AllowChanges(bool bAllow) { m_bAllowChanges = bAllow; };

		void SetZoom(double dZoom);

	public:
		std::vector<TasLayoutNode*> GetNodes();
		std::vector<std::string> GetNodeNames() const;

		double GetScaleFactor() const { return m_dScaleFactor; }
		void SetScaleFactor(double dScale) { m_dScaleFactor = dScale; }

		void SetRealQVisible(bool bVisible);
		bool GetRealQVisible() const { return m_bRealQVisible; }
};


class TasLayoutScene : public QGraphicsScene
{	Q_OBJECT
	protected:
		TasLayout *m_pTas;

		bool m_bDontEmitChange = 1;

	public:
		TasLayoutScene();
		virtual ~TasLayoutScene();

		void SetEmitChanges(bool bEmit) { m_bDontEmitChange = !bEmit; }
		void emitUpdate(const TriangleOptions& opts);

		TasLayout* GetTasLayout() { return m_pTas; }

		void emitAllParams();

	public slots:
		void triangleChanged(const TriangleOptions& opts);
		void scaleChanged(double dTotalScale);

		void recipParamsChanged(const RecipParams& params);

	signals:
		// relevant parameters for triangle view
		void tasChanged(const TriangleOptions& opts);
		// all parameters
		void paramsChanged(const RealParams& parms);
};


class TasLayoutView : public QGraphicsView
{
	Q_OBJECT
	protected:
		double m_dTotalScale = 1.;
		virtual void wheelEvent(QWheelEvent* pEvt);

	public:
		TasLayoutView(QWidget* pParent = 0);
		virtual ~TasLayoutView();

	signals:
		void scaleChanged(double dTotalScale);
};


#endif
