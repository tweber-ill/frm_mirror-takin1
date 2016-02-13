/*
 * Form Factor & Scattering Length Dialog
 * @author tweber
 * @date nov-2015
 * @license GPLv2
 */

#include "FormfactorDlg.h"
#include "helper/formfact.h"
#include "helper/qthelper.h"
#include "tlibs/string/spec_char.h"
#include <qwt_picker_machine.h>


FormfactorDlg::FormfactorDlg(QWidget* pParent, QSettings *pSettings)
	: QDialog(pParent), m_pSettings(pSettings)
{
	this->setupUi(this);
	SetupAtoms();

	QColor colorBck(240, 240, 240, 255);

	// form factors
	plotF->setCanvasBackground(colorBck);

	m_pGrid = new QwtPlotGrid();
	QPen penGrid;
	penGrid.setColor(QColor(0x99,0x99,0x99));
	penGrid.setStyle(Qt::DashLine);
	m_pGrid->setPen(penGrid);
	m_pGrid->attach(plotF);

#if QWT_VER>=6
	m_pZoomer = new QwtPlotZoomer(plotF->canvas());
	m_pZoomer->setMaxStackDepth(-1);
	m_pZoomer->setEnabled(1);
#endif

	m_pCurve = new QwtPlotCurve("Atomic Form Factor");
	QPen penCurve;
	penCurve.setColor(QColor(0,0,0x99));
	penCurve.setWidth(2);
	m_pCurve->setPen(penCurve);
	m_pCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	m_pCurve->attach(plotF);

	plotF->canvas()->setMouseTracking(1);
	m_pPicker = new QwtPlotPicker(plotF->xBottom, plotF->yLeft,
#if QWT_VER<6
		QwtPlotPicker::PointSelection,
#endif
		QwtPlotPicker::NoRubberBand,
#if QWT_VER>=6
		QwtPlotPicker::AlwaysOff,
#else
		QwtPlotPicker::AlwaysOn,
#endif
		plotF->canvas());

#if QWT_VER>=6
	m_pPicker->setStateMachine(new QwtPickerTrackerMachine());
	connect(m_pPicker, SIGNAL(moved(const QPointF&)), this, SLOT(cursorMoved(const QPointF&)));
#endif
	m_pPicker->setEnabled(1);

	plotF->setAxisTitle(QwtPlot::xBottom, "Scattering Wavenumber Q (1/A)");
	plotF->setAxisTitle(QwtPlot::yLeft, "Atomic Form Factor f (e-)");


	// scattering lengths
	plotSc->setCanvasBackground(colorBck);

	m_pGridSc = new QwtPlotGrid();
	m_pGridSc->setPen(penGrid);
	m_pGridSc->attach(plotSc);

#if QWT_VER>=6
	m_pZoomerSc = new QwtPlotZoomer(plotSc->canvas());
	m_pZoomerSc->setMaxStackDepth(-1);
	m_pZoomerSc->setEnabled(1);
#endif

	m_pCurveSc = new QwtPlotCurve("Scattering Lengths");
	m_pCurveSc->setPen(penCurve);
	m_pCurveSc->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	m_pCurveSc->attach(plotSc);

	plotSc->canvas()->setMouseTracking(1);
	m_pPickerSc = new QwtPlotPicker(plotSc->xBottom, plotSc->yLeft,
#if QWT_VER<6
		QwtPlotPicker::PointSelection,
#endif
		QwtPlotPicker::NoRubberBand,
#if QWT_VER>=6
		QwtPlotPicker::AlwaysOff,
#else
		QwtPlotPicker::AlwaysOn,
#endif
		plotSc->canvas());

#if QWT_VER>=6
	m_pPickerSc->setStateMachine(new QwtPickerTrackerMachine());
	connect(m_pPickerSc, SIGNAL(moved(const QPointF&)), this, SLOT(cursorMoved(const QPointF&)));
#endif
	m_pPickerSc->setEnabled(1);

	plotSc->setAxisTitle(QwtPlot::xBottom, "Element");
	plotSc->setAxisTitle(QwtPlot::yLeft, "Scattering Length b (fm)");



	QObject::connect(listAtoms, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
		this, SLOT(AtomSelected(QListWidgetItem*, QListWidgetItem*)));
	QObject::connect(editFilter, SIGNAL(textEdited(const QString&)),
		this, SLOT(SearchAtom(const QString&)));

	QObject::connect(radioCoherent, SIGNAL(toggled(bool)),
		this, SLOT(PlotScatteringLengths()));


	if(m_pSettings && m_pSettings->contains("formfactors/geo"))
		restoreGeometry(m_pSettings->value("formfactors/geo").toByteArray());


	SearchAtom("H");
	//radioCoherent->setChecked(1);
	PlotScatteringLengths();
}

