/*
 * gl plotter
 * @author tweber
 * @date 19-may-2013
 */

#include "plotgl.h"
#include "../tlibs/math/linalg.h"
#include "../tlibs/math/math.h"
#include "../tlibs/string/string.h"
#include "../tlibs/helper/flags.h"

#include <GL/glu.h>
#include <time.h>
#include <iostream>
#include <sstream>


PlotGl::PlotGl(QWidget* pParent, QSettings *pSettings)
		: QGLWidget(pParent), m_pSettings(pSettings),
			m_mutex(QMutex::Recursive),
			m_matProj(tl::unit_matrix<tl::t_mat4>(4)), m_matView(tl::unit_matrix<tl::t_mat4>(4))
{
	m_dMouseRot[0] = m_dMouseRot[1] = 0.;
	m_dMouseScale = 25.;
	updateViewMatrix();

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

void PlotGl::SetEnabled(bool b)
{
	m_mutex.lock();
	m_bEnabled = b;
	m_mutex.unlock();
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

	std::string strFont = DEF_FONT;
	if(m_pSettings)
	{
		if(m_pSettings->contains("gl/font"))
			strFont = m_pSettings->value("gl/font").toString().toStdString();
	}
	m_pFont = new tl::GlFontMap(strFont.c_str(), 12);

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
	m_matProj = tl::perspective_matrix(m_dFOV, double(w)/double(h), 0.1, 100.);
	//m_matProj = ortho_matrix(-1.,1.,-1.,1.,0.1,100.);
	GLdouble glmat[16]; tl::to_gl_array(m_matProj, glmat);
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

	m_mutex.lock();
	bool bEnabled = m_bEnabled;
	m_mutex.unlock();
	if(!bEnabled) return;

	glMatrixMode(GL_MODELVIEW);
	GLdouble glmat[16];
	m_mutex.lock();
	tl::to_gl_array(m_matView, glmat);
	m_mutex.unlock();
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
		}
		else
			tl::log_warn("Unknown plot object.");

		if(obj.bUseLOD)
		{
			double dLenDist = tl::gl_proj_sphere_size(/*dRadius*/1.);
			//std::cout << "proj sphere size: " << dLenDist << std::endl;
			iLOD = dLenDist * 50.;
			if(iLOD >= int(sizeof(m_iLstSphere)/sizeof(*m_iLstSphere)))
				iLOD = sizeof(m_iLstSphere)/sizeof(*m_iLstSphere)-1;
			if(iLOD < 0) iLOD = 0;
			iLOD = sizeof(m_iLstSphere)/sizeof(*m_iLstSphere) - iLOD - 1;
			//std::cout << "dist: " << dLenDist << ", lod: " << iLOD << std::endl;
		}

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
		if(m_pFont && m_pFont->IsOk())
		{
			m_pFont->BindTexture();

			glColor4d(0., 0., 1., 1.);
			m_pFont->DrawText(m_dXMax*dAxisScale, 0., 0., m_strLabels[0].toStdString());
			m_pFont->DrawText(0., m_dYMax*dAxisScale , 0., m_strLabels[1].toStdString());
			m_pFont->DrawText(0., 0., m_dZMax*dAxisScale , m_strLabels[2].toStdString());

			glColor4d(0., 0., 0., 1.);
			m_pFont->DrawText(m_dXMin, 0., 0., tl::var_to_str(m_dXMin+m_dXMinMaxOffs));
			m_pFont->DrawText(m_dXMax, 0., 0., tl::var_to_str(m_dXMax+m_dXMinMaxOffs));
			m_pFont->DrawText(0., m_dYMin, 0., tl::var_to_str(m_dYMin+m_dYMinMaxOffs));
			m_pFont->DrawText(0., m_dYMax, 0., tl::var_to_str(m_dYMax+m_dYMinMaxOffs));
			m_pFont->DrawText(0., 0., m_dZMin, tl::var_to_str(m_dZMin+m_dZMinMaxOffs));
			m_pFont->DrawText(0., 0., m_dZMax, tl::var_to_str(m_dZMax+m_dZMinMaxOffs));
		}
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

void PlotGl::SetObjectUseLOD(int iObjIdx, bool bLOD)
{
	m_mutex.lock();
	if(m_vecObjs.size() <= (unsigned int)iObjIdx || iObjIdx<0)
		return;
	m_vecObjs[iObjIdx].bUseLOD = bLOD;
	m_mutex.unlock();
}

void PlotGl::PlotSphere(const ublas::vector<double>& vecPos,
						double dRadius, int iObjIdx)
{
	if(iObjIdx < 0)
	{
		clear();
		iObjIdx = 0;
	}

	m_mutex.lock();

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
	if(iObjIdx < 0)
	{
		clear();
		iObjIdx = 0;
	}

	m_mutex.lock();

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

void PlotGl::mouseMoveEvent(QMouseEvent *pEvt)
{
	bool bUpdateView = 0;
	if(m_bMouseRotateActive)
	{
		double dNewX = pEvt->posF().x();
		double dNewY = pEvt->posF().y();

		m_dMouseRot[0] += dNewX - m_dMouseBegin[0];
		m_dMouseRot[1] += dNewY - m_dMouseBegin[1];

		m_dMouseBegin[0] = dNewX;
		m_dMouseBegin[1] = dNewY;

		bUpdateView = 1;
	}

	if(m_bMouseScaleActive)
	{
		double dNewY = pEvt->posF().y();

		m_dMouseScale *= 1.-(dNewY - m_dMouseScaleBegin)/double(height()) * 2.;
		m_dMouseScaleBegin = dNewY;

		bUpdateView = 1;
	}

	if(bUpdateView)
		updateViewMatrix();



	m_dMouseX = 2.*pEvt->posF().x()/double(m_iW) - 1.;
	m_dMouseY = -(2.*pEvt->posF().y()/double(m_iH) - 1.);
	//std::cout << m_dMouseX << ", " << m_dMouseY << std::endl;

	m_mutex.lock();
	bool bEnabled = m_bEnabled;
	m_mutex.unlock();

	if(bEnabled)
		mouseSelectObj(m_dMouseX, m_dMouseY);
}

void PlotGl::updateViewMatrix()
{
	tl::t_mat4 matScale = tl::make_mat<tl::t_mat4>(
			{{m_dMouseScale,              0,             0, 0},
			{            0,   m_dMouseScale,             0, 0},
			{            0,               0, m_dMouseScale, 0},
			{            0,               0,             0, 1}});

	tl::t_mat4 matR0 = tl::rotation_matrix_3d_z(m_dMouseRot[0]/180.*M_PI);
	tl::t_mat4 matR1 = tl::rotation_matrix_3d_x((-90. + m_dMouseRot[1])/180.*M_PI);
	matR0.resize(4,4,1); matR1.resize(4,4,1);
	matR0(3,3) = matR1(3,3) = 1.;
	for(short i=0; i<3; ++i) matR0(i,3)=matR0(3,i)=matR1(i,3)=matR1(3,i)=0.;
	tl::t_mat4 matRot0 = matR0, matRot1 = matR1;

	tl::t_mat4 matTrans = tl::make_mat<tl::t_mat4>(
			{{ 1, 0, 0,  0},
			{  0, 1, 0,  0},
			{  0, 0, 1, -2},
			{  0, 0, 0,  1}});

	m_mutex.lock();
	m_matView = ublas::prod(matTrans, matRot1);
	m_matView = ublas::prod(m_matView, matRot0);
	m_matView = ublas::prod(m_matView, matScale);
	m_mutex.unlock();
}

void PlotGl::mouseSelectObj(double dX, double dY)
{
	m_mutex.lock();
	tl::Line<double> ray = tl::screen_ray(dX, dY, m_matProj, m_matView);

	for(PlotObjGl& obj : m_vecObjs)
	{
		obj.bSelected = 0;

		tl::Quadric<double> *pQuad = nullptr;
		tl::t_vec3 vecOffs = ublas::zero_vector<double>(3);

		if(obj.plttype == PLOT_SPHERE)
		{
			pQuad = new tl::QuadSphere<double>(obj.vecParams[3]);
			vecOffs = tl::make_vec<tl::t_vec3>({obj.vecParams[0], obj.vecParams[1], obj.vecParams[2]});
		}
		else if(obj.plttype == PLOT_ELLIPSOID)
		{
			pQuad = new tl::QuadEllipsoid<double>(obj.vecParams[0], obj.vecParams[1], obj.vecParams[2]);

			vecOffs = tl::make_vec<tl::t_vec3>({obj.vecParams[3], obj.vecParams[4], obj.vecParams[5]});
			tl::t_mat3 matRot = tl::make_mat<tl::t_mat3>(
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



/*
// gcc -std=c++11 -I/usr/include/freetype2 -o tst_gl helper/plotgl.cpp helper/log.cpp helper/gl.cpp -lm -lstdc++ -lQtGui -lQtCore -lQtOpenGL -lGL -lGLU -lX11 -lfreetype

#include <QtGui/QApplication>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>

extern "C" int XInitThreads();
int main(int argc, char **argv)
{
	//QuadSphere<double> sph(1.23);
	//sph.SetOffset(make_vec({0.,0.,0.}));
	//Line<double> line(make_vec({0.,0.,0.0}), make_vec({0.,0.,1.}));
	//std::vector<double> vec = sph.intersect(line);
	//for(double d : vec)
	//	std::cout << d << ", ";
	//std::cout << std::endl;

	XInitThreads();
	QApplication a(argc, argv);

	QDialog dlg;
	PlotGl *pGl = new PlotGl(&dlg);

	pGl->SetObjectCount(3);
	pGl->PlotSphere(make_vec({0., 0.5, -0.5}), 0.2, 0);
	pGl->SetObjectColor(0, {1., 0., 0., 0.5});

	pGl->PlotSphere(make_vec({0., 0., -2.}), 0.4, 1);
	pGl->SetObjectColor(1, {0., 0., 1., 0.5});

	pGl->PlotEllipsoid(make_vec({0.1, 0.2, 0.3}),
						make_vec({0., 0.5, 0.}),
						make_mat({{1.,0.,0.},{0.,1.,0.},{0.,0.,1.}}), 2);
	pGl->SetObjectColor(2, {0., 1., 0., 0.5});

	pGl->SetMinMax(make_vec({-1.,-1.,-1.}), make_vec({1.,1.,1.}));

	dlg.resize(640,480);
	QGridLayout *pGrid = new QGridLayout(&dlg);
	pGrid->addWidget(pGl, 0,0,1,1);
	dlg.exec();

	delete pGl;
	return 0;
}
*/
