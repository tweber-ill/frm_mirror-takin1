/**
 * 3d Brillouin zone drawing
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date apr-2017
 * @license GPLv2
 */

#include "bz3d.h"
#include <QGridLayout>

#include "tlibs/math/linalg.h"
#include "tlibs/string/string.h"


using t_real = t_real_glob;
using t_vec = ublas::vector<t_real>;


BZ3DDlg::BZ3DDlg(QWidget* pParent, QSettings *pSettings)
	: QDialog(pParent, Qt::Tool), m_pSettings(pSettings),
	m_pStatus(new QStatusBar(this)),
	m_pPlot(new PlotGl(this, pSettings, 0.25))
{
	m_pPlot->SetEnabled(0);

	setWindowTitle("Reciprocal Space / Brillouin Zone");
	m_pStatus->setSizeGripEnabled(1);
	m_pStatus->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	if(m_pSettings)
	{
		QFont font;
		if(m_pSettings->contains("main/font_gen") && font.fromString(m_pSettings->value("main/font_gen", "").toString()))
			setFont(font);

		if(m_pSettings->contains("bz3d/geo"))
			restoreGeometry(m_pSettings->value("bz3d/geo").toByteArray());
		else
			resize(800, 600);
	}

	m_pPlot->SetPrec(g_iPrecGfx);
	m_pPlot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	QGridLayout *gridLayout = new QGridLayout(this);
	gridLayout->setContentsMargins(4, 4, 4, 4);
	gridLayout->addWidget(m_pPlot.get(), 0, 0, 1, 1);
	gridLayout->addWidget(m_pStatus, 1, 0, 1, 1);

	m_pPlot->AddHoverSlot([this](const PlotObjGl* pObj)
	{
		std::string strStatus;
		if(pObj)
			strStatus = pObj->strLabel;
		tl::find_all_and_replace<std::string>(strStatus, "\n", ", ");

		if(strStatus.length())
			m_pStatus->showMessage(strStatus.c_str());
		else
			m_pStatus->clearMessage();
	});

	m_pPlot->SetLabels("x (1/A)", "y (1/A)", "z (1/A)");
	m_pPlot->SetEnabled(1);
}


// ----------------------------------------------------------------------------


void BZ3DDlg::RenderBZ(const tl::Brillouin3D<t_real_glob>& bz)
{
	if(!bz.IsValid() || !m_pPlot)
		return;

	static const std::vector<t_real> vecColVertices = { 1., 0., 0., 0.75 };
	static const std::vector<t_real> vecColPolys = { 0., 0., 1., 0.75 };
	static const std::vector<t_real> vecColEdges = { 0., 0., 0., 1. };

	m_pPlot->SetEnabled(0);
	m_pPlot->clear();


	// all objects: vertices + polys + edges
	const std::size_t iNumObjs = bz.GetVertices().size() + 2*bz.GetPolys().size();
	m_pPlot->SetObjectCount(iNumObjs);


	std::size_t iCurObjIdx = 0;

	// render vertices
	for(const t_vec& vec : bz.GetVertices())
	{
		m_pPlot->PlotSphere(vec, 0.05, iCurObjIdx);
		m_pPlot->SetObjectColor(iCurObjIdx, vecColVertices);

		++iCurObjIdx;
	}

	// render polygons
	for(const std::vector<t_vec>& vecPoly : bz.GetPolys())
	{
		m_pPlot->PlotPoly(vecPoly, iCurObjIdx);
		m_pPlot->SetObjectColor(iCurObjIdx, vecColPolys);

		++iCurObjIdx;
	}

	// render edges
	for(const std::vector<t_vec>& vecPoly : bz.GetPolys())
	{
		m_pPlot->PlotLines(vecPoly, iCurObjIdx);
		m_pPlot->SetObjectColor(iCurObjIdx, vecColEdges);

		++iCurObjIdx;
	}


	m_pPlot->SetEnabled(1);
}


// ----------------------------------------------------------------------------


void BZ3DDlg::closeEvent(QCloseEvent* pEvt)
{
	if(m_pSettings)
		m_pSettings->setValue("bz3d/geo", saveGeometry());
	QDialog::closeEvent(pEvt);
}

void BZ3DDlg::hideEvent(QHideEvent *pEvt)
{
	if(m_pPlot) m_pPlot->SetEnabled(0);
	QDialog::hideEvent(pEvt);
}

void BZ3DDlg::showEvent(QShowEvent *pEvt)
{
	QDialog::showEvent(pEvt);
	if(m_pPlot) m_pPlot->SetEnabled(1);
}


#include "bz3d.moc"
