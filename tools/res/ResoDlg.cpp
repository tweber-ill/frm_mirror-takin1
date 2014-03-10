/*
 * mieze-tool
 * @author tweber
 * @date 01-may-2013
 */

#include "ResoDlg.h"
#include <iostream>
#include <map>

#include "../../main/settings.h"
#include "helper/string.h"
#include "helper/misc.h"
#include "helper/xml.h"
#include "helper/math.h"
//#include "../data/export.h"

#include <QtGui/QPainter>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QGridLayout>


EllipseDlg::EllipseDlg(QWidget* pParent) : QDialog(pParent), m_pLabelStatus(0)
{
	setWindowFlags(Qt::Tool);
	setWindowTitle("Resolution Ellipses");

	QGridLayout *gridLayout = new QGridLayout(this);

	unsigned int iPos0[] = {0,0,1,1};
	unsigned int iPos1[] = {0,1,0,1};
	for(unsigned int i=0; i<4; ++i)
	{
		Plot* pPlot = new Plot(this);
		pPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		gridLayout->addWidget(pPlot, iPos0[i], iPos1[i], 1, 1);
		QObject::connect(pPlot, SIGNAL(SetStatusMsg(const char*, int)), this, SLOT(SetStatusMsg(const char*, int)));

		m_pPlots.push_back(pPlot);
	}
	m_elliProj.resize(4);
	m_elliSlice.resize(4);

	m_pLabelStatus = new QLabel(this);
	m_pLabelStatus->setFrameShape(QFrame::Panel);
	m_pLabelStatus->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(m_pLabelStatus, 2, 0, 1, 2);

	resize(320,320);
}

EllipseDlg::~EllipseDlg()
{
	for(Plot* pPlot : m_pPlots)
		delete pPlot;
	m_pPlots.clear();
}

void EllipseDlg::paintEvent (QPaintEvent *pEvent)
{}

void EllipseDlg::hideEvent (QHideEvent *event)
{
	/* was for debug
	export_py("0.py", m_pPlots[0]);
	export_py("1.py", m_pPlots[1]);
	export_py("2.py", m_pPlots[2]);
	export_py("3.py", m_pPlots[3]);
	*/

	Settings::GetGlobals()->setValue("reso/wnd_ellipses_geo", saveGeometry());
}
void EllipseDlg::showEvent(QShowEvent *event)
{
	restoreGeometry(Settings::GetGlobals()->value("reso/wnd_ellipses_geo").toByteArray());
}

void EllipseDlg::SetStatusMsg(const char* pcMsg, int iPos)
{
	if(!m_pLabelStatus) return;

	if(iPos==2 && m_pLabelStatus->text()!=QString(pcMsg))
		m_pLabelStatus->setText(pcMsg);
}

void EllipseDlg::SetParams(const PopParams& pop, const CNResults& res)
{
	int iParams[2][4][5] =
	{
		{
			{0, 3, 1, 2, -1},
			{1, 3, 0, 2, -1},
			{2, 3, 0, 1, -1},
			{0, 1, 3, 2, -1}
		},
		{
			{0, 3, -1, 2, 1},
			{1, 3, -1, 2, 0},
			{2, 3, -1, 1, 0},
			{0, 1, -1, 2, 3}
		}
	};


	Xml xmlparams;
	bool bXMLLoaded = xmlparams.Load("res/res.conf");
	bool bCenterOn0 = xmlparams.Query<bool>("/res/center_around_origin", 0);

	for(unsigned int iEll=0; iEll<4; ++iEll)
	{
		if(bXMLLoaded)
		{
			std::ostringstream ostrCfg;
			ostrCfg << "/res/ellipse_2d_" << iEll;

			std::string strProjPath = ostrCfg.str() + "_proj";
			std::string strSlicePath = ostrCfg.str() + "_slice";

			std::string strProj = xmlparams.QueryString(strProjPath.c_str(), "");
			std::string strSlice = xmlparams.QueryString(strSlicePath.c_str(), "");

			const std::string* strEllis[] = {&strProj, &strSlice};

			for(unsigned int iWhichEll=0; iWhichEll<2; ++iWhichEll)
			{
				if(*strEllis[iWhichEll] != "")
				{
					std::vector<int> vecIdx;
					::get_tokens(*strEllis[iWhichEll], std::string(","), vecIdx);

					if(vecIdx.size() == 5)
					{
						for(unsigned int iParam=0; iParam<5; ++iParam)
							iParams[iWhichEll][iEll][iParam] = vecIdx[iParam];
					}
					else
					{
						std::cerr << "Error in res.conf: Wrong size of parameters for "
								 << strProj << "." << std::endl;
					}
				}
			}
		}

		ublas::vector<double> Q_avg = res.Q_avg;
		if(bCenterOn0)
			Q_avg = ublas::zero_vector<double>(Q_avg.size());

		m_elliProj[iEll] = ::calc_res_ellipse(res.reso, Q_avg, iParams[0][iEll][0], iParams[0][iEll][1], iParams[0][iEll][2], iParams[0][iEll][3], iParams[0][iEll][4]);
		m_elliSlice[iEll] = ::calc_res_ellipse(res.reso, Q_avg, iParams[1][iEll][0], iParams[1][iEll][1], iParams[1][iEll][2], iParams[1][iEll][3], iParams[1][iEll][4]);
	}


	for(unsigned int i=0; i<m_pPlots.size(); ++i)
	{
		m_pPlots[i]->SetLabels(m_elliProj[i].x_lab.c_str(), m_elliProj[i].y_lab.c_str());
		std::ostringstream ostrSlope;
		ostrSlope.precision(4);

		// We do not know which principal axis corresponds to which axis,
		// so the angle can be rotated by PI/2
		// TODO: correct the angle which is printed here
		ostrSlope << "Slope (proj): " << std::tan(-m_elliProj[i].phi)
				  << ", Angle (proj): " << m_elliProj[i].phi;

		m_pPlots[i]->SetTitle(ostrSlope.str().c_str());
		m_pPlots[i]->plot_param(m_elliProj[i],1);
		m_pPlots[i]->plot_param(m_elliSlice[i],0);
	}

	for(Plot* pPlot : m_pPlots)
		pPlot->RefreshPlot();
}

