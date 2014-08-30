/*
 * Ellipse Dialog
 * @author Tobias Weber
 * @date may-2013, 29-apr-2014
 */

#include "EllipseDlg.h"


EllipseDlg::EllipseDlg(QWidget* pParent, QSettings* pSett)
			: QDialog(pParent), m_pSettings(pSett)
{
	setupUi(this);
	m_vecPlots = {plot1,plot2,plot3,plot4};

	m_elliProj.resize(4);
	m_elliSlice.resize(4);
	m_vecGrid.resize(4);

	m_vecPlotCurves.resize(8);

	m_vecXCurvePoints.resize(8);
	m_vecYCurvePoints.resize(8);

	for(unsigned int i=0; i<4; ++i)
	{
		m_vecPlots[i]->setCanvasBackground(QColor(0xff,0xff,0xff));
		m_vecPlots[i]->setMinimumSize(200,200);

		m_vecGrid[i] = new QwtPlotGrid();
		QPen penGrid;
		penGrid.setColor(QColor(0x99,0x99,0x99));
		penGrid.setStyle(Qt::DashLine);
		m_vecGrid[i]->setPen(penGrid);
		m_vecGrid[i]->attach(m_vecPlots[i]);

		QwtPlotCurve *pCurveProj = new QwtPlotCurve("projected");
		QwtPlotCurve *pCurveSlice = new QwtPlotCurve("sliced");

		QPen penProj, penSlice;
		penProj.setColor(QColor(0, 0x99,0));
		penSlice.setColor(QColor(0,0,0x99));
		penProj.setWidth(2);
		penSlice.setWidth(2);

		pCurveProj->setPen(penProj);
		pCurveSlice->setPen(penSlice);

		pCurveProj->setRenderHint(QwtPlotItem::RenderAntialiased, true);
		pCurveSlice->setRenderHint(QwtPlotItem::RenderAntialiased, true);

		pCurveProj->attach(m_vecPlots[i]);
		pCurveSlice->attach(m_vecPlots[i]);

		m_vecPlotCurves[i*2+0] = pCurveProj;
		m_vecPlotCurves[i*2+1] = pCurveSlice;
	}
}

EllipseDlg::~EllipseDlg()
{
#ifndef USE_QWT6
	for(QwtPlot* pPlot : m_vecPlots)
		pPlot->clear();
#endif

	for(QwtPlotGrid* pGrid : m_vecGrid)
		delete pGrid;
	m_vecGrid.clear();

	//for(QwtPlotCurve *pCurve : m_vecPlotCurves)
	//	delete pCurve;
	m_vecPlotCurves.clear();
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


	//Xml xmlparams;
	bool bXMLLoaded = 0; //xmlparams.Load("res/res.conf");
	bool bCenterOn0 = 1; //xmlparams.Query<bool>("/res/center_around_origin", 0);

	for(unsigned int iEll=0; iEll<4; ++iEll)
	{/*
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
		}*/

		ublas::vector<double> Q_avg = res.Q_avg;
		if(bCenterOn0)
			Q_avg = ublas::zero_vector<double>(Q_avg.size());

		m_elliProj[iEll] = ::calc_res_ellipse(res.reso, Q_avg, iParams[0][iEll][0], iParams[0][iEll][1],
										iParams[0][iEll][2], iParams[0][iEll][3], iParams[0][iEll][4]);
		m_elliSlice[iEll] = ::calc_res_ellipse(res.reso, Q_avg, iParams[1][iEll][0], iParams[1][iEll][1],
										iParams[1][iEll][2], iParams[1][iEll][3], iParams[1][iEll][4]);

		QwtPlot* pPlot = m_vecPlots[iEll];
		QwtPlotCurve* pCurveProj = m_vecPlotCurves[iEll*2+0];
		QwtPlotCurve* pCurveSlice = m_vecPlotCurves[iEll*2+1];
		std::vector<double>& vecXProj = m_vecXCurvePoints[iEll*2+0];
		std::vector<double>& vecYProj = m_vecYCurvePoints[iEll*2+0];
		std::vector<double>& vecXSlice = m_vecXCurvePoints[iEll*2+1];
		std::vector<double>& vecYSlice = m_vecYCurvePoints[iEll*2+1];

		m_elliProj[iEll].GetCurvePoints(vecXProj, vecYProj, 512);
		m_elliSlice[iEll].GetCurvePoints(vecXSlice, vecYSlice, 512);

#ifdef USE_QWT6
		pCurveProj->setRawSamples(vecXProj.data(), vecYProj.data(), vecXProj.size());
		pCurveSlice->setRawSamples(vecXSlice.data(), vecYSlice.data(), vecXSlice.size());
#else
		pCurveProj->setRawData(vecXProj.data(), vecYProj.data(), vecXProj.size());
		pCurveSlice->setRawData(vecXSlice.data(), vecYSlice.data(), vecXSlice.size());
#endif


		std::ostringstream ostrSlope;
		ostrSlope.precision(4);
		ostrSlope << "Slope (proj): " << std::tan(-m_elliProj[iEll].phi)
				  << ", Angle (proj): " << m_elliProj[iEll].phi << "; ";
		ostrSlope << "Slope (slice): " << std::tan(-m_elliSlice[iEll].phi)
				  << ", Angle (slice): " << m_elliSlice[iEll].phi;
		//pPlot->setTitle(ostrSlope.str().c_str());
		std::cout << "Ellipse " << iEll << ": " << ostrSlope.str() << std::endl;


		pPlot->setAxisTitle(QwtPlot::xBottom, m_elliProj[iEll].x_lab.c_str());
		pPlot->setAxisTitle(QwtPlot::yLeft, m_elliProj[iEll].y_lab.c_str());

		pPlot->replot();
	}
}

void EllipseDlg::accept()
{
	if(m_pSettings)
		m_pSettings->setValue("reso/ellipse_geo", saveGeometry());

	QDialog::accept();
}

void EllipseDlg::showEvent(QShowEvent *pEvt)
{
	if(m_pSettings && m_pSettings->contains("reso/ellipse_geo"))
		restoreGeometry(m_pSettings->value("reso/ellipse_geo").toByteArray());

	QDialog::showEvent(pEvt);
}

#include "EllipseDlg.moc"
