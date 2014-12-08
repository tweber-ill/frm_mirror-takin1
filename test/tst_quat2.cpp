// moc-qt4 tst_quat2.cpp > tst_quat2.moc
// gcc -o tst_quat2 tst_quat2.cpp -lstdc++ -lm -lQtCore -lQtGui -lQtOpenGL -lGL -lGLU -std=c++11

#include <iostream>

#include "../helper/linalg.h"
#include "../helper/quat.h"

#include <QtOpenGL/QGLWidget>
#include <QtGui/QApplication>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtCore/QTimer>

#include <GL/glu.h>


class GlWidget : public QGLWidget
{ Q_OBJECT
	protected:
		double m_dTick = 0.;
		QTimer m_timer;

	protected slots:
		void tick()
		{
			m_dTick += 0.02;
			update();
		}

	public:
		GlWidget(QWidget* pParent) : QGLWidget(pParent), m_timer(this)
		{
			QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
			m_timer.start(5);
		}
		virtual ~GlWidget()
		{
			m_timer.stop();
		}

	protected:
		virtual void initializeGL() override
		{
			glClearColor(1.,1.,1.,0.);
			glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
			glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
			glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
			glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
		}

		virtual void resizeGL(int w, int h) override
		{
			if(w<0) w=1; if(h<0) h=1;
			glViewport(0,0,w,h);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(45., double(w)/double(h), 0.1, 100.);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
		}

		virtual void paintGL() override
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glPushMatrix();

			glTranslated(0., 0., -50.);

			ublas::matrix<double> mat0 = rotation_matrix_3d_z(0.);
			ublas::matrix<double> mat1 = rotation_matrix_3d_z(M_PI/2.);

			math::quaternion<double> quat0 = rot3_to_quat(mat0);
			math::quaternion<double> quat1 = rot3_to_quat(mat1);
			math::quaternion<double> quat = slerp(quat0, quat1, m_dTick);
			//math::quaternion<double> quat = lerp(quat0, quat1, m_dTick);

			ublas::matrix<double> mat = quat_to_rot3(quat);

			GLdouble glmat[] =
			{
				mat(0,0), mat(0,1), mat(0,2), 0,
				mat(1,0), mat(1,1), mat(1,2), 0,
				mat(2,0), mat(2,1), mat(2,2), 0,
				0,               0,        0, 1
			};
			glMultMatrixd(glmat);

			glColor3f(0.,0.,0.);
			glBegin(GL_LINE_STRIP);
				glVertex3f(-10., 0., 0.);
				glVertex3f(10., 0., 0.);
				glVertex3f(0., 10., 0.);
				glVertex3f(-10., 0., 0.);
			glEnd();

			glPopMatrix();
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