//----------------------------------------------------------------------------------------


EllipseDlg3D::EllipseDlg3D(QWidget* pParent) : QDialog(pParent)
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
	Settings::GetGlobals()->setValue("reso/wnd_ellipses3d_geo", saveGeometry());
}
void EllipseDlg3D::showEvent(QShowEvent *event)
{
	restoreGeometry(Settings::GetGlobals()->value("reso/wnd_ellipses3d_geo").toByteArray());
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

void EllipseDlg3D::SetParams(const PopParams& pop, const CNResults& res)
{
	const int iX[] = {0, 0};
	const int iY[] = {1, 1};
	const int iZ[] = {3, 2};
	const int iIntOrRem[] = {2, 3};

	Xml xmlparams;
	bool bXMLLoaded = xmlparams.Load("res/res.conf");
	bool bCenterOn0 = xmlparams.Query<bool>("/res/center_around_origin", 0);

	ublas::vector<double> Q_avg = res.Q_avg;
	if(bCenterOn0)
		Q_avg = ublas::zero_vector<double>(Q_avg.size());

	for(unsigned int i=0; i<m_pPlots.size(); ++i)
	{
		m_elliProj[i] = ::calc_res_ellipsoid(res.reso, Q_avg, iX[i], iY[i], iZ[i], iIntOrRem[i], -1);
		m_elliSlice[i] = ::calc_res_ellipsoid(res.reso, Q_avg, iX[i], iY[i], iZ[i], -1, iIntOrRem[i]);

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

//----------------------------------------------------------------------------------------



InstLayoutDlg::InstLayoutDlg(QWidget* pParent) : QDialog(pParent)
{
	setWindowFlags(Qt::Tool);
	setWindowTitle("Instrument Layout");
	resize(320,320);
}

InstLayoutDlg::~InstLayoutDlg()
{}

void InstLayoutDlg::hideEvent(QHideEvent *event)
{
	Settings::GetGlobals()->setValue("reso/wnd_layout_geo", saveGeometry());
}

void InstLayoutDlg::showEvent(QShowEvent *event)
{
	restoreGeometry(Settings::GetGlobals()->value("reso/wnd_layout_geo").toByteArray());
}

void InstLayoutDlg::paintEvent(QPaintEvent *pEvent)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);

	QSize size = this->size();
	painter.fillRect(0, 0, size.width(), size.height(), QColor::fromRgbF(1.,1.,1.,1.));

	QTransform T;
	T.translate(0, size.height());
	T.scale(double(size.width())/m_dWidth, -double(size.height())/m_dHeight);
	painter.setTransform(T);



	QPen penBlue(QColor::fromRgbF(0.,0.,1.,1.));
	penBlue.setWidthF(m_dWidth/150.);
	QPen penBlack(QColor::fromRgbF(0.,0.,0.,1.));


	painter.save();

	painter.setPen(penBlue);
	painter.drawLine(QLineF(m_dPosMonoX, 0, m_dPosMonoX, m_dPosMonoY));
	painter.setPen(penBlack);
	painter.translate(m_dPosMonoX,m_dPosMonoY);

	painter.rotate(m_dMono2Theta);
	painter.setPen(penBlue);
	painter.drawLine(QLineF(0, 0, 0, m_dDistMonoSample));
	painter.setPen(penBlack);
	painter.save();
	painter.rotate(-m_dMono2Theta+m_dMonoTheta);
	// Monochromator
	painter.fillRect(QRectF(-m_dMonoD/2.,-m_dMonoW/2.,m_dMonoD,m_dMonoW), QColor::fromRgbF(0.,0.,0.,1.));
	painter.restore();
	painter.translate(0, m_dDistMonoSample);

	painter.rotate(m_dSample2Theta);
	painter.setPen(penBlue);
	painter.drawLine(QLineF(0, 0, 0, m_dDistSampleAna));
	painter.setPen(penBlack);
	painter.save();
	painter.rotate(-m_dSample2Theta+m_dSampleTheta);
	// Sample
	painter.fillRect(QRectF(-m_dSamp_Q/2.,-m_dSamp_perpQ/2.,m_dSamp_Q,m_dSamp_perpQ), QColor::fromRgbF(0.,0.,0.,1.));
	painter.restore();
	painter.translate(0, m_dDistSampleAna);

	painter.rotate(m_dAna2Theta);
	painter.setPen(penBlue);
	painter.drawLine(QLineF(0, 0, 0, m_dDistAnaDet));
	painter.setPen(penBlack);
	painter.save();
	painter.rotate(-m_dAna2Theta+m_dAnaTheta);
	// Analyser
	painter.fillRect(QRectF(-m_dAnaD/2.,-m_dAnaW/2.,m_dAnaD,m_dAnaW), QColor::fromRgbF(0.,0.,0.,1.));
	painter.restore();
	painter.translate(0, m_dDistAnaDet);
	// Detector
	painter.fillRect(QRectF(-m_dDetW/2.,-m_dDetD/2.,m_dDetW, m_dDetD), QColor::fromRgbF(0.,0.,0.,1.));

	painter.restore();
}

void InstLayoutDlg::SetParams(const PopParams& pop, const CNResults& res)
{
	m_dAnaTheta = res.thetaa/units::si::radians / M_PI*180.;
	m_dMonoTheta = res.thetam/units::si::radians / M_PI*180.;
	m_dSampleTheta = res.thetas/units::si::radians / M_PI*180.;

	m_dAna2Theta = 2.*m_dAnaTheta;
	m_dMono2Theta = 2.*m_dMonoTheta;
	m_dSample2Theta = res.twotheta/units::si::radians / M_PI*180.;

	//std::cout << (res.angle_ki_Q/units::si::radians / M_PI*180.) << ", " << (res.angle_kf_Q/units::si::radians / M_PI*180.) << std::endl;

	m_dDistMonoSample = pop.dist_mono_sample / (units::si::meters*0.01);
	m_dDistSampleAna = pop.dist_sample_ana / (units::si::meters*0.01);
	m_dDistAnaDet = pop.dist_ana_det / (units::si::meters*0.01);

	m_dWidth = m_dDistMonoSample+m_dDistSampleAna+m_dDistAnaDet;
	m_dHeight = m_dDistMonoSample+m_dDistSampleAna+m_dDistAnaDet;

	m_dMonoW = m_dWidth/15.; //pop.mono_w / (units::si::meters*0.01);
	m_dMonoD = m_dWidth/75.; //pop.mono_thick / (units::si::meters*0.01);
	m_dAnaW = m_dWidth/15.; //pop.ana_w / (units::si::meters*0.01);
	m_dAnaD = m_dWidth/75.; //pop.ana_thick / (units::si::meters*0.01);
	m_dDetW = m_dWidth/15.; //pop.det_w / (units::si::meters*0.01);
	m_dDetD = m_dWidth/75.;

	m_dSamp_Q = m_dWidth/40.; //pop.sample_w_q / (units::si::meters*0.01);
	m_dSamp_perpQ = m_dWidth/40.; //pop.sample_w_perpq / (units::si::meters*0.01);

	if(pop.dmono_sense > 0.)
		m_dPosMonoX = m_dWidth;
	else
		m_dPosMonoX = m_dWidth/10.;

	m_dPosMonoY = m_dHeight/2.;

	m_dWidth += m_dWidth/10.;
	//m_dHeight += m_dPosMonoY;

	repaint();
}


//----------------------------------------------------------------------------------------



ScatterTriagDlg::ScatterTriagDlg(QWidget* pParent)
	: QDialog(pParent),
	  m_vec_ki(2), m_vec_kf(2), m_vec_Q(2)
{
	setWindowFlags(Qt::Tool);
	setWindowTitle("Scattering Triangle");
	resize(320,320);
}

ScatterTriagDlg::~ScatterTriagDlg()
{}

void ScatterTriagDlg::SetParams(const PopParams& pop, const CNResults& res)
{
	m_vec_ki[0] = 100.;
	m_vec_ki[1] = 0.;

	m_d2Theta = res.twotheta / units::si::radians;
	m_dKiQ = res.angle_ki_Q / units::si::radians;

	if(m_d2Theta<0.)	// neg. scattering sense
		m_dKiQ=-m_dKiQ;

	ublas::matrix<double> rot_kikf = rotation_matrix_2d(m_d2Theta);
	ublas::matrix<double> rot_kiQ = rotation_matrix_2d(M_PI-m_dKiQ);

	m_vec_kf = ublas::prod(rot_kikf, m_vec_ki) * (pop.kf/pop.ki);
	m_vec_Q = ublas::prod(rot_kiQ, m_vec_ki) * (pop.Q/pop.ki);

	//std::cout << m_vec_Q << std::endl;
	//std::cout << m_vec_kf-m_vec_ki << std::endl;
	repaint();
}

void ScatterTriagDlg::hideEvent(QHideEvent *event)
{
	Settings::GetGlobals()->setValue("reso/wnd_scatter_geo", saveGeometry());
}

void ScatterTriagDlg::showEvent(QShowEvent *event)
{
	restoreGeometry(Settings::GetGlobals()->value("reso/wnd_scatter_geo").toByteArray());
}

void ScatterTriagDlg::paintEvent(QPaintEvent *pEvent)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);

	QPen penBlack(QColor::fromRgbF(0.,0.,0.,1.));
	penBlack.setWidthF(1.5);
	painter.setPen(penBlack);

	QSize size = this->size();
	painter.fillRect(0, 0, size.width(), size.height(), QColor::fromRgbF(1.,1.,1.,1.));

	const double dWidth = 200.;
	const double dHeight = 200.;

	ublas::vector<double> vecOffs(2);
	vecOffs[0] = 50.;
	vecOffs[1] = 50.;
	if(m_d2Theta<0.)
		vecOffs[1] = dHeight-75.;

	QTransform T;
	T.translate(0, size.height());
	T.scale(double(size.width())/dWidth, -double(size.height())/dHeight);

	painter.setTransform(T);
	painter.drawLine(QLineF(vecOffs[0], vecOffs[1], vecOffs[0]+m_vec_ki[0], vecOffs[1]+m_vec_ki[1]));
	painter.drawLine(QLineF(vecOffs[0], vecOffs[1], vecOffs[0]+m_vec_kf[0], vecOffs[1]+m_vec_kf[1]));
	painter.drawLine(QLineF(vecOffs[0]+m_vec_ki[0], vecOffs[1]+m_vec_ki[1], vecOffs[0]+m_vec_ki[0]+m_vec_Q[0], vecOffs[1]+m_vec_ki[1]+m_vec_Q[1]));


	const double dFontSize = 11.;
	QFont font("Nimbus Sans L");
	font.setPixelSize(dFontSize);
	font.setBold(1);
	painter.setFont(font);

	painter.save();
	painter.translate(QPointF(vecOffs[0]+m_vec_ki[0]/2.-dFontSize/2., vecOffs[1]+m_vec_ki[1]/2.-dFontSize));
	painter.scale(1.,-1.);
	painter.drawText(QPointF(0., 0.), "ki");
	painter.restore();

	painter.save();
	painter.translate(QPointF(vecOffs[0]+m_vec_kf[0]/2., vecOffs[1]+m_vec_kf[1]/2.));
	painter.rotate(m_d2Theta/M_PI*180.);
	painter.translate(QPointF(-dFontSize/2., dFontSize/2.));
	painter.scale(1.,-1.);
	painter.drawText(QPointF(0., 0.), "kf");
	painter.restore();

	painter.save();
	painter.translate(QPointF(vecOffs[0]+m_vec_ki[0]+m_vec_Q[0]/2., vecOffs[1]+m_vec_ki[1]+m_vec_Q[1]/2.));
	painter.rotate((-m_dKiQ)/M_PI*180.);
	painter.translate(QPointF(-dFontSize/2., dFontSize/2.));
	painter.scale(1.,-1.);
	painter.drawText(QPointF(0., 0.), "Q");
	painter.restore();

	font.setPixelSize(8);
	painter.setFont(font);
	painter.translate(QPointF(dWidth/10., dHeight*9./10.));
	painter.scale(1.,-1.);
	std::ostringstream ostr2t;
	ostr2t.precision(4);
	ostr2t << "2theta = " << m_d2Theta/M_PI*180. << " deg, ";
	ostr2t << "ki_q = " << m_dKiQ/M_PI*180. << " deg";
	//ostr2t << "kf_q = " << 180.-(m_dKiQ/M_PI*180.) << " deg";
	painter.drawText(QPointF(0., 0.), ostr2t.str().c_str());
}




