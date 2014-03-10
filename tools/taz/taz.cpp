/*
 * Scattering Triangle Tool
 * @author tweber
 * @date feb-2014
 */

#include "taz.h"

#include <iostream>
#include <algorithm>
#include <vector>

#include <QtGui/QApplication>

#include "helper/lattice.h"
#include "helper/spec_char.h"


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

	QObject::connect(editPlaneTolerance, SIGNAL(textEdited(const QString&)), this, SLOT(ChangedTolerance()));
	QObject::connect(btn3D, SIGNAL(clicked()), this, SLOT(Show3D()));

	UpdateDs();
	CalcPeaks();

	m_sceneRecip.emitUpdate();
}

TazDlg::~TazDlg()
{
	if(m_pRecip3d)
	{
		delete m_pRecip3d;
		m_pRecip3d = 0;
	}
}


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

void TazDlg::ChangedTolerance()
{
	if(!m_sceneRecip.GetTriangle())
		return;

	double dTol = editPlaneTolerance->text().toDouble();
	m_sceneRecip.GetTriangle()->SetPlaneDistTolerance(dTol);
	if(m_pRecip3d)
		m_pRecip3d->SetPlaneDistTolerance(dTol);

	CalcPeaks();
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
	if(!m_sceneRecip.GetTriangle())
		return;

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


	double dX0 = editScatX0->text().toDouble();
	double dX1 = editScatX1->text().toDouble();
	double dX2 = editScatX2->text().toDouble();
	ublas::vector<double> vecPlaneX = dX0*recip_unrot.GetVec(0) +
									dX1*recip_unrot.GetVec(1) +
									dX2*recip_unrot.GetVec(2);

	double dY0 = editScatY0->text().toDouble();
	double dY1 = editScatY1->text().toDouble();
	double dY2 = editScatY2->text().toDouble();
	ublas::vector<double> vecPlaneY = dY0*recip_unrot.GetVec(0) +
									dY1*recip_unrot.GetVec(1) +
									dY2*recip_unrot.GetVec(2);


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

	const std::wstring& strAA = ::get_spec_char_utf16("AA");
	double dVol = lattice.GetVol();
	std::wostringstream ostrSample;
	ostrSample.precision(4);
	ostrSample << "Sample - ";
	ostrSample << "Unit Cell Volume: ";
	ostrSample << "Real: " << dVol << " " << strAA << "^3";
	ostrSample << ", Reciprocal: " << (1./dVol) << " (1/" << strAA << ")^3";
	groupSample->setTitle(QString::fromWCharArray(ostrSample.str().c_str()));

	m_sceneRecip.GetTriangle()->CalcPeaks(lattice, recip, recip_unrot, plane);
	m_sceneRecip.emitUpdate();

	if(m_pRecip3d)
		m_pRecip3d->CalcPeaks(lattice, recip, recip_unrot, plane);
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

void TazDlg::Show3D()
{
	if(!m_pRecip3d)
	{
		m_pRecip3d = new Recip3DDlg(this);

		double dTol = editPlaneTolerance->text().toDouble();
		m_pRecip3d->SetPlaneDistTolerance(dTol);
	}

	if(!m_pRecip3d->isVisible())
		m_pRecip3d->show();
	m_pRecip3d->activateWindow();

	CalcPeaks();
}

#include "taz.moc"
