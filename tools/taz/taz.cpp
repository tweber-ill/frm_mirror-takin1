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


	std::vector<QLineEdit*> vecEdits{editA, editB, editC, editAlpha, editBeta, editGamma,
									editScatX0, editScatX1, editScatX2,
									editScatY0, editScatY1, editScatY2};
	for(QLineEdit* pEdit : vecEdits)
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(CalcPeaks()));

	CalcPeaks();
}

TazDlg::~TazDlg()
{}


void TazDlg::CalcPeaks()
{
	double a = editA->text().toDouble();
	double b = editB->text().toDouble();
	double c = editC->text().toDouble();

	double alpha = editAlpha->text().toDouble();
	double beta = editBeta->text().toDouble();
	double gamma = editGamma->text().toDouble();

	double plane_x[] = {editScatX0->text().toDouble(),
						editScatX1->text().toDouble(),
						editScatX2->text().toDouble()};
	double plane_y[] = {editScatY0->text().toDouble(),
						editScatY1->text().toDouble(),
						editScatY2->text().toDouble()};

	Lattice lattice(a,b,c, alpha,beta,gamma);
	//lattice.GetRecip();
}


#include "taz.moc"