//----------------------------------------------------------------------------------------


ResoDlg::ResoDlg(QWidget *pParent)
				: QDialog(pParent),
				  m_bDontCalc(1),
				  m_pElliDlg(0), m_pElli3dDlg(0), m_pInstDlg(0), m_pScatterDlg(0)
{
	setupUi(this);
	m_vecSpinBoxes = {spinMonod, spinMonoMosaic, spinAnad,
						spinAnaMosaic, spinSampleMosaic, spinkfix,
						spinE, spinQ, spinHCollMono, spinHCollBSample,
						spinHCollASample, spinHCollAna, spinVCollMono,
						spinVCollBSample, spinVCollASample, spinVCollAna,
						spinMonoRefl, spinAnaEffic,

						spinMonoW, spinMonoH, spinMonoThick, spinMonoCurvH, spinMonoCurvV,
						spinSampleW_Q, spinSampleW_perpQ, spinSampleH,
						spinAnaW, spinAnaH, spinAnaThick, spinAnaCurvH, spinAnaCurvV,
						spinSrcW, spinSrcH,
						spinGuideDivH, spinGuideDivV,
						spinDetW, spinDetH,
						spinDistMonoSample, spinDistSampleAna, spinDistAnaDet, spinDistSrcMono};

	m_vecSpinNames = {"reso/mono_d", "reso/mono_mosaic", "reso/ana_d",
					"reso/ana_mosaic", "reso/sample_mosaic", "reso/k_fix",
					"reso/E", "reso/Q", "reso/h_coll_mono", "reso/h_coll_before_sample",
					"reso/h_coll_after_sample", "reso/h_coll_ana",
					"reso/v_coll_mono", "reso/v_coll_before_sample",
					"reso/v_coll_after_sample", "reso/v_coll_ana",
					"reso/mono_refl", "reso/ana_effic",

					"reso/pop_mono_w", "reso/pop_mono_h", "reso/pop_mono_thick", "reso/pop_mono_curvh", "reso/pop_mono_curvv",
					"reso/pop_sample_wq", "reso/pop_sampe_wperpq", "reso/pop_sample_h",
					"reso/pop_ana_w", "reso/pop_ana_h", "reso/pop_ana_thick", "reso/pop_ana_curvh", "reso/pop_ana_curvv",
					"reso/pop_src_w", "reso/pop_src_h",
					"reso/pop_guide_divh", "reso/pop_guide_divv",
					"reso/pop_det_w", "reso/pop_det_h",
					"reso/pop_dist_mono_sample", "reso/pop_dist_sample_ana", "reso/pop_dist_ana_det", "reso/pop_dist_src_mono"};


	m_vecCheckBoxes = {checkAnaCurvH, checkAnaCurvV, checkMonoCurvH, checkMonoCurvV};
	m_vecCheckNames = {"reso/pop_ana_use_curvh", "reso/pop_ana_use_curvv", "reso/pop_mono_use_curvh", "reso/pop_mono_use_curvv"};


	m_vecRadioPlus = {radioFixedki, radioMonoScatterPlus, radioAnaScatterPlus,
						radioSampleScatterPlus, radioConstMon, radioCN,
						radioSampleCub, radioSrcRect, radioDetRect};
	m_vecRadioMinus = {radioFixedkf, radioMonoScatterMinus, radioAnaScatterMinus,
						radioSampleScatterMinus, radioConstTime, radioPop,
						radioSampleCyl, radioSrcCirc, radioDetCirc};
	m_vecRadioNames = {"reso/check_fixed_ki", "reso/mono_scatter_sense", "reso/ana_scatter_sense",
						"reso/sample_scatter_sense", "reso/meas_const_mon",
						"reso/algo",
						"reso/pop_sample_cuboid", "reso/pop_source_rect", "reso/pop_det_rect"};


	UpdateUI();
	QObject::connect(radioFixedki, SIGNAL(toggled(bool)), this, SLOT(UpdateUI()));

	ReadLastConfig();

	QObject::connect(groupGuide, SIGNAL(toggled(bool)), this, SLOT(Calc()));

	QCheckBox* pCheckBoxes[] = {checkAnaCurvH, checkAnaCurvV, checkMonoCurvH, checkMonoCurvV};
	for(QCheckBox* pbox : pCheckBoxes)
		QObject::connect(pbox, SIGNAL(toggled(bool)), this, SLOT(Calc()));

	for(QDoubleSpinBox* pSpinBox : m_vecSpinBoxes)
		QObject::connect(pSpinBox, SIGNAL(valueChanged(double)), this, SLOT(Calc()));
	for(QRadioButton* pRadio : m_vecRadioPlus)
		QObject::connect(pRadio, SIGNAL(toggled(bool)), this, SLOT(Calc()));

	connect(btnGraphics, SIGNAL(clicked(bool)), this, SLOT(ShowEllipses()));
	connect(btnGraphics3D, SIGNAL(clicked(bool)), this, SLOT(ShowEllipses3d()));
	connect(btnShowInstr, SIGNAL(clicked(bool)), this, SLOT(ShowInstrLayout()));
	connect(btnShowTriangle, SIGNAL(clicked(bool)), this, SLOT(ShowScatterTriag()));

	connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(ButtonBoxClicked(QAbstractButton*)));

	connect(btnLoad, SIGNAL(clicked(bool)), this, SLOT(LoadFile()));
	connect(btnSave, SIGNAL(clicked(bool)), this, SLOT(SaveFile()));

	m_bDontCalc = 0;
	Calc();
}