FormfactorDlg::~FormfactorDlg()
{
	for(QwtPlotPicker** pPicker : {&m_pPicker, &m_pPickerSc})
		if(*pPicker)
		{
			(*pPicker)->setEnabled(0);
			delete (*pPicker);
			*pPicker = nullptr;
		}

	for(QwtPlotGrid** pGrid : {&m_pGrid, &m_pGridSc})
		if(*pGrid)
		{
			delete (*pGrid);
			*pGrid = nullptr;
		}

	for(QwtPlotZoomer** pZoomer : {&m_pZoomer, &m_pZoomerSc})
		if(*pZoomer)
		{
			delete (*pZoomer);
			*pZoomer = nullptr;
		}
}


static QListWidgetItem* create_header_item(const char *pcTitle, bool bSubheader=0)
{
	QListWidgetItem *pHeaderItem = new QListWidgetItem(pcTitle);
	pHeaderItem->setTextAlignment(Qt::AlignHCenter);

	QFont fontHeader = pHeaderItem->font();
	fontHeader.setBold(1);
	pHeaderItem->setFont(fontHeader);

	QBrush brushHeader = pHeaderItem->foreground();
	brushHeader.setColor(QColor(0xff, 0xff, 0xff));
	pHeaderItem->setForeground(brushHeader);

	pHeaderItem->setData(Qt::UserRole, 0);
	pHeaderItem->setBackgroundColor(bSubheader ? QColor(0x85, 0x85, 0x85) : QColor(0x65, 0x65, 0x65));

	return pHeaderItem;
}

void FormfactorDlg::SetupAtoms()
{
	FormfactList lstff;
	listAtoms->addItem(create_header_item("Atoms"));
	for(unsigned int iFF=0; iFF<lstff.GetNumAtoms(); ++iFF)
	{
		if(iFF==0) listAtoms->addItem(create_header_item("Period 1", 1));
		else if(iFF==2) listAtoms->addItem(create_header_item("Period 2", 1));
		else if(iFF==10) listAtoms->addItem(create_header_item("Period 3", 1));
		else if(iFF==18) listAtoms->addItem(create_header_item("Period 4", 1));
		else if(iFF==36) listAtoms->addItem(create_header_item("Period 5", 1));
		else if(iFF==54) listAtoms->addItem(create_header_item("Period 6", 1));
		else if(iFF==86) listAtoms->addItem(create_header_item("Period 7", 1));

		const Formfact<double>& ff = lstff.GetAtom(iFF);
		const std::string& strAtom = ff.GetAtomName();

		std::ostringstream ostrAtom;
		ostrAtom << (iFF+1) << " " << strAtom;
		QListWidgetItem* pItem = new QListWidgetItem(ostrAtom.str().c_str());
		pItem->setData(Qt::UserRole, 1);
		pItem->setData(Qt::UserRole+1, iFF);
		listAtoms->addItem(pItem);
	}

	listAtoms->addItem(create_header_item("Ions"));
	for(unsigned int iFF=0; iFF<lstff.GetNumIons(); ++iFF)
	{
		const Formfact<double>& ff = lstff.GetIon(iFF);
		const std::string& strAtom = ff.GetAtomName();

		std::ostringstream ostrAtom;
		ostrAtom << strAtom;
		QListWidgetItem* pItem = new QListWidgetItem(ostrAtom.str().c_str());
		pItem->setData(Qt::UserRole, 2);
		pItem->setData(Qt::UserRole+1, iFF);
		listAtoms->addItem(pItem);
	}
}

