/*
 * mieze-tool
 * plotter
 * @author tweber
 * @date 19-may-2013
 */

#include "plotgl.h"
#include <GL/glu.h>
#include <cmath>
#include <time.h>
#include <iostream>
#include <sstream>

static QString to_string(double d)
{
	std::ostringstream ostr;
	ostr.precision(3);
	ostr << d;

	return QString(ostr.str().c_str());
}

QMutex PlotGl::s_mutexShared(QMutex::Recursive);


PlotGl::PlotGl(QWidget* pParent) : QGLWidget(pParent),
								m_mutex(QMutex::Recursive),
								m_bMouseRotateActive(0), m_bMouseScaleActive(0),
								m_bRenderThreadActive(1), m_bGLInited(0), m_bDoResize(1),
								m_iW(640), m_iH(480),
								m_pfont(0),
								m_pfontsmall(0),
								m_dXMin(-10.), m_dXMax(10.),
								m_dYMin(-10.), m_dYMax(10.),
								m_dZMin(-10.), m_dZMax(10.)
{
	m_dMouseRot[0] = m_dMouseRot[1] = 0.;
	m_dMouseScale = 25.;

	setAutoBufferSwap(false);
	//setUpdatesEnabled(0);
	doneCurrent();
	start(); 		// render thread
}

PlotGl::~PlotGl()
{
	m_bRenderThreadActive = 0;
	terminate();
	wait();

	if(m_pfont) delete m_pfont;
	if(m_pfontsmall) delete m_pfontsmall;
}


void PlotGl::SetColor(unsigned int iIdx)
{
	static const GLfloat cols[4][4] =
	{
		{ 0., 0., 1., 0.7 },
		{ 0., 0.5, 0., 0.7 },
		{ 1., 0., 0., 0.7 },
		{ 0., 0., 0., 0.7 }
	};

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, cols[iIdx % 4]);
}

void PlotGl::initializeGLThread()
{
	m_pfont = new QFont("Nimbus Sans L", 12);
	m_pfontsmall = new QFont("Nimbus Sans L", 8);

	glClearColor(1.,1.,1.,0.);
	glShadeModel(GL_SMOOTH);

	glDisable(GL_DEPTH_TEST);
	glClearDepth(1.);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);

	GLfloat vecLightCol[] = { 1., 1., 1., 1. };
	glLightfv(GL_LIGHT0, GL_AMBIENT_AND_DIFFUSE, vecLightCol);


	m_iLstSphere = glGenLists(1);

	GLUquadricObj *pQuad = gluNewQuadric();
	gluQuadricDrawStyle(pQuad, GLU_FILL);
	gluQuadricNormals(pQuad, GLU_SMOOTH);
	glNewList(m_iLstSphere, GL_COMPILE);
		GLfloat vecLight0[] = { 1., 1., 1., 0. };
		glLightfv(GL_LIGHT0, GL_POSITION, vecLight0);

		gluSphere(pQuad, 1., 32, 32);
	glEndList();
}

