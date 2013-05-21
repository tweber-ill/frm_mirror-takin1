/*
 * mieze-tool
 * plotter
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

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
namespace ublas = boost::numeric::ublas;


enum PlotTypeGl
{
	PLOT_ELLIPSOID,
};

struct PlotObjGl
{
	PlotTypeGl plttype;
	std::vector<double> vecParams;
};

class PlotGl : public QGLWidget, QThread
{
protected:
	static QMutex m_mutex;

	std::vector<PlotObjGl> m_vecObjs;
	GLuint m_iLstSphere;
	QString m_strLabels[3];
	QFont *m_pfont, *m_pfontsmall;

	double m_dXMin, m_dXMax;
	double m_dYMin, m_dYMax;
	double m_dZMin, m_dZMax;

	//void initializeGL();
	void resizeEvent(QResizeEvent *evt);
	void paintEvent(QPaintEvent *evt);
	void SetColor(unsigned int iIdx);


	bool m_bMouseRotateActive;
	double m_dMouseRot[2];
	double m_dMouseBegin[2];

	bool m_bMouseScaleActive;
	double m_dMouseScale;
	double m_dMouseScaleBegin;

	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

	// ------------------------------------------------------------------------
	// render thread
	bool m_bGLInited;
	bool m_bDoResize;
	bool m_bRenderThreadActive;

	void initializeGLThread();
	void resizeGLThread(int w, int h);
	void paintGLThread();
	void run();

	int m_iW, m_iH;
	// ------------------------------------------------------------------------

public:
	PlotGl(QWidget* pParent);
	virtual ~PlotGl();

	void PlotEllipsoid(const ublas::vector<double>& widths,
						const ublas::vector<double>& offsets,
						const ublas::matrix<double>& rot,
						int iObjsIdx=-1);
	void clear();

	void SetLabels(const char* pcLabX, const char* pcLabY, const char* pcLabZ);
	void SetMinMax(const ublas::vector<double>& vec);
};


#endif