void FormfactorDlg::AtomSelected(QListWidgetItem *pItem, QListWidgetItem*)
{
	if(!pItem) return;
	const unsigned int iAtomOrIon = pItem->data(Qt::UserRole).toUInt();
	const unsigned int iAtom = pItem->data(Qt::UserRole+1).toUInt();

	const unsigned int NUM_POINTS = 512;

	double dMinQ = 0.;
	double dMaxQ = 25.;

	m_vecQ.clear();
	m_vecFF.clear();

	m_vecQ.reserve(NUM_POINTS);
	m_vecFF.reserve(NUM_POINTS);

	FormfactList lstff;
	if((iAtomOrIon==1 && iAtom < lstff.GetNumAtoms()) ||
		(iAtomOrIon==2 && iAtom < lstff.GetNumIons()))
	{
		const Formfact<double>& ff = (iAtomOrIon==1 ? lstff.GetAtom(iAtom) : lstff.GetIon(iAtom));

		for(unsigned int iPt=0; iPt<NUM_POINTS; ++iPt)
		{
			const double dQ = (dMinQ + (dMaxQ - dMinQ)/double(NUM_POINTS)*double(iPt));
			const double dFF = ff.GetFormfact(dQ);

			m_vecQ.push_back(dQ);
			m_vecFF.push_back(dFF);
		}
	}

#if QWT_VER>=6
	m_pCurve->setRawSamples(m_vecQ.data(), m_vecFF.data(), m_vecQ.size());
#else
	m_pCurve->setRawData(m_vecQ.data(), m_vecFF.data(), m_vecQ.size());
#endif

	set_zoomer_base(m_pZoomer, m_vecQ, m_vecFF);
	plotF->replot();
}


void FormfactorDlg::PlotScatteringLengths()
{
	const bool bCoh = radioCoherent->isChecked();

	m_vecElem.clear();
	m_vecSc.clear();

	ScatlenList lstsc;
	for(unsigned int iAtom=0; iAtom<lstsc.GetNumElems(); ++iAtom)
	{
		const ScatlenList::elem_type& sc = lstsc.GetElem(iAtom);
		std::complex<double> b = bCoh ? sc.GetCoherent() : sc.GetIncoherent();

		m_vecElem.push_back(double(iAtom+1));
		m_vecSc.push_back(b.real());
	}

#if QWT_VER>=6
	m_pCurveSc->setRawSamples(m_vecElem.data(), m_vecSc.data(), m_vecElem.size());
#else
	m_pCurveSc->setRawData(m_vecElem.data(), m_vecSc.data(), m_vecElem.size());
#endif

	set_zoomer_base(m_pZoomerSc, m_vecElem, m_vecSc);
	plotSc->replot();
}

void FormfactorDlg::cursorMoved(const QPointF& pt)
{
	if(tabWidget->currentIndex() == 0)
	{
		std::wstring strX = std::to_wstring(pt.x());
		std::wstring strY = std::to_wstring(pt.y());

		const std::wstring strAA = tl::get_spec_char_utf16("AA") +
			tl::get_spec_char_utf16("sup-") +
			tl::get_spec_char_utf16("sup1");

		std::wostringstream ostr;
		ostr << L"Q = " << strX << L" " << strAA;
		ostr << L", f = " << strY;

		labelStatus->setText(QString::fromWCharArray(ostr.str().c_str()));
	}
	else if(tabWidget->currentIndex() == 1)
	{
		ScatlenList lst;

		int iElem = std::round(pt.x());
		if(iElem<=0 || iElem>=int(lst.GetNumElems()))
		{
			labelStatus->setText("");
			return;
		}

		const std::string& strName = lst.GetElem(unsigned(iElem-1)).GetAtomName();
		std::complex<double> b_coh = lst.GetElem(unsigned(iElem-1)).GetCoherent();
		std::complex<double> b_inc = lst.GetElem(unsigned(iElem-1)).GetIncoherent();

		std::ostringstream ostr;
		ostr << iElem << ", " << strName
			<< ": b_coh = " << b_coh << ", b_inc = " << b_inc;
		labelStatus->setText(ostr.str().c_str());
	}
}

void FormfactorDlg::closeEvent(QCloseEvent* pEvt)
{
	if(m_pSettings)
		m_pSettings->setValue("formfactors/geo", saveGeometry());
}


void FormfactorDlg::SearchAtom(const QString& qstr)
{
	QList<QListWidgetItem*> lstItems = listAtoms->findItems(qstr, Qt::MatchContains);
	if(lstItems.size())
		listAtoms->setCurrentItem(lstItems[0], QItemSelectionModel::SelectCurrent);
}


#include "FormfactorDlg.moc"
