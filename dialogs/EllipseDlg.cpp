/*
 * Ellipse Dialog
 * @author Tobias Weber
 * @date may-2013, 29-apr-2014
 */

#include "EllipseDlg.h"


EllipseDlg::EllipseDlg(QWidget* pParent, QSettings* pSett)
			: QDialog(pParent), m_pSettings(pSett)
{
	m_elliProj.resize(4);
	m_elliSlice.resize(4);

	setupUi(this);
}

EllipseDlg::~EllipseDlg()
{}

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

		m_elliProj[iEll] = ::calc_res_ellipse(res.reso, Q_avg, iParams[0][iEll][0], iParams[0][iEll][1], iParams[0][iEll][2], iParams[0][iEll][3], iParams[0][iEll][4]);
		m_elliSlice[iEll] = ::calc_res_ellipse(res.reso, Q_avg, iParams[1][iEll][0], iParams[1][iEll][1], iParams[1][iEll][2], iParams[1][iEll][3], iParams[1][iEll][4]);
	}

/* TODO: use new qwt plotter
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
*/
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