ResoDlg::~ResoDlg()
{
	if(m_pElliDlg) delete m_pElliDlg;
	if(m_pElli3dDlg) delete m_pElli3dDlg;
	if(m_pInstDlg) delete m_pInstDlg;
	if(m_pScatterDlg) delete m_pScatterDlg;
}


void ResoDlg::UpdateUI()
{
	if(radioFixedki->isChecked())
	{
		labelkfix->setText(QString::fromUtf8("k_i (1/���):"));
		labelkvar->setText(QString::fromUtf8("k_f (1/���):"));
	}
	else
	{
		labelkfix->setText(QString::fromUtf8("k_f (1/���):"));
		labelkvar->setText(QString::fromUtf8("k_i (1/���):"));
	}
}

void ResoDlg::Calc()
{
	if(m_bDontCalc)
		return;

	const units::quantity<units::si::length> angstrom = 1e-10 * units::si::meter;

	PopParams& cn = m_pop;
	CNResults &res = m_res;

	// CN
	cn.mono_d = spinMonod->value() * angstrom;
	cn.mono_mosaic = spinMonoMosaic->value() / (180.*60.) * M_PI * units::si::radians;
	cn.ana_d = spinAnad->value() * angstrom;
	cn.ana_mosaic = spinAnaMosaic->value() / (180.*60.) * M_PI * units::si::radians;
	cn.sample_mosaic = spinSampleMosaic->value() / (180.*60.) * M_PI * units::si::radians;

	cn.bki_fix = radioFixedki->isChecked();
	cn.ki = cn.kf = spinkfix->value() / angstrom;
	cn.E = spinE->value() * (1e-3 * codata::e * units::si::volts);
	cn.Q = spinQ->value() / angstrom;

	cn.dmono_sense = (radioMonoScatterPlus->isChecked() ? +1. : -1.);
	cn.dana_sense = (radioAnaScatterPlus->isChecked() ? +1. : -1.);
	cn.dsample_sense = (radioSampleScatterPlus->isChecked() ? +1. : -1.);

	cn.coll_h_pre_mono = spinHCollMono->value() / (180.*60.) * M_PI * units::si::radians;
	cn.coll_h_pre_sample = spinHCollBSample->value() / (180.*60.) * M_PI * units::si::radians;
	cn.coll_h_post_sample = spinHCollASample->value() / (180.*60.) * M_PI * units::si::radians;
	cn.coll_h_post_ana = spinHCollAna->value() / (180.*60.) * M_PI * units::si::radians;

	cn.coll_v_pre_mono = spinVCollMono->value() / (180.*60.) * M_PI * units::si::radians;
	cn.coll_v_pre_sample = spinVCollBSample->value() / (180.*60.) * M_PI * units::si::radians;
	cn.coll_v_post_sample = spinVCollASample->value() / (180.*60.) * M_PI * units::si::radians;
	cn.coll_v_post_ana = spinVCollAna->value() / (180.*60.) * M_PI * units::si::radians;

	cn.dmono_refl = spinMonoRefl->value();
	cn.dana_effic = spinAnaEffic->value();
	cn.bConstMon = radioConstMon->isChecked();

	// Pop
	cn.mono_w = spinMonoW->value()*0.01*units::si::meter;
	cn.mono_h = spinMonoH->value()*0.01*units::si::meter;
	cn.mono_thick = spinMonoThick->value()*0.01*units::si::meter;
	cn.mono_curvh = spinMonoCurvH->value()*0.01*units::si::meter;
	cn.mono_curvv = spinMonoCurvV->value()*0.01*units::si::meter;
	cn.bMonoIsCurvedH = checkMonoCurvH->isChecked();
	cn.bMonoIsCurvedV = checkMonoCurvV->isChecked();

	cn.ana_w = spinAnaW->value()*0.01*units::si::meter;
	cn.ana_h = spinAnaH->value()*0.01*units::si::meter;
	cn.ana_thick = spinAnaThick->value()*0.01*units::si::meter;
	cn.ana_curvh = spinAnaCurvH->value()*0.01*units::si::meter;
	cn.ana_curvv = spinAnaCurvV->value()*0.01*units::si::meter;
	cn.bAnaIsCurvedH = checkAnaCurvH->isChecked();
	cn.bAnaIsCurvedV = checkAnaCurvV->isChecked();

	cn.bSampleCub = radioSampleCub->isChecked();
	cn.sample_w_q = spinSampleW_Q->value()*0.01*units::si::meter;
	cn.sample_w_perpq = spinSampleW_perpQ->value()*0.01*units::si::meter;
	cn.sample_h = spinSampleH->value()*0.01*units::si::meter;

	cn.bSrcRect = radioSrcRect->isChecked();
	cn.src_w = spinSrcW->value()*0.01*units::si::meter;
	cn.src_h = spinSrcH->value()*0.01*units::si::meter;

	cn.bDetRect = radioDetRect->isChecked();
	cn.det_w = spinDetW->value()*0.01*units::si::meter;
	cn.det_h = spinDetH->value()*0.01*units::si::meter;

	cn.bGuide = groupGuide->isChecked();
	cn.guide_div_h = spinGuideDivH->value() / (180.*60.) * M_PI * units::si::radians;
	cn.guide_div_v = spinGuideDivV->value() / (180.*60.) * M_PI * units::si::radians;

	cn.dist_mono_sample = spinDistMonoSample->value()*0.01*units::si::meter;
	cn.dist_sample_ana = spinDistSampleAna->value()*0.01*units::si::meter;
	cn.dist_ana_det = spinDistAnaDet->value()*0.01*units::si::meter;
	cn.dist_src_mono = spinDistSrcMono->value()*0.01*units::si::meter;

	const bool bUseCN = radioCN->isChecked();
	res = (bUseCN ? calc_cn(cn) : calc_pop(cn));

	if(m_pInstDlg) m_pInstDlg->SetParams(cn, res);
	if(m_pScatterDlg) m_pScatterDlg->SetParams(cn, res);

	if(res.bOk)
	{
		Ellipse ell = calc_res_ellipse(res.reso, res.Q_avg, 0, 3, 1, 2, -1);
		double dVanadiumFWHM = ell.y_hwhm*2.;

		std::ostringstream ostrRes;

		//ostrRes << std::scientific;
		ostrRes.precision(8);
		ostrRes << "Resolution Volume: " << res.dR0 << " meV \342\204\253^(-3)";
		ostrRes << "\nBragg FWHMs: " << res.dBraggFWHMs[0] << " \342\204\253^(-1), "
		                                          << res.dBraggFWHMs[1] << " \342\204\253^(-1), "
		                                          << res.dBraggFWHMs[2] << " \342\204\253^(-1), "
		                                          << res.dBraggFWHMs[3] << " meV ";
		ostrRes << "\nVanadium FWHM: " << dVanadiumFWHM << " meV";
		ostrRes << "\n";
		ostrRes << "\nResolution Matrix (Q_para, Q_ortho, Q_z, E): \n\n";

		for(unsigned int i=0; i<res.reso.size1(); ++i)
		{
			for(unsigned int j=0; j<res.reso.size2(); ++j)
				ostrRes << std::setw(20) << res.reso(i,j);

			if(i!=res.reso.size1()-1)
				ostrRes << "\n";
		}

		labelStatus->setText("Calculation successful.");
		labelResult->setText(QString::fromUtf8(ostrRes.str().c_str()));

		std::ostringstream ostrkvar;

		double dKVar = cn.bki_fix?(cn.kf*angstrom):(cn.ki*angstrom);
		//ostrkvar.precision(4);
		ostrkvar << dKVar;
		labelkvar_val->setText(ostrkvar.str().c_str());

		if(m_pElliDlg) m_pElliDlg->SetParams(cn, res);
		if(m_pElli3dDlg) m_pElli3dDlg->SetParams(cn, res);
	}
	else
	{
		QString strErr = "Error: ";
		strErr += res.strErr.c_str();
		labelStatus->setText(QString("<font color='red'>") + strErr + QString("</font>"));

		labelkvar_val->setText("<error>");
	}
}

