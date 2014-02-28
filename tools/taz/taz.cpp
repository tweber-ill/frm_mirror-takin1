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


static QString dtoqstr(double dVal, unsigned int iPrec=3)
{
	std::ostringstream ostr;
	ostr.precision(iPrec);
	ostr << dVal;
	return QString(ostr.str().c_str());
}

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


	std::vector<QDoubleSpinBox*> vecSpins{spinRotPhi, spinRotTheta, spinRotPsi};
	for(QDoubleSpinBox* pSpin : vecSpins)
		QObject::connect(pSpin, SIGNAL(valueChanged(double)), this, SLOT(CalcPeaks()));

	std::vector<QLineEdit*> vecEditsRecip{editARecip, editBRecip, editCRecip,
										editAlphaRecip, editBetaRecip, editGammaRecip};
	for(QLineEdit* pEdit : vecEditsRecip)
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(CalcPeaksRecip()));


	QObject::connect(checkSenseM, SIGNAL(stateChanged(int)), this, SLOT(UpdateMonoSense()));
	QObject::connect(checkSenseS, SIGNAL(stateChanged(int)), this, SLOT(UpdateSampleSense()));
	QObject::connect(checkSenseA, SIGNAL(stateChanged(int)), this, SLOT(UpdateAnaSense()));


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

void TazDlg::CalcPeaksRecip()
{
	double a = editARecip->text().toDouble();
	double b = editBRecip->text().toDouble();
	double c = editCRecip->text().toDouble();

	double alpha = editAlphaRecip->text().toDouble()/180.*M_PI;
	double beta = editBetaRecip->text().toDouble()/180.*M_PI;
	double gamma = editGammaRecip->text().toDouble()/180.*M_PI;

	Lattice lattice(a,b,c, alpha,beta,gamma);
	Lattice recip = lattice.GetRecip();

	editA->setText(dtoqstr(recip.GetA()));
	editB->setText(dtoqstr(recip.GetB()));
	editC->setText(dtoqstr(recip.GetC()));
	editAlpha->setText(dtoqstr(recip.GetAlpha()/M_PI*180.));
	editBeta->setText(dtoqstr(recip.GetBeta()/M_PI*180.));
	editGamma->setText(dtoqstr(recip.GetGamma()/M_PI*180.));

	m_bUpdateRecipEdits = 0;
	CalcPeaks();
	m_bUpdateRecipEdits = 1;
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
	Lattice recip_unrot = lattice.GetRecip();

	double dPhi = spinRotPhi->value() / 180. * M_PI;
	double dTheta = spinRotTheta->value() / 180. * M_PI;
	double dPsi = spinRotPsi->value() / 180. * M_PI;
	lattice.RotateEuler(dPhi, dTheta, dPsi);

	Lattice recip = lattice.GetRecip();



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


	if(m_bUpdateRecipEdits)
	{
		editARecip->setText(dtoqstr(recip.GetA()));
		editBRecip->setText(dtoqstr(recip.GetB()));
		editCRecip->setText(dtoqstr(recip.GetC()));
		editAlphaRecip->setText(dtoqstr(recip.GetAlpha()/M_PI*180.));
		editBetaRecip->setText(dtoqstr(recip.GetBeta()/M_PI*180.));
		editGammaRecip->setText(dtoqstr(recip.GetGamma()/M_PI*180.));
	}

	std::wostringstream ostrSample;
	ostrSample.precision(4);
	ostrSample << "Sample - ";
	ostrSample << "Unit Cell Volume: ";
	ostrSample << "Real: " << lattice.GetVol() << L" \x212b^3";
	ostrSample << ", Reciprocal: " << recip.GetVol() << L" (1/\x212b)^3";
	groupSample->setTitle(QString::fromWCharArray(ostrSample.str().c_str()));

	m_sceneRecip.CalcPeaks(lattice, recip, recip_unrot, plane);
	m_sceneRecip.emitUpdate();
}

void TazDlg::UpdateSampleSense()
{
	m_sceneRecip.SetSampleSense(checkSenseS->isChecked());
}

void TazDlg::UpdateMonoSense()
{
	m_sceneRecip.SetMonoSense(checkSenseM->isChecked());
}

void TazDlg::UpdateAnaSense()
{
	m_sceneRecip.SetAnaSense(checkSenseA->isChecked());
}


#include "taz.moc"
