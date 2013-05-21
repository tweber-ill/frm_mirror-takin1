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

class PlotGl : public QGLWidget
{
protected:
	std::vector<PlotObjGl> m_vecObjs;
	GLuint m_iLstSphere;


	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

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

public:
	PlotGl(QWidget* pParent);
	virtual ~PlotGl();

	void PlotEllipsoid(const ublas::vector<double>& widths,
						const ublas::vector<double>& offsets,
						const ublas::matrix<double>& rot,
						int iObjsIdx=-1);
	void clear();
};


#endif
