/*
 * Ellipse Dialog
 * @author Tobias Weber
 * @date may-2013, 29-apr-2014
 * @license GPLv2
 */

#include "EllipseDlg.h"
#include "tlibs/string/spec_char.h"
#include "tlibs/helper/flags.h"

#include <qwt_picker_machine.h>
#include <future>

EllipseDlg::EllipseDlg(QWidget* pParent, QSettings* pSett)
			: QDialog(pParent), m_pSettings(pSett)
{
	setupUi(this);
	m_vecPlots = {plot1,plot2,plot3,plot4};

	QColor colorBck(240, 240, 240, 255);
	for(QwtPlot *pPlt : m_vecPlots)
		pPlt->setCanvasBackground(colorBck);


	m_elliProj.resize(4);
	m_elliSlice.resize(4);
	m_vecGrid.resize(4);
	m_vecPickers.resize(4);
	m_vecZoomers.resize(4);

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


		m_vecPlots[i]->canvas()->setMouseTracking(1);
		m_vecPickers[i] = new QwtPlotPicker(m_vecPlots[i]->xBottom,
											m_vecPlots[i]->yLeft,
#if QWT_VER<6
											QwtPlotPicker::PointSelection,
#endif
											//QwtPlotPicker::CrossRubberBand,
											QwtPlotPicker::NoRubberBand,
#if QWT_VER>=6
											QwtPlotPicker::AlwaysOff,
#else
											QwtPlotPicker::AlwaysOn,
#endif
											m_vecPlots[i]->canvas());

#if QWT_VER>=6
		m_vecPickers[i]->setStateMachine(new QwtPickerTrackerMachine());
		connect(m_vecPickers[i], SIGNAL(moved(const QPointF&)),
				this, SLOT(cursorMoved(const QPointF&)));
#endif
		m_vecPickers[i]->setEnabled(1);


		m_vecZoomers[i] = new QwtPlotZoomer(m_vecPlots[i]->canvas());
		m_vecZoomers[i]->setMaxStackDepth(-1);
		m_vecZoomers[i]->setEnabled(1);
	}


	if(m_pSettings && m_pSettings->contains("reso/ellipse_geo"))
		restoreGeometry(m_pSettings->value("reso/ellipse_geo").toByteArray());
}


