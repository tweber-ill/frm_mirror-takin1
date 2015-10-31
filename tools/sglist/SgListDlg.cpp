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
#include "helper/spacegroup_clp.h"


SgListDlg::SgListDlg(QWidget *pParent)
	: QDialog(pParent, Qt::WindowTitleHint|Qt::WindowCloseButtonHint|Qt::WindowMinMaxButtonsHint),
		m_settings("tobis_stuff", "sglist")
{
	setupUi(this);
	SetupSpacegroups();


	QObject::connect(listSGs, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
		this, SLOT(SGSelected(QListWidgetItem*, QListWidgetItem*)));
	QObject::connect(checkMatrices, SIGNAL(toggled(bool)), this, SLOT(UpdateSG()));

	for(QSpinBox* pSpin : {spinH, spinK, spinL})
		QObject::connect(pSpin, SIGNAL(valueChanged(int)), this, SLOT(RecalcBragg()));

	QObject::connect(editFilter, SIGNAL(textEdited(const QString&)),
		this, SLOT(SearchSG(const QString&)));


	if(m_settings.contains("sglist/geo"))
		restoreGeometry(m_settings.value("sglist/geo").toByteArray());
}

SgListDlg::~SgListDlg()
{}

void SgListDlg::closeEvent(QCloseEvent* pEvt)
{
	m_settings.setValue("sglist/geo", saveGeometry());
}


static QListWidgetItem* create_header_item(const char *pcTitle)
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
	pHeaderItem->setBackgroundColor(QColor(0x65, 0x65, 0x65));

	return pHeaderItem;
}

void SgListDlg::SetupSpacegroups()
{
	listSGs->clear();

	// actually: space group TYPE, not space group...
	for(int iSg=1; iSg<=230; ++iSg)
	{
		// list headers
		if(iSg==1) listSGs->addItem(create_header_item("Triclinic"));
		else if(iSg==3) listSGs->addItem(create_header_item("Monoclinic"));
		else if(iSg==16) listSGs->addItem(create_header_item("Orthorhombic"));
		else if(iSg==75) listSGs->addItem(create_header_item("Tetragonal"));
		else if(iSg==143) listSGs->addItem(create_header_item("Trigonal"));
		else if(iSg==168) listSGs->addItem(create_header_item("Hexagonal"));
		else if(iSg==195) listSGs->addItem(create_header_item("Cubic"));


		clipper::Spgr_descr dsc(iSg);

		std::string strSg = dsc.symbol_hm();
		convert_hm_symbol(strSg);

		std::ostringstream ostrSg;
		ostrSg << "No. " << iSg << ": " << strSg;

		QListWidgetItem* pItem = new QListWidgetItem(ostrSg.str().c_str());
		pItem->setData(Qt::UserRole, iSg);
		listSGs->addItem(pItem);
	}
}

void SgListDlg::UpdateSG()
{
	SGSelected(listSGs->currentItem(), nullptr);
}

void SgListDlg::SGSelected(QListWidgetItem *pItem, QListWidgetItem*)
{
	listSymOps->clear();
	for(QLineEdit *pEdit : {editHM, editHall, editLaue, editNr, editAxisSym})
		pEdit->setText("");
	if(!pItem) return;

	// header selected?
	int iSG = pItem->data(Qt::UserRole).toInt();
	if(iSG <= 0)
		return;

	clipper::Spgr_descr dsc(iSG);
	clipper::Spacegroup sg(dsc);

	std::string strLaue = sg.symbol_laue();
	std::string strCrysSys = get_crystal_system_name(get_crystal_system_from_laue_group(strLaue.c_str()));

	editNr->setText(tl::var_to_str(iSG).c_str());
	editHM->setText(sg.symbol_hm().c_str());
	editHall->setText(sg.symbol_hall().c_str());
	editLaue->setText((strLaue + " (" + strCrysSys + ")").c_str());

	const int iSymAxisA = sg.order_of_symmetry_about_axis(clipper::Spacegroup::A);
	const int iSymAxisB = sg.order_of_symmetry_about_axis(clipper::Spacegroup::B);
	const int iSymAxisC = sg.order_of_symmetry_about_axis(clipper::Spacegroup::C);
	std::ostringstream ostrAxisSym;
	ostrAxisSym << "a: " << iSymAxisA << "-fold, "
		<< "b: " << iSymAxisB << "-fold, "
		<< "c: " << iSymAxisC << "-fold";
	editAxisSym->setText(ostrAxisSym.str().c_str());

	bool bShowMatrices = checkMatrices->isChecked();

	{
		std::ostringstream ostr;
		ostr << "All Symmetry Operations (" << sg.num_symops() << ")";
		listSymOps->addItem(create_header_item(ostr.str().c_str()));
		for(int iSymOp=0; iSymOp<sg.num_symops(); ++iSymOp)
		{
			const clipper::Symop& symop = sg.symop(iSymOp);
			if(bShowMatrices)
				listSymOps->addItem(print_matrix(symop_to_matrix(symop)).c_str());
			else
				listSymOps->addItem(symop.format().c_str());
		}
	}
	if(sg.num_primitive_symops())
	{
		std::ostringstream ostr;
		ostr << "Primitive Symmetry Operations (" << sg.num_primitive_symops() << ")";
		listSymOps->addItem(create_header_item(ostr.str().c_str()));
		for(int iSymOp=0; iSymOp<sg.num_primitive_symops(); ++iSymOp)
		{
			const clipper::Symop& symop = sg.primitive_symop(iSymOp);
			if(bShowMatrices)
				listSymOps->addItem(print_matrix(symop_to_matrix(symop)).c_str());
			else
				listSymOps->addItem(symop.format().c_str());
		}
	}
	if(sg.num_inversion_symops())
	{
		std::ostringstream ostr;
		ostr << "Inverting Symmetry Operations (" << sg.num_inversion_symops() << ")";
		listSymOps->addItem(create_header_item(ostr.str().c_str()));
		for(int iSymOp=0; iSymOp<sg.num_inversion_symops(); ++iSymOp)
		{
			const clipper::Symop& symop = sg.inversion_symop(iSymOp);
			if(bShowMatrices)
				listSymOps->addItem(print_matrix(symop_to_matrix(symop)).c_str());
			else
				listSymOps->addItem(symop.format().c_str());
		}
	}
	if(sg.num_centering_symops())
	{
		std::ostringstream ostr;
		ostr << "Centering Symmetry Operations (" << sg.num_centering_symops() << ")";
		listSymOps->addItem(create_header_item(ostr.str().c_str()));
		for(int iSymOp=0; iSymOp<sg.num_centering_symops(); ++iSymOp)
		{
			const clipper::Symop& symop = sg.centering_symop(iSymOp);
			if(bShowMatrices)
				listSymOps->addItem(print_matrix(symop_to_matrix(symop)).c_str());
			else
				listSymOps->addItem(symop.format().c_str());
		}
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

void SgListDlg::SearchSG(const QString& qstr)
{
	QList<QListWidgetItem*> lstItems = listSGs->findItems(qstr, Qt::MatchContains);
	if(lstItems.size())
		listSGs->setCurrentItem(lstItems[0], QItemSelectionModel::SelectCurrent);
}


#include "SgListDlg.moc"
