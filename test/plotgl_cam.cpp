/*
 * gl plotter
 * @author tweber
 * @date 19-may-2013
 */

#include "plotgl.h"
#include "linalg.h"
#include "math.h"
#include "string.h"
#include "flags.h"

#include <GL/glu.h>
#include <time.h>
#include <iostream>
#include <sstream>


PlotGl::PlotGl(QWidget* pParent)
		: QGLWidget(pParent), m_mutex(QMutex::Recursive),
			m_matProj(unit_matrix<t_mat4>(4))
{
	m_dMouseRot[0] = m_dMouseRot[1] = 0.;
	m_dMouseMove[0] = m_dMouseMove[1] = 0.;

	setAutoBufferSwap(false);
	//setUpdatesEnabled(0);
	doneCurrent();
	start();		// render thread
}

PlotGl::~PlotGl()
{
	m_bRenderThreadActive = 0;
	wait(250);
	terminate();
}

void PlotGl::SetColor(double r, double g, double b, double a)
{
	GLfloat pfCol[] = {GLfloat(r), GLfloat(g), GLfloat(b), GLfloat(a)};
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, pfCol);
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
	glClearColor(1.,1.,1.,0.);
	glShadeModel(GL_SMOOTH);

	glDisable(GL_DEPTH_TEST);
	//glEnable(GL_DEPTH_TEST);
	glClearDepth(1.);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);

	GLfloat vecLightCol[] = { 1., 1., 1., 1. };
	glLightfv(GL_LIGHT0, GL_AMBIENT_AND_DIFFUSE, vecLightCol);

	unsigned int iLOD = 32;
	for(unsigned iSphere=0; iSphere<sizeof(m_iLstSphere)/sizeof(*m_iLstSphere); ++iSphere)
	{
		m_iLstSphere[iSphere] = glGenLists(1);

		GLUquadricObj *pQuadSphere = gluNewQuadric();
		gluQuadricDrawStyle(pQuadSphere, GLU_FILL /*GLU_LINE*/);
		gluQuadricNormals(pQuadSphere, GLU_SMOOTH);
		glNewList(m_iLstSphere[iSphere], GL_COMPILE);
			GLfloat vecLight0[] = { 1., 1., 1., 0. };
			glLightfv(GL_LIGHT0, GL_POSITION, vecLight0);
			gluSphere(pQuadSphere, 1., iLOD, iLOD);
		glEndList();
		gluDeleteQuadric(pQuadSphere);

		iLOD *= 0.8;
	}

	glActiveTexture(GL_TEXTURE0);
	m_pFont = new GlFontMap(DEF_FONT, 12);

	this->setMouseTracking(1);
}

void PlotGl::freeGLThread()
{
	this->setMouseTracking(0);

	for(unsigned iSphere=0; iSphere<sizeof(m_iLstSphere)/sizeof(*m_iLstSphere); ++iSphere)
		glDeleteLists(m_iLstSphere[iSphere], 1);

	if(m_pFont) { delete m_pFont; m_pFont = nullptr; }
}

