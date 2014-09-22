/*
 * 3D Ellipsoid Dialog
 * @author Tobias Weber
 * @date may-2013, 29-apr-2014
 */

#include "EllipseDlg3D.h"
#include <QtGui/QGridLayout>

EllipseDlg3D::EllipseDlg3D(QWidget* pParent, QSettings* pSett)
		: QDialog(pParent), m_pSettings(pSett)
{
	setWindowFlags(Qt::Tool);
	setWindowTitle("Resolution Ellipsoids");

	QGridLayout *gridLayout = new QGridLayout(this);

	unsigned int iPos0[] = {0,0,1,1};
	unsigned int iPos1[] = {0,1,0,1};
	for(unsigned int i=0; i<2; ++i)
	{
		PlotGl* pPlot = new PlotGl(this);
		pPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		gridLayout->addWidget(pPlot, iPos0[i], iPos1[i], 1, 1);

		m_pPlots.push_back(pPlot);
	}
	m_elliProj.resize(2);
	m_elliSlice.resize(2);

	resize(640,480);
}

EllipseDlg3D::~EllipseDlg3D()
{
	for(PlotGl* pPlot : m_pPlots)
		delete pPlot;
	m_pPlots.clear();
}

void EllipseDlg3D::hideEvent (QHideEvent *event)
{
	if(m_pSettings)
		m_pSettings->setValue("reso/ellipsoid3d_geo", saveGeometry());
}
void EllipseDlg3D::showEvent(QShowEvent *event)
{
	if(m_pSettings && m_pSettings->contains("reso/ellipsoid3d_geo"))
		restoreGeometry(m_pSettings->value("reso/ellipsoid3d_geo").toByteArray());
}


ublas::vector<double>
EllipseDlg3D::ProjRotatedVec(const ublas::matrix<double>& rot,
														const ublas::vector<double>& vec)
{
	ublas::vector<double> vecCoord(3);

	ublas::vector<double> x = ublas::zero_vector<double>(3);
	x[0] = 1.;
	ublas::vector<double> y = ublas::zero_vector<double>(3);
	y[1] = 1.;
	ublas::vector<double> z = ublas::zero_vector<double>(3);
	z[2] = 1.;

	const ublas::vector<double> vecrot_x = ublas::prod(rot, x*vec[0]);
	const ublas::vector<double> vecrot_y = ublas::prod(rot, y*vec[1]);
	const ublas::vector<double> vecrot_z = ublas::prod(rot, z*vec[2]);

	vecCoord[0] = std::fabs(vecrot_x[0]) + std::fabs(vecrot_y[0]) + std::fabs(vecrot_z[0]);
	vecCoord[1] = std::fabs(vecrot_x[1]) + std::fabs(vecrot_y[1]) + std::fabs(vecrot_z[1]);
	vecCoord[2] = std::fabs(vecrot_x[2]) + std::fabs(vecrot_y[2]) + std::fabs(vecrot_z[2]);

	return vecCoord;
}

void EllipseDlg3D::SetParams(const ublas::matrix<double>& reso, const ublas::vector<double>& _Q_avg)
{
	const int iX[] = {0, 0};
	const int iY[] = {1, 1};
	const int iZ[] = {3, 2};
	const int iIntOrRem[] = {2, 3};

	//Xml xmlparams;
	//bool bXMLLoaded = xmlparams.Load("res/res.conf");
	bool bCenterOn0 = 1; //xmlparams.Query<bool>("/res/center_around_origin", 0);

	ublas::vector<double> Q_avg = _Q_avg;
	if(bCenterOn0)
		Q_avg = ublas::zero_vector<double>(Q_avg.size());

	for(unsigned int i=0; i<m_pPlots.size(); ++i)
	{
		m_elliProj[i] = ::calc_res_ellipsoid(reso, Q_avg, iX[i], iY[i], iZ[i], iIntOrRem[i], -1);
		m_elliSlice[i] = ::calc_res_ellipsoid(reso, Q_avg, iX[i], iY[i], iZ[i], -1, iIntOrRem[i]);

		ublas::vector<double> vecWProj(3), vecWSlice(3);
		ublas::vector<double> vecOffsProj(3), vecOffsSlice(3);

		vecWProj[0] = m_elliProj[i].x_hwhm;
		vecWProj[1] = m_elliProj[i].y_hwhm;
		vecWProj[2] = m_elliProj[i].z_hwhm;

		vecWSlice[0] = m_elliSlice[i].x_hwhm;
		vecWSlice[1] = m_elliSlice[i].y_hwhm;
		vecWSlice[2] = m_elliSlice[i].z_hwhm;

		vecOffsProj[0] = m_elliProj[i].x_offs;
		vecOffsProj[1] = m_elliProj[i].y_offs;
		vecOffsProj[2] = m_elliProj[i].z_offs;

		vecOffsSlice[0] = m_elliSlice[i].x_offs;
		vecOffsSlice[1] = m_elliSlice[i].y_offs;
		vecOffsSlice[2] = m_elliSlice[i].z_offs;

		m_pPlots[i]->PlotEllipsoid(vecWProj, vecOffsProj, m_elliProj[i].rot, 1);
		m_pPlots[i]->PlotEllipsoid(vecWSlice, vecOffsSlice, m_elliSlice[i].rot, 0);

		m_pPlots[i]->SetMinMax(ProjRotatedVec(m_elliProj[i].rot, vecWProj), &vecOffsProj);
		m_pPlots[i]->SetLabels(m_elliProj[i].x_lab.c_str(), m_elliProj[i].y_lab.c_str(), m_elliProj[i].z_lab.c_str());
	}
}

#include "EllipseDlg3D.moc"