void ResoDlg::WriteLastConfig()
{
	for(unsigned int iSpinBox=0; iSpinBox<m_vecSpinBoxes.size(); ++iSpinBox)
		Settings::Set<double>(m_vecSpinNames[iSpinBox].c_str(), m_vecSpinBoxes[iSpinBox]->value());
	for(unsigned int iRadio=0; iRadio<m_vecRadioPlus.size(); ++iRadio)
		Settings::Set<bool>(m_vecRadioNames[iRadio].c_str(), m_vecRadioPlus[iRadio]->isChecked());
	for(unsigned int iCheck=0; iCheck<m_vecCheckBoxes.size(); ++iCheck)
		Settings::Set<bool>(m_vecCheckNames[iCheck].c_str(), m_vecCheckBoxes[iCheck]->isChecked());

	Settings::Set<bool>("reso/use_guide", groupGuide->isChecked());
}

void ResoDlg::ReadLastConfig()
{
	bool bOldDontCalc = m_bDontCalc;
	m_bDontCalc = 1;

	for(unsigned int iSpinBox=0; iSpinBox<m_vecSpinBoxes.size(); ++iSpinBox)
	{
		if(!Settings::HasKey(m_vecSpinNames[iSpinBox].c_str()))
			continue;
		m_vecSpinBoxes[iSpinBox]->setValue(Settings::Get<double>(m_vecSpinNames[iSpinBox].c_str()));
	}

	for(unsigned int iCheckBox=0; iCheckBox<m_vecCheckBoxes.size(); ++iCheckBox)
	{
		if(!Settings::HasKey(m_vecCheckNames[iCheckBox].c_str()))
			continue;
		m_vecCheckBoxes[iCheckBox]->setChecked(Settings::Get<bool>(m_vecCheckNames[iCheckBox].c_str()));
	}

	for(unsigned int iRadio=0; iRadio<m_vecRadioPlus.size(); ++iRadio)
	{
		if(!Settings::HasKey(m_vecRadioNames[iRadio].c_str()))
			continue;

		bool bChecked = Settings::Get<bool>(m_vecRadioNames[iRadio].c_str());
		if(bChecked)
		{
			m_vecRadioPlus[iRadio]->setChecked(1);
			//m_vecRadioMinus[iRadio]->setChecked(0);;
		}
		else
		{
			//m_vecRadioPlus[iRadio]->setChecked(0);
			m_vecRadioMinus[iRadio]->setChecked(1);;
		}
	}

	groupGuide->setChecked(Settings::Get<bool>("reso/use_guide"));

	m_bDontCalc = bOldDontCalc;
	Calc();
}

