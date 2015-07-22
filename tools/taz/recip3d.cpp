/*
 * Scattering Triangle Tool
 * @author tweber
 * @date mar-2014
 * @copyright GPLv2
 */

#include "recip3d.h"
#include "tlibs/math/geo.h"
#include <QGridLayout>

Recip3DDlg::Recip3DDlg(QWidget* pParent, QSettings *pSettings)
			: QDialog(pParent), m_pPlot(new PlotGl(this, pSettings))
{
	setWindowFlags(Qt::Tool);
	setWindowTitle("Reciprocal Space");

	QGridLayout *gridLayout = new QGridLayout(this);
	m_pPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	gridLayout->addWidget(m_pPlot, 0, 0, 1, 1);

	resize(640, 480);
}

Recip3DDlg::~Recip3DDlg()
{
	if(m_pPlot)
	{
		delete m_pPlot;
		m_pPlot = 0;
	}
}


void Recip3DDlg::CalcPeaks(const tl::Lattice<double>& lattice,
							const tl::Lattice<double>& recip, const tl::Lattice<double>& recip_unrot,
							const tl::Plane<double>& plane,
							const SpaceGroup* pSpaceGroup)
{
	const unsigned int iObjCnt = (unsigned int)((m_dMaxPeaks*2 + 1)*
												(m_dMaxPeaks*2 + 1)*
												(m_dMaxPeaks*2 + 1));
	//log_info("Number of objects: ", iObjCnt);

	m_pPlot->SetEnabled(0);
	m_pPlot->clear();
	m_pPlot->SetObjectCount(iObjCnt);

	unsigned int iPeakIdx = 0;
	const double dLimMax = std::numeric_limits<double>::max();

	std::vector<double> vecMin = {dLimMax, dLimMax, dLimMax},
						vecMax = {-dLimMax, -dLimMax, -dLimMax};

	for(double h=-m_dMaxPeaks; h<=m_dMaxPeaks; h+=1.)
		for(double k=-m_dMaxPeaks; k<=m_dMaxPeaks; k+=1.)
			for(double l=-m_dMaxPeaks; l<=m_dMaxPeaks; l+=1.)
			{
				int ih = int(h), ik = int(k), il = int(l);
				if(pSpaceGroup)
				{
					if(!pSpaceGroup->HasReflection(ih, ik, il))
						continue;
				}

				bool bInScatteringPlane = 0;
				ublas::vector<double> vecPeak = recip.GetPos(h,k,l);
				for(unsigned int i=0; i<3; ++i)
				{
					vecMin[i] = std::min(vecPeak[i], vecMin[i]);
					vecMax[i] = std::max(vecPeak[i], vecMax[i]);
				}

				double dDist = 0.;
				ublas::vector<double> vecDropped = plane.GetDroppedPerp(vecPeak, &dDist);

				std::vector<double> vecColor{0., 0., 1., 0.7};
				if(tl::float_equal(dDist, 0., m_dPlaneDistTolerance))
				{
					bool bIsDirectBeam = 0;
					if(tl::float_equal(h, 0.) && tl::float_equal(k, 0.) && tl::float_equal(l, 0.))
						bIsDirectBeam = 1;

					if(bIsDirectBeam)
						vecColor = std::vector<double>{0., 1., 0., 0.7};
					else
						vecColor = std::vector<double>{1., 0., 0., 0.7};

					bInScatteringPlane = 1;
				}

				m_pPlot->PlotSphere(vecPeak, 0.1, iPeakIdx);
				m_pPlot->SetObjectColor(iPeakIdx, vecColor);

				std::ostringstream ostrLab;
				ostrLab << "(" << ih << " " << ik << " " << il << ")";
				m_pPlot->SetObjectLabel(iPeakIdx, ostrLab.str());

				//log_info("Index: ", iPeakIdx);
				++iPeakIdx;
			}

	m_pPlot->SetObjectCount(iPeakIdx);	// actual count (some peaks forbidden by sg)
	m_pPlot->SetMinMax(vecMin, vecMax);
	m_pPlot->SetEnabled(1);
}


void Recip3DDlg::hideEvent(QHideEvent *event)
{
	if(m_pPlot) m_pPlot->SetEnabled(0);

	//if(m_pSettings)
	//	m_pSettings->setValue("recip3d/geo", saveGeometry());
}
void Recip3DDlg::showEvent(QShowEvent *event)
{
	//if(m_pSettings && m_pSettings->contains("recip3d/geo"))
	//	restoreGeometry(m_pSettings->value("recip3d/geo").toByteArray());

	if(m_pPlot) m_pPlot->SetEnabled(1);
}


#include "recip3d.moc"
