/*
 * 3D Ellipsoid Dialog
 * @author Tobias Weber
 * @date may-2013, 29-apr-2014
 * @license GPLv2
 */

#include "EllipseDlg3D.h"
#include <QGridLayout>
//#include <QtGui/QSplitter>

EllipseDlg3D::EllipseDlg3D(QWidget* pParent, QSettings* pSett)
	: QDialog(pParent), m_pSettings(pSett)
{
	setWindowFlags(Qt::Tool);
	setWindowTitle("Resolution Ellipsoids");
	setSizeGripEnabled(1);
	if(m_pSettings)
	{
		QFont font;
		if(m_pSettings->contains("main/font_gen") && font.fromString(m_pSettings->value("main/font_gen", "").toString()))
			setFont(font);
	}

	PlotGl* pPlotLeft = new PlotGl(this, m_pSettings);
	pPlotLeft->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_pPlots.push_back(pPlotLeft);

	PlotGl* pPlotRight = new PlotGl(this, m_pSettings);
	pPlotRight->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_pPlots.push_back(pPlotRight);

	m_pComboCoord = new QComboBox(this);
	m_pComboCoord->insertItem(0, "(Q perpendicular, Q parallel, Q up) System (1/A)");
	m_pComboCoord->insertItem(1, "Crystal (hkl) System (rlu)");
	m_pComboCoord->insertItem(2, "Scattering Plane System (rlu)");

	QGridLayout *pgridLayout = new QGridLayout(this);
	pgridLayout->setContentsMargins(4, 4, 4, 4);
	pgridLayout->addWidget(pPlotLeft, 0, 0, 1, 1);
	pgridLayout->addWidget(pPlotRight, 0, 1, 1, 1);
	pgridLayout->addWidget(m_pComboCoord, 1, 0, 1, 1);

	/*QSplitter *pSplitter = new QSplitter(this);
	pSplitter->setOrientation(Qt::Horizontal);
	pSplitter->addWidget(pPlotLeft);
	pSplitter->addWidget(pPlotRight);
	pgridLayout->addWidget(pSplitter, 0, 0, 1, 1);*/

	m_elliProj.resize(2);
	m_elliSlice.resize(2);

	resize(640,480);


	QObject::connect(m_pComboCoord, SIGNAL(currentIndexChanged(int)), this, SLOT(Calc()));


	if(m_pSettings && m_pSettings->contains("reso/ellipsoid3d_geo"))
		restoreGeometry(m_pSettings->value("reso/ellipsoid3d_geo").toByteArray());

	for(unsigned int i=0; i<m_pPlots.size(); ++i)
		m_pPlots[i]->SetEnabled(1);
}

EllipseDlg3D::~EllipseDlg3D()
{
	for(PlotGl* pPlot : m_pPlots)
		delete pPlot;
	m_pPlots.clear();
}

void EllipseDlg3D::hideEvent(QHideEvent *event)
{
	for(unsigned int i=0; i<m_pPlots.size(); ++i)
		m_pPlots[i]->SetEnabled(0);

	if(m_pSettings)
		m_pSettings->setValue("reso/ellipsoid3d_geo", saveGeometry());
}

void EllipseDlg3D::showEvent(QShowEvent *event)
{
	QDialog::showEvent(event);

	for(unsigned int i=0; i<m_pPlots.size(); ++i)
		if(m_pPlots[i])
			m_pPlots[i]->SetEnabled(1);
}


ublas::vector<t_real_reso>
EllipseDlg3D::ProjRotatedVec(const ublas::matrix<t_real_reso>& rot,
	const ublas::vector<t_real_reso>& vec)
{
	ublas::vector<t_real_reso> vecCoord(3);

	ublas::vector<t_real_reso> x = ublas::zero_vector<t_real_reso>(3);
	x[0] = 1.;
	ublas::vector<t_real_reso> y = ublas::zero_vector<t_real_reso>(3);
	y[1] = 1.;
	ublas::vector<t_real_reso> z = ublas::zero_vector<t_real_reso>(3);
	z[2] = 1.;

	const ublas::vector<t_real_reso> vecrot_x = ublas::prod(rot, x*vec[0]);
	const ublas::vector<t_real_reso> vecrot_y = ublas::prod(rot, y*vec[1]);
	const ublas::vector<t_real_reso> vecrot_z = ublas::prod(rot, z*vec[2]);

	vecCoord[0] = std::fabs(vecrot_x[0]) + std::fabs(vecrot_y[0]) + std::fabs(vecrot_z[0]);
	vecCoord[1] = std::fabs(vecrot_x[1]) + std::fabs(vecrot_y[1]) + std::fabs(vecrot_z[1]);
	vecCoord[2] = std::fabs(vecrot_x[2]) + std::fabs(vecrot_y[2]) + std::fabs(vecrot_z[2]);

	return vecCoord;
}