void ResoDlg::SaveFile()
{
	QSettings *pGlobals = Settings::GetGlobals();
	QString strLastDir = pGlobals->value("reso/lastdir", ".").toString();

	QString strFile = QFileDialog::getSaveFileName(this, "Save resolution file...", strLastDir,
					"RES files (*.res *.RES);;All files (*.*)"/*,
					0, QFileDialog::DontUseNativeDialog*/);
	if(strFile.length() == 0)
		return;


	typedef std::map<std::string, std::string> tmap;
	tmap mapConf;

	for(unsigned int iSpinBox=0; iSpinBox<m_vecSpinBoxes.size(); ++iSpinBox)
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << m_vecSpinBoxes[iSpinBox]->value();

		mapConf[m_vecSpinNames[iSpinBox]] = ostrVal.str();
	}

	for(unsigned int iCheckBox=0; iCheckBox<m_vecCheckBoxes.size(); ++iCheckBox)
		mapConf[m_vecCheckNames[iCheckBox]] = (m_vecCheckBoxes[iCheckBox]->isChecked() ? "1" : "0");

	for(unsigned int iRadio=0; iRadio<m_vecRadioPlus.size(); ++iRadio)
		mapConf[m_vecRadioNames[iRadio]] = (m_vecRadioPlus[iRadio]->isChecked() ? "1" : "0");

	mapConf["reso/use_guide"] = groupGuide->isChecked() ? "1" : "0";

	if(!Xml::SaveMap(strFile.toStdString().c_str(), mapConf))
	{
		QMessageBox::critical(this, "Error", "Could not save configuration file.");
		return;
	}

	std::string strFile1 = strFile.toStdString();
	std::string strFileName = get_file(strFile1);
	setWindowTitle(QString("Resolution - ") + strFileName.c_str());
	pGlobals->setValue("reso/lastdir", QString(::get_dir(strFile1).c_str()));
}