EllipseDlg::~EllipseDlg()
{
	for(QwtPlotPicker* pPicker : m_vecPickers)
	{
		pPicker->setEnabled(0);
		//QwtPickerMachine *pMachine = pPicker->stateMachine();
		//pPicker->setStateMachine(0);
		//if(pMachine)
		//	delete pMachine;
		delete pPicker;
	}
	m_vecPickers.clear();

	for(QwtPlotZoomer* pZoomer : m_vecZoomers)
	{
		pZoomer->setEnabled(0);
		delete pZoomer;
	}
	m_vecZoomers.clear();

#if QWT_VER<6
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

void EllipseDlg::cursorMoved(const QPointF& pt)
{
	std::string strX = std::to_string(pt.x());
	std::string strY = std::to_string(pt.y());

	std::ostringstream ostr;
	ostr << "(" << strX << ", " << strY << ")";

	this->labelStatus->setText(ostr.str().c_str());
}

void EllipseDlg::SetParams(const ublas::matrix<double>& reso, const ublas::vector<double>& _Q_avg)
{
	try
	{
		static const int iParams[2][4][5] =
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

		static const std::string strDeg = tl::get_spec_char_utf8("deg");


		//Xml xmlparams;
		bool bXMLLoaded = 0; //xmlparams.Load("res/res.conf");
		bool bCenterOn0 = 1; //xmlparams.Query<bool>("/res/center_around_origin", 0);

		ublas::vector<double> Q_avg = _Q_avg;
		if(bCenterOn0)
			Q_avg = ublas::zero_vector<double>(Q_avg.size());


		std::vector<std::future<Ellipse>> tasks_ell_proj, tasks_ell_slice;

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

			const int *iP = iParams[0][iEll];
			const int *iS = iParams[1][iEll];

			std::future<Ellipse> ell_proj = std::async(std::launch::deferred|std::launch::async,
						[=, &reso, &Q_avg]()
						{ return ::calc_res_ellipse(reso, Q_avg, iP[0], iP[1], iP[2], iP[3], iP[4]); });
			std::future<Ellipse> ell_slice = std::async(std::launch::deferred|std::launch::async,
						[=, &reso, &Q_avg]()
						{ return ::calc_res_ellipse(reso, Q_avg, iS[0], iS[1], iS[2], iS[3], iS[4]); });

			tasks_ell_proj.push_back(std::move(ell_proj));
			tasks_ell_slice.push_back(std::move(ell_slice));
		}

		for(unsigned int iEll=0; iEll<4; ++iEll)
		{
			m_elliProj[iEll] = tasks_ell_proj[iEll].get();
			m_elliSlice[iEll] = tasks_ell_slice[iEll].get();

			/*m_elliProj[iEll] = ::calc_res_ellipse(res.reso, Q_avg, iParams[0][iEll][0], iParams[0][iEll][1],
											iParams[0][iEll][2], iParams[0][iEll][3], iParams[0][iEll][4]);
			m_elliSlice[iEll] = ::calc_res_ellipse(res.reso, Q_avg, iParams[1][iEll][0], iParams[1][iEll][1],
											iParams[1][iEll][2], iParams[1][iEll][3], iParams[1][iEll][4]);*/

			QwtPlot* pPlot = m_vecPlots[iEll];
			QwtPlotCurve* pCurveProj = m_vecPlotCurves[iEll*2+0];
			QwtPlotCurve* pCurveSlice = m_vecPlotCurves[iEll*2+1];
			std::vector<double>& vecXProj = m_vecXCurvePoints[iEll*2+0];
			std::vector<double>& vecYProj = m_vecYCurvePoints[iEll*2+0];
			std::vector<double>& vecXSlice = m_vecXCurvePoints[iEll*2+1];
			std::vector<double>& vecYSlice = m_vecYCurvePoints[iEll*2+1];

			double dBBProj[4], dBBSlice[4];
			m_elliProj[iEll].GetCurvePoints(vecXProj, vecYProj, 512, dBBProj);
			m_elliSlice[iEll].GetCurvePoints(vecXSlice, vecYSlice, 512, dBBSlice);

	#if QWT_VER>=6
			pCurveProj->setRawSamples(vecXProj.data(), vecYProj.data(), vecXProj.size());
			pCurveSlice->setRawSamples(vecXSlice.data(), vecYSlice.data(), vecXSlice.size());
	#else
			pCurveProj->setRawData(vecXProj.data(), vecYProj.data(), vecXProj.size());
			pCurveSlice->setRawData(vecXSlice.data(), vecYSlice.data(), vecXSlice.size());
	#endif


			std::ostringstream ostrSlope;
			ostrSlope.precision(4);
			ostrSlope << "Projected ellipse (green):\n";
			ostrSlope << "\tSlope: " << m_elliProj[iEll].slope << "\n";
			ostrSlope << "\tAngle: " << m_elliProj[iEll].phi/M_PI*180. << strDeg << "\n";
			ostrSlope << "\tArea " << m_elliProj[iEll].area << "\n";
			ostrSlope << "Sliced ellipse (blue):\n";
			ostrSlope << "\tSlope: " << m_elliSlice[iEll].slope << "\n";
			ostrSlope << "\tAngle: " << m_elliSlice[iEll].phi/M_PI*180. << strDeg << "\n";
			ostrSlope << "\tArea " << m_elliSlice[iEll].area;
			//pPlot->setTitle(ostrSlope.str().c_str());
			//std::cout << "Ellipse " << iEll << ": " << ostrSlope.str() << std::endl;
			m_vecPlots[iEll]->setToolTip(QString::fromUtf8(ostrSlope.str().c_str()));

			pPlot->setAxisTitle(QwtPlot::xBottom, m_elliProj[iEll].x_lab.c_str());
			pPlot->setAxisTitle(QwtPlot::yLeft, m_elliProj[iEll].y_lab.c_str());

			pPlot->replot();

			QRectF rect;
			rect.setLeft(std::min(dBBProj[0], dBBSlice[0]));
			rect.setRight(std::max(dBBProj[1], dBBSlice[1]));
			rect.setTop(std::max(dBBProj[2], dBBSlice[2]));
			rect.setBottom(std::min(dBBProj[3], dBBSlice[3]));
			m_vecZoomers[iEll]->setZoomBase(rect);
		}
	}
	catch(const std::exception& ex)
	{
		tl::log_err("Cannot calculate ellipses.");
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
	QDialog::showEvent(pEvt);
}

#include "EllipseDlg.moc"
