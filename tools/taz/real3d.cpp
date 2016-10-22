/**
 * 3d unit cell drawing
 * @author tweber
 * @date oct-2016
 * @license GPLv2
 */

#include "real3d.h"
#include <QGridLayout>


using t_real = t_real_glob;
using t_vec = ublas::vector<t_real>;


Real3DDlg::Real3DDlg(QWidget* pParent, QSettings *pSettings)
	: QDialog(pParent), m_pPlot(new PlotGl(this, pSettings))
{
	setWindowFlags(Qt::Tool);
	setWindowTitle("Real Space");
	setSizeGripEnabled(1);

	QGridLayout *gridLayout = new QGridLayout(this);
	m_pPlot->SetPrec(g_iPrecGfx);
	m_pPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	gridLayout->addWidget(m_pPlot, 0, 0, 1, 1);

	resize(640, 480);
}

Real3DDlg::~Real3DDlg()
{
	if(m_pPlot)
	{
		delete m_pPlot;
		m_pPlot = 0;
	}
}


void Real3DDlg::CalcPeaks(const LatticeCommon<t_real_glob>& latticecommon)
{
	m_pPlot->SetEnabled(0);
	m_pPlot->clear();
	m_pPlot->SetObjectCount(latticecommon.vecAllAtoms.size());

	t_real dA = 0.7;
	std::vector<std::vector<t_real>> vecColor = {
		{1., 0., 0., dA},
		{0., 1., 0., dA},
		{0., 0., 1., dA},
		{1., 1., 0., dA},
		{0., 1., 1., dA},
		{1., 0., 1., dA},

		{0.5, 0., 0., dA},
		{0., 0.5, 0., dA},
		{0., 0., 0.5, dA},
		{0.5, 0.5, 0., dA},
		{0., 0.5, 0.5, dA},
		{0.5, 0., 0.5, dA},

		{1., 1., 1., dA},
		{0., 0., 0., dA},
		{0.5, 0.5, 0.5, dA},
	};

	std::size_t iPeakIdx = 0;
	const t_real dLimMax = std::numeric_limits<t_real>::max();
	std::vector<t_real> vecMin = {dLimMax, dLimMax, dLimMax},
		vecMax = {-dLimMax, -dLimMax, -dLimMax};

	for(std::size_t iAtom=0; iAtom<latticecommon.vecAllAtoms.size(); ++iAtom)
	{
		const std::string& strElem = latticecommon.vecAllNames[iAtom];
		const t_vec& vecThisAtom = latticecommon.vecAllAtoms[iAtom];
		const t_vec& vecThisAtomFrac = latticecommon.vecAllAtomsFrac[iAtom];
		std::size_t iCurAtomType = latticecommon.vecAllAtomTypes[iAtom];

		m_pPlot->PlotSphere(vecThisAtom, 0.1, iPeakIdx);
		m_pPlot->SetObjectColor(iPeakIdx, vecColor[iCurAtomType % vecColor.size()]);

		for(unsigned int i=0; i<3; ++i)
		{
			vecMin[i] = std::min(vecThisAtom[i], vecMin[i]);
			vecMax[i] = std::max(vecThisAtom[i], vecMax[i]);
		}

		std::ostringstream ostrTip;
		ostrTip.precision(g_iPrecGfx);
		ostrTip << strElem;
		ostrTip << "\n("
			<< vecThisAtomFrac[0] << ", "
			<< vecThisAtomFrac[1] << ", "
			<< vecThisAtomFrac[2] << ") frac";
		ostrTip << "\n("
			<< vecThisAtom[0] << ", "
			<< vecThisAtom[1] << ", "
			<< vecThisAtom[2] << ") A";

		m_pPlot->SetObjectLabel(iPeakIdx, ostrTip.str());

		++iPeakIdx;
	}

	m_pPlot->SetMinMax(vecMin, vecMax);
	m_pPlot->SetEnabled(1);
}


void Real3DDlg::hideEvent(QHideEvent *pEvt)
{
	if(m_pPlot) m_pPlot->SetEnabled(0);

	//if(m_pSettings)
	//	m_pSettings->setValue("real3d/geo", saveGeometry());

	QDialog::hideEvent(pEvt);
}
void Real3DDlg::showEvent(QShowEvent *pEvt)
{
	QDialog::showEvent(pEvt);

	//if(m_pSettings && m_pSettings->contains("real3d/geo"))
	//	restoreGeometry(m_pSettings->value("real3d/geo").toByteArray());

	if(m_pPlot) m_pPlot->SetEnabled(1);
}


#include "real3d.moc"