void ResoDlg::LoadFile()
{
	bool bOldDontCalc = m_bDontCalc;
	m_bDontCalc = 1;

	QSettings *pGlobals = Settings::GetGlobals();
	QString strLastDir = pGlobals->value("reso/lastdir", ".").toString();

	QString strFile = QFileDialog::getOpenFileName(this, "Open resolution file...", strLastDir,
					"RES files (*.res *.RES);;All files (*.*)"/*,
					0, QFileDialog::DontUseNativeDialog*/);
	if(strFile.length() == 0)
		return;

	std::string strFile1 = strFile.toStdString();
	std::string strFileName = get_file(strFile1);

	Xml xml;
	if(!xml.Load(strFile1.c_str()))
	{
		QMessageBox::critical(this, "Error", "Could not load configuration file.");
		return;
	}


	bool bOk=0;
	for(unsigned int iSpinBox=0; iSpinBox<m_vecSpinBoxes.size(); ++iSpinBox)
		m_vecSpinBoxes[iSpinBox]->setValue(xml.Query<double>(m_vecSpinNames[iSpinBox].c_str(), 0., &bOk));

	for(unsigned int iCheck=0; iCheck<m_vecCheckBoxes.size(); ++iCheck)
	{
		int bChecked = xml.Query<int>(m_vecCheckNames[iCheck].c_str(), 0, &bOk);
		m_vecCheckBoxes[iCheck]->setChecked(bChecked);
	}

	for(unsigned int iRadio=0; iRadio<m_vecRadioPlus.size(); ++iRadio)
	{
		int bChecked = xml.Query<int>(m_vecRadioNames[iRadio].c_str(), 0, &bOk);
		if(bChecked)
			m_vecRadioPlus[iRadio]->setChecked(1);
		else
			m_vecRadioMinus[iRadio]->setChecked(1);;
	}

	groupGuide->setChecked(xml.Query<int>("reso/use_guide", 0, &bOk));


	setWindowTitle(QString("Resolution - ") + strFileName.c_str());
	pGlobals->setValue("reso/lastdir", QString(::get_dir(strFile1).c_str()));

	m_bDontCalc = bOldDontCalc;
	Calc();
}


