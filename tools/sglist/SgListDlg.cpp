/*
 * Space Group List Dialog
 * @author tweber
 * @date oct-2015
 * @license GPLv2
 */
 
#include "SgListDlg.h"
#include <sstream>
//#include <iostream>
#include <clipper/clipper.h>
#include "tlibs/string/string.h"


SgListDlg::SgListDlg(QWidget *pParent)
	: QDialog(pParent, Qt::WindowTitleHint|Qt::WindowCloseButtonHint|Qt::WindowMinMaxButtonsHint), 
		m_settings("tobis_stuff", "sglist")
{
	setupUi(this);
	SetupSpacegroups();
	
	QObject::connect(listSGs, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
		this, SLOT(SGSelected(QListWidgetItem*, QListWidgetItem*)));

	for(QSpinBox* pSpin : {spinH, spinK, spinL})
		QObject::connect(pSpin, SIGNAL(valueChanged(int)), this, SLOT(RecalcBragg()));
}

SgListDlg::~SgListDlg()
{
}


void SgListDlg::SetupSpacegroups()
{
	listSGs->clear();
	
	// actually: space group TYPE, not space group...
	for(int iSg=1; iSg<=230; ++iSg)
	{
		clipper::Spgr_descr dsc(iSg);
		
		std::ostringstream ostrSg;
		ostrSg << "No. " << iSg << ": ";
		ostrSg << dsc.symbol_hm();
		
		QListWidgetItem* pItem = new QListWidgetItem(ostrSg.str().c_str());
		pItem->setData(Qt::UserRole, iSg);
		listSGs->addItem(pItem);
	}
}

static inline std::string get_stdstring(const clipper::String& str)
{
	std::ostringstream ostr;
	ostr << str;
	return ostr.str();
}

void SgListDlg::SGSelected(QListWidgetItem *pItem, QListWidgetItem *pItemPrev)
{
	listSymOps->clear();
	
	if(!pItem) return;
	int iSG = pItem->data(Qt::UserRole).toInt();
	
	clipper::Spgr_descr dsc(iSG);
	clipper::Spacegroup sg(dsc);
	
	editNr->setText(tl::var_to_str(iSG).c_str());
	editHM->setText(get_stdstring(sg.symbol_hm()).c_str());
	editHall->setText(get_stdstring(sg.symbol_hall()).c_str());
	editLaue->setText(get_stdstring(sg.symbol_laue()).c_str());
	
	for(int iSymOp=0; iSymOp<sg.num_symops(); ++iSymOp)
	{
		const clipper::Symop& symop = sg.symop(iSymOp);
		listSymOps->addItem(get_stdstring(symop.format()).c_str());
	}
	
	RecalcBragg();
}

void SgListDlg::RecalcBragg()
{
	const QListWidgetItem* pItem = listSGs->currentItem();
	if(!pItem) return;
	const int iSG = pItem->data(Qt::UserRole).toInt();
	
	const int h = spinH->value();
	const int k = spinK->value();
	const int l = spinL->value();
	
	//std::cout << h << k << l << std::endl;
	
	clipper::Spgr_descr dsc(iSG);
	clipper::Spacegroup sg(dsc);
	clipper::HKL_class hkl = sg.hkl_class(clipper::HKL(h,k,l));
	const bool bForbidden = hkl.sys_abs();

	QFont font = spinH->font();
	font.setStrikeOut(bForbidden);
	for(QSpinBox* pSpin : {spinH, spinK, spinL})
		pSpin->setFont(font);
}


#include "SgListDlg.moc"
