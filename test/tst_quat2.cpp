// moc-qt5 tst_quat2.cpp > tst_quat2.moc && gcc -fPIC -I/usr/include/qt5 -o tst_quat2 tst_quat2.cpp -lstdc++ -lm -lQt5Core -lQt5Gui -lQt5OpenGL -lQt5Widgets -lGL -std=c++14

#include <iostream>

#include "../helper/linalg.h"
#include "../helper/quat.h"

#include <QtOpenGL/QGLWidget>
#include <QtGui/QOpenGLFunctions_2_0>
#include <QtGui/QApplication>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtCore/QTimer>


static inline void to_array(const ublas::matrix<double>& mat, GLdouble* glmat)
{
	glmat[0]=mat(0,0);  glmat[1]=mat(1,0);  glmat[2]=mat(2,0);
	glmat[4]=mat(0,1);  glmat[5]=mat(1,1);  glmat[6]=mat(2,1);
	glmat[8]=mat(0,2);  glmat[9]=mat(1,2);  glmat[10]=mat(2,2);

	if(mat.size1()>=4 && mat.size2()>=4)
	{
		glmat[3]=mat(3,0); glmat[7]=mat(3,1); glmat[11]=mat(3,2);
		glmat[12]=mat(0,3); glmat[13]=mat(1,3); glmat[14]=mat(2,3); glmat[15]=mat(3,3);
	}
	else
	{
		glmat[3]=0; glmat[7]=0; glmat[11]=0;
		glmat[12]=0; glmat[13]=0; glmat[14]=0; glmat[15]=1;
	}
}

class GlWidget : public QGLWidget /*QOpenGLWidget*/
{ Q_OBJECT
	protected:
		QOpenGLFunctions_2_0 *m_pGL = 0;
		double m_dTick = 0.;
		QTimer m_timer;

	protected slots:
		void tick()
		{
			m_dTick += 0.02;
			update();
		}

	public:
		GlWidget(QWidget* pParent) : QGLWidget/*QOpenGLWidget*/(pParent), m_timer(this)
		{
			QObject::connect(&m_timer, &QTimer::timeout, this, &GlWidget::tick);
			m_timer.start(5);
		}
		virtual ~GlWidget()
		{
			m_timer.stop();
		}

	protected:
		virtual void initializeGL() override
		{
			m_pGL = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_2_0>();
			m_pGL->initializeOpenGLFunctions();

			m_pGL->glClearColor(1.,1.,1.,0.);
			m_pGL->glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
			m_pGL->glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
			m_pGL->glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
			m_pGL->glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
		}

		virtual void resizeGL(int w, int h) override
		{
			if(w<0) w=1; if(h<0) h=1;
			m_pGL->glViewport(0,0,w,h);

			m_pGL->glMatrixMode(GL_PROJECTION);

			ublas::matrix<double> mat = perspective_matrix(45./180.*M_PI, double(w)/double(h), 0.1, 100.);
			GLdouble glmat[16]; to_array(mat, glmat);
			m_pGL->glLoadMatrixd(glmat);

			m_pGL->glMatrixMode(GL_MODELVIEW);
			m_pGL->glLoadIdentity();
		}

		virtual void paintGL() override
		{
			m_pGL->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			m_pGL->glPushMatrix();

			m_pGL->glTranslated(0., 0., -50.);

			ublas::matrix<double> mat0 = rotation_matrix_3d_z(0.);
			ublas::matrix<double> mat1 = rotation_matrix_3d_z(M_PI/2.);

			math::quaternion<double> quat0 = rot3_to_quat(mat0);
			math::quaternion<double> quat1 = rot3_to_quat(mat1);
			math::quaternion<double> quat = slerp(quat0, quat1, m_dTick);
			//math::quaternion<double> quat = lerp(quat0, quat1, m_dTick);

			ublas::matrix<double> mat = quat_to_rot3(quat);
			GLdouble glmat[16]; to_array(mat, glmat);
			m_pGL->glMultMatrixd(glmat);

			m_pGL->glColor3f(0.,0.,0.);
			m_pGL->glBegin(GL_LINE_STRIP);
				m_pGL->glVertex3f(-10., 0., 0.);
				m_pGL->glVertex3f(10., 0., 0.);
				m_pGL->glVertex3f(0., 10., 0.);
				m_pGL->glVertex3f(-10., 0., 0.);
			m_pGL->glEnd();

			m_pGL->glPopMatrix();
			swapBuffers();
		}
};


int main(int argc, char **argv)
{
	QApplication a(argc, argv);

	QDialog dlg;
	GlWidget *pGl = new GlWidget(&dlg);
	dlg.resize(800, 600);

	QGridLayout *pGrid = new QGridLayout(&dlg);
	pGrid->addWidget(pGl, 0,0,1,1);
	dlg.exec();

	return 0;
}

#include "tst_quat2.moc"
