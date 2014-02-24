/*
 * Scattering Triangle
 * @author tweber
 * @date feb-2014
 */

#include "taz.h"

#include <iostream>
#include <algorithm>
#include <vector>

#include <QtGui/QApplication>

#include "helper/lattice.h"


TazDlg::TazDlg(QWidget* pParent) : QDialog(pParent)
{
	this->setupUi(this);

	gfxRecip->setRenderHints(QPainter::Antialiasing);
	gfxRecip->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	gfxRecip->setDragMode(QGraphicsView::ScrollHandDrag);
	gfxRecip->setScene(&m_sceneRecip);

	gfxReal->setRenderHints(QPainter::Antialiasing);
	gfxReal->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	gfxReal->setDragMode(QGraphicsView::ScrollHandDrag);
	gfxReal->setScene(&m_sceneReal);

	QObject::connect(&m_sceneRecip, SIGNAL(triangleChanged(const TriangleOptions&)),
					&m_sceneReal, SLOT(triangleChanged(const TriangleOptions&)));
	QObject::connect(&m_sceneReal, SIGNAL(tasChanged(const TriangleOptions&)),
					&m_sceneRecip, SLOT(tasChanged(const TriangleOptions&)));


	std::vector<QLineEdit*> vecEditDs{editMonoD, editAnaD};
	for(QLineEdit* pEdit : vecEditDs)
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(UpdateDs()));

	std::vector<QLineEdit*> vecEdits{editA, editB, editC, editAlpha, editBeta, editGamma,
									editScatX0, editScatX1, editScatX2,
									editScatY0, editScatY1, editScatY2};
	for(QLineEdit* pEdit : vecEdits)
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(CalcPeaks()));

	UpdateDs();
	CalcPeaks();

	m_sceneRecip.emitUpdate();
}

TazDlg::~TazDlg()
{}


void TazDlg::UpdateDs()
{
	double dMonoD = editMonoD->text().toDouble();
	double dAnaD = editAnaD->text().toDouble();

	m_sceneRecip.SetDs(dMonoD, dAnaD);
}

std::ostream& operator<<(std::ostream& ostr, const Lattice& lat)
{
	ostr << "a = " << lat.GetA();
	ostr << ", b = " << lat.GetB();
	ostr << ", c = " << lat.GetC();
	ostr << ", alpha = " << lat.GetAlpha();
	ostr << ", beta = " << lat.GetBeta();
	ostr << ", gamma = " << lat.GetGamma();
	return ostr;
}

void TazDlg::CalcPeaks()
{
	double a = editA->text().toDouble();
	double b = editB->text().toDouble();
	double c = editC->text().toDouble();

	double alpha = editAlpha->text().toDouble()/180.*M_PI;
	double beta = editBeta->text().toDouble()/180.*M_PI;
	double gamma = editGamma->text().toDouble()/180.*M_PI;

	Lattice lattice(a,b,c, alpha,beta,gamma);



	ublas::vector<double> vecPlaneX(3);
	vecPlaneX[0] = editScatX0->text().toDouble();
	vecPlaneX[1] = editScatX1->text().toDouble();
	vecPlaneX[2] = editScatX2->text().toDouble();

	ublas::vector<double> vecPlaneY(3);
	vecPlaneY[0] = editScatY0->text().toDouble();
	vecPlaneY[1] = editScatY1->text().toDouble();
	vecPlaneY[2] = editScatY2->text().toDouble();

	ublas::vector<double> vecX0 = ublas::zero_vector<double>(3);
	Plane<double> plane(vecX0, vecPlaneX, vecPlaneY);


	//Lattice recip = lattice.GetRecip();
	//std::cout << "Lattice: " << lattice << "\nReciprocal: " << recip << "\n" << std::endl;

	m_sceneRecip.CalcPeaks(lattice, plane);
}


#include "taz.moc"
