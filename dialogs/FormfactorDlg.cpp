/*
 * Form Factor Dialog
 * @author tweber
 * @date nov-2015
 * @license GPLv2
 */

#include "FormfactorDlg.h"
#include "helper/formfact.h"
#include "tlibs/string/spec_char.h"

#include <qwt_picker_machine.h>


FormfactorDlg::FormfactorDlg(QWidget* pParent, QSettings *pSettings)
	: QDialog(pParent), m_pSettings(pSettings)
{
	this->setupUi(this);
	SetupAtoms();
	
	QColor colorBck(240, 240, 240, 255);
	plotF->setCanvasBackground(colorBck);

	m_pGrid = new QwtPlotGrid();
	QPen penGrid;
	penGrid.setColor(QColor(0x99,0x99,0x99));
	penGrid.setStyle(Qt::DashLine);
	m_pGrid->setPen(penGrid);
	m_pGrid->attach(plotF);

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
	plotF->setAxisTitle(QwtPlot::yLeft, "Atomic Form Factor f");	




	QObject::connect(listAtoms, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
		this, SLOT(AtomSelected(QListWidgetItem*, QListWidgetItem*)));
	
	QObject::connect(editFilter, SIGNAL(textEdited(const QString&)),
		this, SLOT(SearchAtom(const QString&)));


	if(m_pSettings && m_pSettings->contains("formfactors/geo"))
		restoreGeometry(m_pSettings->value("formfactors/geo").toByteArray());


	SearchAtom("H");
}

FormfactorDlg::~FormfactorDlg()
{
	if(m_pPicker)
	{
		m_pPicker->setEnabled(0);
		delete m_pPicker;
		m_pPicker = nullptr;
	}

	if(m_pGrid)
	{
		delete m_pGrid;
		m_pGrid = nullptr;
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

	pHeaderItem->setData(Qt::UserRole, 1000);	// set invalid atom number
	pHeaderItem->setBackgroundColor(bSubheader ? QColor(0x85, 0x85, 0x85) : QColor(0x65, 0x65, 0x65));

	return pHeaderItem;
}

void FormfactorDlg::SetupAtoms()
{
	bool bHadIonsHeader = 0, bIonsBegin = 0;
	
	FormfactList lstff;
	for(unsigned int iFF=0; iFF<lstff.GetNumFormfacts(); ++iFF)
	{
		if(iFF==0)
		{
			listAtoms->addItem(create_header_item("Atoms"));
			listAtoms->addItem(create_header_item("Period 1", 1));
		}
		else if(iFF==2) listAtoms->addItem(create_header_item("Period 2", 1));
		else if(iFF==10) listAtoms->addItem(create_header_item("Period 3", 1));
		else if(iFF==18) listAtoms->addItem(create_header_item("Period 4", 1));
		else if(iFF==36) listAtoms->addItem(create_header_item("Period 5", 1));
		else if(iFF==54) listAtoms->addItem(create_header_item("Period 6", 1));
		else if(iFF==86) listAtoms->addItem(create_header_item("Period 7", 1));

		const Formfact<double>& ff = lstff.GetFormfact(iFF);
		const std::string& strAtom = ff.GetAtomName();

		if(!bHadIonsHeader && (strAtom.find('+')!=std::string::npos || strAtom.find('-')!=std::string::npos))
			bIonsBegin = 1;
		if(bIonsBegin && !bHadIonsHeader)
		{
			listAtoms->addItem(create_header_item("Ions"));
			bHadIonsHeader = 1;
		}
		
		std::ostringstream ostrAtom;
		if(!bHadIonsHeader)
			ostrAtom << (iFF+1) << " ";
		ostrAtom << strAtom;
		QListWidgetItem* pItem = new QListWidgetItem(ostrAtom.str().c_str());
		pItem->setData(Qt::UserRole, iFF);
		listAtoms->addItem(pItem);
	}
}

void FormfactorDlg::AtomSelected(QListWidgetItem *pItem, QListWidgetItem*)
{
	if(!pItem) return;
	const unsigned int iAtom = pItem->data(Qt::UserRole).toUInt();

	const unsigned int NUM_POINTS = 512;

	double dMinQ = 0.;
	double dMaxQ = 25.;

	m_vecQ.clear();
	m_vecFF.clear();

	m_vecQ.reserve(NUM_POINTS);
	m_vecFF.reserve(NUM_POINTS);
	
	FormfactList lstff;
	if(iAtom < lstff.GetNumFormfacts())
	{
		const Formfact<double>& ff = lstff.GetFormfact(iAtom);

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

	plotF->replot();
}


void FormfactorDlg::cursorMoved(const QPointF& pt)
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
