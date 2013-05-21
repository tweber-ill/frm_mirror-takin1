/*
 * mieze-tool
 * plotter
 * @author tweber
 * @date 19-may-2013
 */

#include "plotgl.h"
#include <GL/glu.h>
#include <cmath>
#include <iostream>


PlotGl::PlotGl(QWidget* pParent) : QGLWidget(pParent),
								m_bMouseRotateActive(0), m_bMouseScaleActive(0)
{
	m_dMouseRot[0] = m_dMouseRot[1] = 0.;
	m_dMouseScale = 25.;
}

PlotGl::~PlotGl()
{}

void PlotGl::SetColor(unsigned int iIdx)
{
	static const GLfloat cols[4][4] =
	{
		{ 0., 0., 1., 0.8 },
		{ 0., 0.5, 0., 0.8 },
		{ 1., 0., 0., 0.8 },
		{ 0., 0., 0., 0.8 }
	};

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, cols[iIdx % 4]);
}

void PlotGl::initializeGL()
{
	glClearColor(1.,1.,1.,0.);
	glShadeModel(GL_SMOOTH);

	glDisable(GL_DEPTH_TEST);
	//glClearDepth(1.0f);
	//glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(GL_CULL_FACE);

	GLfloat vecLightCol[] = { 1., 1., 1., 1. };
	glLightfv(GL_LIGHT0, GL_AMBIENT_AND_DIFFUSE, vecLightCol);


	m_iLstSphere = glGenLists(1);

	GLUquadricObj *pQuad = gluNewQuadric();
	gluQuadricDrawStyle(pQuad, GLU_FILL);
	gluQuadricNormals(pQuad, GLU_SMOOTH);
	glNewList(m_iLstSphere, GL_COMPILE);
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);

		GLfloat vecLight0[] = { 1., 1., 1., 0. };
		glLightfv(GL_LIGHT0, GL_POSITION, vecLight0);

		gluSphere(pQuad, 1., 32, 32);
	glEndList();
}

void PlotGl::resizeGL(int w, int h)
{
	glViewport(0,0,w,h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90., double(w)/double(h), 0.1, 100.);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void PlotGl::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();

	glTranslated(0.,0.,-2.);

	glRotated(m_dMouseRot[1], 1., 0., 0.);
	glRotated(m_dMouseRot[0], 0., 1., 0.);

	glScaled(m_dMouseScale, m_dMouseScale, m_dMouseScale);

	glPushMatrix();
		glDisable(GL_LIGHTING);
		glLineWidth(2.);
		glColor3d(0., 0., 0.);
		glBegin(GL_LINES);
		glVertex3d(-10., 0., 0.);
		glVertex3d(10, 0., 0.);
		glVertex3d(0., -10., 0.);
		glVertex3d(0, 10., 0.);
		glVertex3d(0., 0., -10.);
		glVertex3d(0, 0., 10.);
		glEnd();
	glPopMatrix();


	unsigned int iPltIdx=0;
	for(const PlotObjGl& obj : m_vecObjs)
	{
		if(obj.plttype == PLOT_ELLIPSOID)
		{
			glPushMatrix();

			glTranslated(-obj.vecParams[3], -obj.vecParams[4], -obj.vecParams[5]);

			GLdouble dMatRot[] = {obj.vecParams[6], obj.vecParams[7], obj.vecParams[8], 0.,
								obj.vecParams[9], obj.vecParams[10], obj.vecParams[11], 0.,
								obj.vecParams[12], obj.vecParams[13], obj.vecParams[14], 0.,
								0., 0., 0., 1. };
			glMultMatrixd(dMatRot);

			glScaled(obj.vecParams[0], obj.vecParams[1], obj.vecParams[2]);

			glEnable(GL_LIGHTING);
			glEnable(GL_LIGHT0);

			SetColor(iPltIdx);
			glCallList(m_iLstSphere);
			glPopMatrix();
		}

		++iPltIdx;
	}

	glPopMatrix();
	glFlush();
}

void PlotGl::clear()
{
	m_vecObjs.clear();
}

void PlotGl::PlotEllipsoid(const ublas::vector<double>& widths,
							const ublas::vector<double>& offsets,
							const ublas::matrix<double>& rot,
							int iObjIdx)
{
	PlotObjGl obj;
	obj.plttype = PLOT_ELLIPSOID;
	obj.vecParams.push_back(widths[0]);
	obj.vecParams.push_back(widths[1]);
	obj.vecParams.push_back(widths[2]);
	obj.vecParams.push_back(offsets[0]);
	obj.vecParams.push_back(offsets[1]);
	obj.vecParams.push_back(offsets[2]);
	for(unsigned int i=0; i<3; ++i)
		for(unsigned int j=0; j<3; ++j)
			obj.vecParams.push_back(rot(j,i));

	if(iObjIdx < 0)
		clear();

	if(iObjIdx >= m_vecObjs.size())
		m_vecObjs.resize(iObjIdx+1);
	m_vecObjs[iObjIdx] = obj;
	
	glDraw();
}

void PlotGl::mousePressEvent(QMouseEvent *event)
{
	if(event->buttons() & Qt::RightButton)
	{
		m_bMouseRotateActive = 1;

		m_dMouseBegin[0] = event->posF().x();
		m_dMouseBegin[1] = event->posF().y();
	}

	if(event->buttons() & Qt::LeftButton)
	{
		m_bMouseScaleActive = 1;
		m_dMouseScaleBegin = event->posF().y();
	}
}

void PlotGl::mouseReleaseEvent(QMouseEvent *event)
{
	if((event->buttons() & Qt::RightButton) == 0)
		m_bMouseRotateActive = 0;

	if((event->buttons() & Qt::LeftButton) == 0)
		m_bMouseScaleActive = 0;
}

void PlotGl::mouseMoveEvent(QMouseEvent *event)
{
	bool bUpdate = 0;

	if(m_bMouseRotateActive)
	{
		double dNewX = event->posF().x();
		double dNewY = event->posF().y();

		m_dMouseRot[0] += dNewX - m_dMouseBegin[0];
		m_dMouseRot[1] += dNewY - m_dMouseBegin[1];

		m_dMouseBegin[0] = dNewX;
		m_dMouseBegin[1] = dNewY;

		bUpdate = 1;
	}

	if(m_bMouseScaleActive)
	{
		double dNewY = event->posF().y();

		m_dMouseScale *= 1.-(dNewY - m_dMouseScaleBegin)/double(height()) * 2.;
		m_dMouseScaleBegin = dNewY;

		bUpdate = 1;
	}

	if(bUpdate)
		glDraw();
}


/*
#include <QtGui/QApplication>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>

int main(int argc, char **argv)
{
	QApplication a(argc, argv);

	QDialog dlg;
	PlotGl *pGl = new PlotGl(&dlg);

	pGl->PlotEllipsoid(1.,0.5,0.75, 0.,0.,M_PI/4, 0.,0.,1.,0);

	dlg.resize(640,480);
	QGridLayout *pGrid = new QGridLayout(&dlg);
	pGrid->addWidget(pGl, 0,0,1,1);
	dlg.exec();

	return 0;
}
*/