void PlotGl::resizeGLThread(int w, int h)
{
	if(w<0) w=1;
	if(h<0) h=1;
	glViewport(0,0,w,h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45., double(w)/double(h), 0.1, 100.);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void PlotGl::paintGLThread()
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

		const double dAxisScale = 1.8;
		glBegin(GL_LINES);
			glVertex3d(m_dXMin*dAxisScale, 0., 0.);
			glVertex3d(m_dXMax*dAxisScale, 0., 0.);
			glVertex3d(0., m_dYMin*dAxisScale, 0.);
			glVertex3d(0., m_dYMax*dAxisScale, 0.);
			glVertex3d(0., 0., m_dZMin*dAxisScale);
			glVertex3d(0., 0., m_dZMax*dAxisScale);
		glEnd();
	glPopMatrix();

	m_mutex.lock();
	unsigned int iPltIdx=0;
	for(const PlotObjGl& obj : m_vecObjs)
	{
		if(obj.plttype == PLOT_ELLIPSOID)
		{
			glPushMatrix();
			//glTranslated(obj.vecParams[3], obj.vecParams[4], obj.vecParams[5]);

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
	m_mutex.unlock();

	glPushMatrix();
		glDisable(GL_LIGHTING);
		glColor3d(0., 0., 0.);

		s_mutexShared.lock();
		m_mutex.lock();

		if(m_pfont)
		{
			renderText(m_dXMax*dAxisScale, 0., 0., m_strLabels[0], *m_pfont);
			renderText(0., m_dYMax*dAxisScale , 0., m_strLabels[1], *m_pfont);
			renderText(0., 0., m_dZMax*dAxisScale , m_strLabels[2], *m_pfont);
		}
		if(m_pfontsmall)
		{
			renderText(m_dXMin, 0., 0., to_string(m_dXMin+m_dXMinMaxOffs), *m_pfontsmall);
			renderText(m_dXMax, 0., 0., to_string(m_dXMax+m_dXMinMaxOffs), *m_pfontsmall);
			renderText(0., m_dYMin, 0., to_string(m_dYMin+m_dYMinMaxOffs), *m_pfontsmall);
			renderText(0., m_dYMax, 0., to_string(m_dYMax+m_dYMinMaxOffs), *m_pfontsmall);
			renderText(0., 0., m_dZMin, to_string(m_dZMin+m_dZMinMaxOffs), *m_pfontsmall);
			renderText(0., 0., m_dZMax, to_string(m_dZMax+m_dZMinMaxOffs), *m_pfontsmall);
		}
		m_mutex.unlock();
		s_mutexShared.unlock();
	glPopMatrix();

	glPopMatrix();
	swapBuffers();
	//glFlush();
}

void PlotGl::run()
{
	makeCurrent();
	initializeGLThread();

	while(m_bRenderThreadActive)
	{
		if(m_bDoResize)
		{
			m_mutex.lock();
			resizeGLThread(m_iW, m_iH);
			m_bDoResize = 0;
			m_mutex.unlock();
		}

		if(isVisible())
			paintGLThread();

		timespec ts;
		ts.tv_nsec = 50000;
		ts.tv_sec = 0;
		nanosleep(&ts, 0);
	}
}

void PlotGl::paintEvent(QPaintEvent *evt)
{}

void PlotGl::resizeEvent(QResizeEvent *evt)
{
	m_mutex.lock();
	m_iW = size().width();
	m_iH = size().height();
	m_bDoResize = 1;
	m_mutex.unlock();
}

void PlotGl::clear()
{
	m_vecObjs.clear();
}

void PlotGl::SetMinMax(const ublas::vector<double>& vec, const ublas::vector<double>* pOffs)
{
	m_dXMin = -vec[0]; m_dXMax = vec[0];
	m_dYMin = -vec[1]; m_dYMax = vec[1];
	m_dZMin = -vec[2]; m_dZMax = vec[2];

	m_dXMinMaxOffs =  pOffs ? (*pOffs)[0] : 0.;
	m_dYMinMaxOffs =  pOffs ? (*pOffs)[1] : 0.;
	m_dZMinMaxOffs =  pOffs ? (*pOffs)[2] : 0.;
}

void PlotGl::PlotEllipsoid(const ublas::vector<double>& widths,
							const ublas::vector<double>& offsets,
							const ublas::matrix<double>& rot,
							int iObjIdx)
{
	m_mutex.lock();

	if(iObjIdx < 0)
	{
		clear();
		iObjIdx = 0;
	}

	if(iObjIdx >= int(m_vecObjs.size()))
		m_vecObjs.resize(iObjIdx+1);
	PlotObjGl& obj = m_vecObjs[iObjIdx];

	obj.plttype = PLOT_ELLIPSOID;
	if(obj.vecParams.size() != 15)
		obj.vecParams.resize(15);

	obj.vecParams[0] = widths[0];
	obj.vecParams[1] = widths[1];
	obj.vecParams[2] = widths[2];
	obj.vecParams[3] = offsets[0];
	obj.vecParams[4] = offsets[1];
	obj.vecParams[5] = offsets[2];
	unsigned int iNum = 6;
	for(unsigned int i=0; i<3; ++i)
		for(unsigned int j=0; j<3; ++j)
			obj.vecParams[iNum++] = rot(j,i);
	
	m_mutex.unlock();
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
	if(m_bMouseRotateActive)
	{
		double dNewX = event->posF().x();
		double dNewY = event->posF().y();

		m_dMouseRot[0] += dNewX - m_dMouseBegin[0];
		m_dMouseRot[1] += dNewY - m_dMouseBegin[1];

		m_dMouseBegin[0] = dNewX;
		m_dMouseBegin[1] = dNewY;
	}

	if(m_bMouseScaleActive)
	{
		double dNewY = event->posF().y();

		m_dMouseScale *= 1.-(dNewY - m_dMouseScaleBegin)/double(height()) * 2.;
		m_dMouseScaleBegin = dNewY;
	}
}

void PlotGl::SetLabels(const char* pcLabX, const char* pcLabY, const char* pcLabZ)
{
	m_mutex.lock();
	m_strLabels[0] = pcLabX;
	m_strLabels[1] = pcLabY;
	m_strLabels[2] = pcLabZ;
	m_mutex.unlock();
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
