/*
 * Scattering Triangle Tool
 * @author tweber
 * @date mar-2014
 */

#include "recip3d.h"
#include <QtGui/QGridLayout>

Recip3DDlg::Recip3DDlg(QWidget* pParent)
			: QDialog(pParent), m_pPlot(new PlotGl(this))
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


void Recip3DDlg::CalcPeaks(const Lattice& lattice,
							const Lattice& recip, const Lattice& recip_unrot,
							const Plane<double>& plane)
{
	const unsigned int iObjCnt = (unsigned int)((m_dMaxPeaks*2 + 1)*
												(m_dMaxPeaks*2 + 1)*
												(m_dMaxPeaks*2 + 1));
	m_pPlot->SetObjectCount(iObjCnt);

	unsigned int iPeakIdx = 0;
	const double dLimMax = std::numeric_limits<double>::max();

	std::vector<double> vecMin = {dLimMax, dLimMax, dLimMax},
						vecMax = {-dLimMax, -dLimMax, -dLimMax};

	for(double h=-m_dMaxPeaks; h<=m_dMaxPeaks; h+=1.)
		for(double k=-m_dMaxPeaks; k<=m_dMaxPeaks; k+=1.)
			for(double l=-m_dMaxPeaks; l<=m_dMaxPeaks; l+=1.)
			{
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
				if(::float_equal(dDist, 0., m_dPlaneDistTolerance))
				{
					bool bIsDirectBeam = 0;
					if(float_equal(h, 0.) && float_equal(k, 0.) && float_equal(l, 0.))
						bIsDirectBeam = 1;

					if(bIsDirectBeam)
						vecColor = std::vector<double>{0., 1., 0., 0.7};
					else
						vecColor = std::vector<double>{1., 0., 0., 0.7};

					bInScatteringPlane = 1;
				}

				m_pPlot->PlotSphere(vecPeak, 0.1, iPeakIdx);
				m_pPlot->SetObjectColor(iPeakIdx, vecColor);

				++iPeakIdx;
			}

	m_pPlot->SetMinMax(vecMin, vecMax);
}


#include "recip3d.moc"