void PlotGl::resizeGLThread(int w, int h)
{
	if(w<=0) w=1; if(h<=0) h=1;
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	m_matProj = perspective_matrix(m_dFOV, double(w)/double(h), 0.1, 100.);
	//m_matProj = ortho_matrix(-1.,1.,-1.,1.,0.1,100.);
	GLdouble glmat[16]; to_gl_array(m_matProj, glmat);
	glLoadMatrixd(glmat);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void PlotGl::tickThread(double dTime)
{

}

void PlotGl::paintGLThread()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);

	m_mutex.lock();
	t_mat4 matView = m_cam.GetHomMatrix<t_mat4>();
	m_mutex.unlock();
	GLdouble glmat[16];
	to_gl_array(matView, glmat);
	glLoadMatrixd(glmat);


	glPushMatrix();
		glDisable(GL_LIGHTING);
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
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

	glEnable(GL_BLEND);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glDisable(GL_TEXTURE_2D);

	m_mutex.lock();
	unsigned int iPltIdx=0;
	for(const PlotObjGl& obj : m_vecObjs)
	{
		int iLOD = 0;

		bool bColorSet = 0;
		if(obj.bSelected)
		{
			SetColor(0.25, 0.25, 0.25, 0.9);
			bColorSet = 1;
		}

		glPushMatrix();
		if(obj.plttype == PLOT_SPHERE)
		{
			glTranslated(obj.vecParams[0], obj.vecParams[1], obj.vecParams[2]);
			glScaled(obj.vecParams[3], obj.vecParams[3], obj.vecParams[3]);

			//double dLenDist0 = gl_proj_sphere_size(m_dFOV, /*obj.vecParams[3]*/1.);
			double dLenDist = gl_proj_sphere_size(/*obj.vecParams[3]*/1.);
		}
		else if(obj.plttype == PLOT_ELLIPSOID)
		{
			glTranslated(obj.vecParams[3], obj.vecParams[4], obj.vecParams[5]);

			GLdouble dMatRot[] = {obj.vecParams[6], obj.vecParams[7], obj.vecParams[8], 0.,
								obj.vecParams[9], obj.vecParams[10], obj.vecParams[11], 0.,
								obj.vecParams[12], obj.vecParams[13], obj.vecParams[14], 0.,
								0., 0., 0., 1. };
			glMultMatrixd(dMatRot);
			glScaled(obj.vecParams[0], obj.vecParams[1], obj.vecParams[2]);

			//double dRadius = std::max(std::max(obj.vecParams[0], obj.vecParams[1]), obj.vecParams[2]);
		}
		else
			log_warn("Unknown plot object.");

		double dLenDist = gl_proj_sphere_size(/*dRadius*/1.);
		iLOD = dLenDist * 50.;
		if(iLOD >= int(sizeof(m_iLstSphere)/sizeof(*m_iLstSphere)))
			iLOD = sizeof(m_iLstSphere)/sizeof(*m_iLstSphere)-1;
		if(iLOD < 0) iLOD = 0;
		iLOD = sizeof(m_iLstSphere)/sizeof(*m_iLstSphere) - iLOD - 1;
		//std::cout << "dist: " << dLenDist << ", lod: " << iLOD << std::endl;

		if(!bColorSet)
		{
			if(obj.vecColor.size())
				SetColor(obj.vecColor[0], obj.vecColor[1], obj.vecColor[2], obj.vecColor[3]);
			else
				SetColor(iPltIdx);
		}
		glCallList(m_iLstSphere[iLOD]);

		if(obj.bSelected && obj.strLabel.length() && m_pFont && m_pFont->IsOk())
		{
			glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_LIGHTING_BIT);
			m_pFont->BindTexture();
			glColor4d(0., 0., 0., 1.);
			m_pFont->DrawText(0., 0., 0., obj.strLabel);
			glPopAttrib();
		}

		glPopMatrix();

		++iPltIdx;
	}
	m_mutex.unlock();

	glPushMatrix();
		m_mutex.lock();

		if(m_pFont && m_pFont->IsOk())
		{
			m_pFont->BindTexture();

			glColor4d(0., 0., 1., 1.);
			m_pFont->DrawText(m_dXMax*dAxisScale, 0., 0., m_strLabels[0].toStdString());
			m_pFont->DrawText(0., m_dYMax*dAxisScale , 0., m_strLabels[1].toStdString());
			m_pFont->DrawText(0., 0., m_dZMax*dAxisScale , m_strLabels[2].toStdString());

			glColor4d(0., 0., 0., 1.);
			m_pFont->DrawText(m_dXMin, 0., 0., var_to_str(m_dXMin+m_dXMinMaxOffs));
			m_pFont->DrawText(m_dXMax, 0., 0., var_to_str(m_dXMax+m_dXMinMaxOffs));
			m_pFont->DrawText(0., m_dYMin, 0., var_to_str(m_dYMin+m_dYMinMaxOffs));
			m_pFont->DrawText(0., m_dYMax, 0., var_to_str(m_dYMax+m_dYMinMaxOffs));
			m_pFont->DrawText(0., 0., m_dZMin, var_to_str(m_dZMin+m_dZMinMaxOffs));
			m_pFont->DrawText(0., 0., m_dZMax, var_to_str(m_dZMax+m_dZMinMaxOffs));
		}

		m_mutex.unlock();
	glPopMatrix();

	swapBuffers();
}

void PlotGl::run()
{
	makeCurrent();
	initializeGLThread();

	double dTime = 0.;
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
		{
			tickThread(dTime);
			paintGLThread();
		}

		timespec ts;
		long fps = isVisible() ? 60 : 6;
		ts.tv_nsec = 1000000000 / fps;
		ts.tv_sec = 0;
		dTime += double(ts.tv_nsec) * 1e-9;
		nanosleep(&ts, 0);
	}

	freeGLThread();
	//log_info("gl thread ended.");
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
	m_mutex.lock();
	m_vecObjs.clear();
	m_mutex.unlock();
}

void PlotGl::SetObjectColor(int iObjIdx, const std::vector<double>& vecCol)
{
	m_mutex.lock();

	if(m_vecObjs.size() <= (unsigned int)iObjIdx || iObjIdx<0)
		return;

	m_vecObjs[iObjIdx].vecColor = vecCol;

	m_mutex.unlock();
}

void PlotGl::SetObjectLabel(int iObjIdx, const std::string& strLab)
{
	m_mutex.lock();

	if(m_vecObjs.size() <= (unsigned int)iObjIdx || iObjIdx<0)
		return;

	m_vecObjs[iObjIdx].strLabel = strLab;

	m_mutex.unlock();
}

