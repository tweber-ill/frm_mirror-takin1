/*
 * gl plotter
 * @author tweber
 * @date 19-may-2013
 */

#ifndef __MIEZE_PLOT_GL__
#define __MIEZE_PLOT_GL__

#include <QtOpenGL/QGLWidget>
#include <QtGui/QMouseEvent>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <vector>

#include "gl.h"

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
namespace ublas = boost::numeric::ublas;


enum PlotTypeGl
{
	PLOT_INVALID,

	PLOT_SPHERE,
	PLOT_ELLIPSOID,
};

struct PlotObjGl
{
	PlotTypeGl plttype = PLOT_INVALID;
	std::vector<double> vecParams;
	std::vector<double> vecColor;

	bool bSelected;
	std::string strLabel;
};

class PlotGl : public QGLWidget, QThread
{
protected:
	QMutex m_mutex;

	Cam<t_mat3, t_vec3> m_cam;
	static constexpr double m_dFOV = 45./180.*M_PI;
	t_mat4 m_matProj;

	GlFontMap *m_pFont = nullptr;

	std::vector<PlotObjGl> m_vecObjs;
	GLuint m_iLstSphere[8];
	QString m_strLabels[3];
	//QFont *m_pfont=0, *m_pfontsmall=0;

	double m_dXMin=-10., m_dXMax=10.;
	double m_dYMin=-10., m_dYMax=10.;
	double m_dZMin=-10., m_dZMax=10.;
	double m_dXMinMaxOffs, m_dYMinMaxOffs, m_dZMinMaxOffs;

	//void initializeGL();
	void resizeEvent(QResizeEvent*);
	void paintEvent(QPaintEvent*);

	void SetColor(double r, double g, double b, double a=1.);
	void SetColor(unsigned int iIdx);

	// ------------------------------------------------------------------------
	// mouse stuff
	bool m_bMouseRotateActive = 0;
	double m_dMouseRot[2];
	double m_dMouseBegin[2];

	bool m_bMouseMoveActive = 0;
	double m_dMouseMove[2];
	double m_dMouseMoveBegin[2];

	double m_dMouseX = 0., m_dMouseY = 0.;

	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);

	void mouseSelectObj(double dX, double dY);

	// ------------------------------------------------------------------------
	// render thread
	bool m_bGLInited = 0;
	bool m_bDoResize = 1;
	bool m_bRenderThreadActive = 1;

	void initializeGLThread();
	void freeGLThread();
	void resizeGLThread(int w, int h);
	void paintGLThread();
	void tickThread(double dTime);
	void run();

	int m_iW=640, m_iH=480;
	// ------------------------------------------------------------------------

public:
	PlotGl(QWidget* pParent);
	virtual ~PlotGl();

	void PlotSphere(const ublas::vector<double>& vecPos, double dRadius, int iObjIdx=-1);
	void PlotEllipsoid(const ublas::vector<double>& widths,
						const ublas::vector<double>& offsets,
						const ublas::matrix<double>& rot,
						int iObjsIdx=-1);
	void SetObjectCount(unsigned int iSize) { m_vecObjs.resize(iSize); }
	void SetObjectColor(int iObjIdx, const std::vector<double>& vecCol);
	void SetObjectLabel(int iObjIdx, const std::string& strLab);
	void clear();

	void SetLabels(const char* pcLabX, const char* pcLabY, const char* pcLabZ);

	template<class t_vec>
	void SetMinMax(const t_vec& vecMin, const t_vec& vecMax, const t_vec* pOffs=0)
	{
		m_dXMin = vecMin[0]; m_dXMax = vecMax[0];
		m_dYMin = vecMin[1]; m_dYMax = vecMax[1];
		m_dZMin = vecMin[2]; m_dZMax = vecMax[2];

		m_dXMinMaxOffs =  pOffs ? (*pOffs)[0] : 0.;
		m_dYMinMaxOffs =  pOffs ? (*pOffs)[1] : 0.;
		m_dZMinMaxOffs =  pOffs ? (*pOffs)[2] : 0.;
	}

	template<class t_vec=ublas::vector<double> >
	void SetMinMax(const t_vec& vec, const t_vec* pOffs=0)
	{
		m_dXMin = -vec[0]; m_dXMax = vec[0];
		m_dYMin = -vec[1]; m_dYMax = vec[1];
		m_dZMin = -vec[2]; m_dZMax = vec[2];

		m_dXMinMaxOffs =  pOffs ? (*pOffs)[0] : 0.;
		m_dYMinMaxOffs =  pOffs ? (*pOffs)[1] : 0.;
		m_dZMinMaxOffs =  pOffs ? (*pOffs)[2] : 0.;
	}

};


#endif