void ResoDlg::ButtonBoxClicked(QAbstractButton* pBtn)
{
	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::ApplyRole ||
	   buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		WriteLastConfig();
	}
	else if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::RejectRole)
	{
		reject();
	}

	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		QDialog::accept();
	}
}

void ResoDlg::ShowEllipses()
{
	if(!m_pElliDlg)
	{
		m_pElliDlg = new EllipseDlg(this);
		m_pElliDlg->SetParams(m_pop, m_res);
	}

	if(!m_pElliDlg->isVisible())
		m_pElliDlg->show();
	m_pElliDlg->activateWindow();
}

void ResoDlg::ShowEllipses3d()
{
	if(!m_pElli3dDlg)
	{
		m_pElli3dDlg = new EllipseDlg3D(this);
		m_pElli3dDlg->SetParams(m_pop, m_res);
	}

	if(!m_pElli3dDlg->isVisible())
		m_pElli3dDlg->show();
	m_pElli3dDlg->activateWindow();
}

void ResoDlg::ShowInstrLayout()
{
	if(!m_pInstDlg)
	{
		m_pInstDlg = new InstLayoutDlg(this);
		m_pInstDlg->SetParams(m_pop, m_res);
	}

	if(!m_pInstDlg->isVisible())
		m_pInstDlg->show();
	m_pInstDlg->activateWindow();
}

void ResoDlg::ShowScatterTriag()
{
	if(!m_pScatterDlg)
	{
		m_pScatterDlg = new ScatterTriagDlg(this);
		m_pScatterDlg->SetParams(m_pop, m_res);
	}

	if(!m_pScatterDlg->isVisible())
		m_pScatterDlg->show();
	m_pScatterDlg->activateWindow();
}

void ResoDlg::hideEvent(QHideEvent *event)
{
	Settings::GetGlobals()->setValue("reso/wnd_geo", saveGeometry());
	if(m_pElliDlg) m_pElliDlg->hide();
	if(m_pElli3dDlg) m_pElli3dDlg->hide();
	if(m_pInstDlg) m_pInstDlg->hide();
	if(m_pScatterDlg) m_pScatterDlg->hide();
}

void ResoDlg::showEvent(QShowEvent *event)
{
	restoreGeometry(Settings::GetGlobals()->value("reso/wnd_geo").toByteArray());
}


#include "ResoDlg.moc"