void PlotGl::PlotSphere(const ublas::vector<double>& vecPos,
						double dRadius, int iObjIdx)
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

	obj.plttype = PLOT_SPHERE;
	if(obj.vecParams.size() != 4)
		obj.vecParams.resize(4);

	obj.vecParams[0] = vecPos[0];
	obj.vecParams[1] = vecPos[1];
	obj.vecParams[2] = vecPos[2];
	obj.vecParams[3] = dRadius;

	m_mutex.unlock();
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
		m_bMouseMoveActive = 1;

		m_dMouseMoveBegin[0] = event->posF().x();
		m_dMouseMoveBegin[1] = event->posF().y();
	}
}

void PlotGl::mouseReleaseEvent(QMouseEvent *event)
{
	if((event->buttons() & Qt::RightButton) == 0)
	{
		m_bMouseRotateActive = 0;
		m_dMouseRot[0] = m_dMouseRot[1] = 0.;
	}

	if((event->buttons() & Qt::LeftButton) == 0)
	{
		m_bMouseMoveActive = 0;
		m_dMouseMove[0] = m_dMouseMove[1] = 0.;
	}
}

void PlotGl::mouseMoveEvent(QMouseEvent *pEvt)
{
	bool bUpdateView = 0;
	if(m_bMouseRotateActive)
	{
		double dNewX = pEvt->posF().x();
		double dNewY = pEvt->posF().y();

		m_dMouseRot[0] = dNewX - m_dMouseBegin[0];
		m_dMouseRot[1] = dNewY - m_dMouseBegin[1];

		m_dMouseBegin[0] = dNewX;
		m_dMouseBegin[1] = dNewY;

		bUpdateView = 1;
	}

	if(m_bMouseMoveActive)
	{
		double dNewX = pEvt->posF().x();
		double dNewY = pEvt->posF().y();

		m_dMouseMove[0] = dNewX - m_dMouseMoveBegin[0];
		m_dMouseMove[1] = dNewY - m_dMouseMoveBegin[1];

		m_dMouseMoveBegin[0] = dNewX;
		m_dMouseMoveBegin[1] = dNewY;

		bUpdateView = 1;
	}


	m_mutex.lock();

	if(bUpdateView)
	{
		m_cam.RotateAroundRight(-m_dMouseRot[1]*0.005);
		m_cam.RotateAroundUp(-m_dMouseRot[0]*0.005);
		m_cam.MoveForward(-m_dMouseMove[1]*0.01);
	}


	m_dMouseX = 2.*pEvt->posF().x()/double(m_iW) - 1.;
	m_dMouseY = -(2.*pEvt->posF().y()/double(m_iH) - 1.);
	//std::cout << m_dMouseX << ", " << m_dMouseY << std::endl;
	m_mutex.unlock();

	mouseSelectObj(m_dMouseX, m_dMouseY);
}

void PlotGl::mouseSelectObj(double dX, double dY)
{
	m_mutex.lock();
	Line<double> ray = screen_ray(dX, dY, m_matProj, m_cam.GetHomMatrix<t_mat4>());

	for(PlotObjGl& obj : m_vecObjs)
	{
		obj.bSelected = 0;

		Quadric<double> *pQuad = nullptr;
		t_vec3 vecOffs = ublas::zero_vector<double>(3);

		if(obj.plttype == PLOT_SPHERE)
		{
			pQuad = new QuadSphere<double>(obj.vecParams[3]);
			vecOffs = make_vec<t_vec3>({obj.vecParams[0], obj.vecParams[1], obj.vecParams[2]});
		}
		else if(obj.plttype == PLOT_ELLIPSOID)
		{
			pQuad = new QuadEllipsoid<double>(obj.vecParams[0], obj.vecParams[1], obj.vecParams[2]);

			vecOffs = make_vec<t_vec3>({obj.vecParams[3], obj.vecParams[4], obj.vecParams[5]});
			t_mat3 matRot = make_mat<t_mat3>(
					{{obj.vecParams[6],  obj.vecParams[7],  obj.vecParams[8]},
					 {obj.vecParams[9],  obj.vecParams[10], obj.vecParams[11]},
					 {obj.vecParams[12], obj.vecParams[13], obj.vecParams[14]}});
			pQuad->transform(matRot);
		}

		pQuad->SetOffset(vecOffs);

		std::vector<double> vecT = pQuad->intersect(ray);
		if(vecT.size() > 0)
		{
			for(double t : vecT)
			{
				if(t < 0.) continue; // beyond "near" plane
				if(t > 1.) continue; // beyond "far" plane
				obj.bSelected = 1;
			}
		}

		delete pQuad;
	}
	m_mutex.unlock();
}

void PlotGl::SetLabels(const char* pcLabX, const char* pcLabY, const char* pcLabZ)
{
	m_mutex.lock();
	m_strLabels[0] = pcLabX;
	m_strLabels[1] = pcLabY;
	m_strLabels[2] = pcLabZ;
	m_mutex.unlock();
}