void EllipseDlg3D::Calc()
{
	const EllipseCoordSys coord = static_cast<EllipseCoordSys>(m_pComboCoord->currentIndex());

	const ublas::matrix<t_real_reso> *pReso = nullptr;
	const ublas::vector<t_real_reso> *pQavg = nullptr;

	switch(coord)
	{
		case EllipseCoordSys::Q_AVG:		// Q|| Qperp system in 1/A
			pReso = &m_reso; pQavg = &m_Q_avg;
			break;
		case EllipseCoordSys::RLU:			// rlu system
			pReso = &m_resoHKL; pQavg = &m_Q_avgHKL;
			break;
		case EllipseCoordSys::RLU_ORIENT:	// rlu system
			pReso = &m_resoOrient; pQavg = &m_Q_avgOrient;
			break;
		default:
			tl::log_err("Unknown coordinate system selected."); return;
	}


	int iAlgo = m_iAlgo;
	const ublas::matrix<t_real_reso>& reso = *pReso;
	const ublas::vector<t_real_reso>& _Q_avg = *pQavg;


	const int iX[] = {0, 0};
	const int iY[] = {1, 1};
	const int iZ[] = {3, 2};
	const int iIntOrRem[] = {2, 3};

	//Xml xmlparams;
	//bool bXMLLoaded = xmlparams.Load("res/res.conf");
	bool bCenterOn0 = 1; //xmlparams.Query<bool>("/res/center_around_origin", 0);

	ublas::vector<t_real_reso> Q_avg = _Q_avg;
	if(bCenterOn0)
		Q_avg = ublas::zero_vector<t_real_reso>(Q_avg.size());

	for(unsigned int i=0; i<m_pPlots.size(); ++i)
	{
		m_elliProj[i] = ::calc_res_ellipsoid(reso, Q_avg, iX[i], iY[i], iZ[i], iIntOrRem[i], -1);
		m_elliSlice[i] = ::calc_res_ellipsoid(reso, Q_avg, iX[i], iY[i], iZ[i], -1, iIntOrRem[i]);

		ublas::vector<t_real_reso> vecWProj(3), vecWSlice(3);
		ublas::vector<t_real_reso> vecOffsProj(3), vecOffsSlice(3);

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

		/*if(i==1)
		{
			std::cout << "widths: " << vecWProj << std::endl;
			std::cout << "offs: " << vecOffsProj << std::endl;
			std::cout << "rot: " << m_elliProj[i].rot << std::endl;
		}*/
		m_pPlots[i]->PlotEllipsoid(vecWProj, vecOffsProj, m_elliProj[i].rot, 1);
		m_pPlots[i]->PlotEllipsoid(vecWSlice, vecOffsSlice, m_elliSlice[i].rot, 0);

		m_pPlots[i]->SetObjectUseLOD(1, 0);
		m_pPlots[i]->SetObjectUseLOD(0, 0);

		m_pPlots[i]->SetMinMax(ProjRotatedVec(m_elliProj[i].rot, vecWProj), &vecOffsProj);

		const std::string& strX = ellipse_labels(iX[i], coord);
		const std::string& strY = ellipse_labels(iY[i], coord);
		const std::string& strZ = ellipse_labels(iZ[i], coord);
		m_pPlots[i]->SetLabels(strX.c_str(), strY.c_str(), strZ.c_str());
	}
}

void EllipseDlg3D::SetParams(const ublas::matrix<t_real_reso>& reso, const ublas::vector<t_real_reso>& Q_avg,
	const ublas::matrix<t_real_reso>& resoHKL, const ublas::vector<t_real_reso>& Q_avgHKL,
	const ublas::matrix<t_real_reso>& resoOrient, const ublas::vector<t_real_reso>& Q_avgOrient,
	int iAlgo)
{
	m_reso = reso;
	m_resoHKL = resoHKL;
	m_resoOrient = resoOrient;
	m_Q_avg = Q_avg;
	m_Q_avgHKL = Q_avgHKL;
	m_Q_avgOrient = Q_avgOrient;
	m_iAlgo = iAlgo;

	Calc();
}

#include "EllipseDlg3D.moc"
