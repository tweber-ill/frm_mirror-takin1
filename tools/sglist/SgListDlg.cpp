/*
 * Space Group List Dialog
 * @author tweber
 * @date oct-2015
 * @license GPLv2
 */

#include "SgListDlg.h"
#include <sstream>
//#include <iostream>
#include "tlibs/string/string.h"
#include "helper/spacegroup.h"


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

	pHeaderItem->setData(Qt::UserRole, 1000);
	pHeaderItem->setBackgroundColor(QColor(0x65, 0x65, 0x65));

	return pHeaderItem;
}

void SgListDlg::SetupSpacegroups()
{
	listSGs->clear();

	const t_vecSpaceGroups* pvecSG = get_space_groups_vec();

	// actually: space group TYPE, not space group...
	for(unsigned int iSG=0; iSG<pvecSG->size(); ++iSG)
	{
		const SpaceGroup* sg = pvecSG->at(iSG);
		unsigned int iSgNr = sg->GetNr();

		// list headers
		if(iSgNr==1) listSGs->addItem(create_header_item("Triclinic"));
		else if(iSgNr==3) listSGs->addItem(create_header_item("Monoclinic"));
		else if(iSgNr==16) listSGs->addItem(create_header_item("Orthorhombic"));
		else if(iSgNr==75) listSGs->addItem(create_header_item("Tetragonal"));
		else if(iSgNr==143) listSGs->addItem(create_header_item("Trigonal"));
		else if(iSgNr==168) listSGs->addItem(create_header_item("Hexagonal"));
		else if(iSgNr==195) listSGs->addItem(create_header_item("Cubic"));


		std::ostringstream ostrSg;
		ostrSg << "No. " << iSgNr << ": " << sg->GetName();

		QListWidgetItem* pItem = new QListWidgetItem(ostrSg.str().c_str());
		pItem->setData(Qt::UserRole, iSG);
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
	for(QLineEdit *pEdit : {editHM, /*editHall,*/ editLaue, editNr/*, editAxisSym*/})
		pEdit->setText("");
	if(!pItem) return;


	const t_vecSpaceGroups* pvecSG = get_space_groups_vec();

	// header selected?
	unsigned int iSG = pItem->data(Qt::UserRole).toUInt();
	if(iSG >= pvecSG->size())
		return;

	const SpaceGroup* sg = pvecSG->at(iSG);
	unsigned int iSgNr = sg->GetNr();

	const std::string& strHM = sg->GetName();
	const std::string& strPointGroup = sg->GetPointGroup();
	const std::string& strLaue = sg->GetLaueGroup();
	const std::string& strCrysSys = sg->GetCrystalSystemName();

	editNr->setText(tl::var_to_str(iSgNr).c_str());
	editHM->setText(strHM.c_str());
	//editHall->setText(sg.symbol_hall().c_str());
	editLaue->setText(("PG: " + strPointGroup + ", LG: " + strLaue +
		" (" + strCrysSys + ")").c_str());

	bool bShowMatrices = checkMatrices->isChecked();
	{
		const std::vector<SpaceGroup::t_mat>& vecTrafos = sg->GetTrafos();

		std::ostringstream ostr;
		ostr << "All Symmetry Operations (" << vecTrafos.size() << ")";
		listSymOps->addItem(create_header_item(ostr.str().c_str()));

		for(unsigned int iSymOp=0; iSymOp<vecTrafos.size(); ++iSymOp)
		{
			const SpaceGroup::t_mat& mat = vecTrafos[iSymOp];

			if(bShowMatrices)
				listSymOps->addItem(print_matrix(mat).c_str());
			else
				listSymOps->addItem(get_trafo_desc(mat).c_str());
		}
	}
	/*if(sg.num_primitive_symops())
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
		ostr << "Inverting Symmetry Operations (" << (sg.num_inversion_symops()) << ")";
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
		ostr << "Centering Symmetry Operations (" << (sg.num_centering_symops()) << ")";
		listSymOps->addItem(create_header_item(ostr.str().c_str()));
		for(int iSymOp=0; iSymOp<sg.num_centering_symops(); ++iSymOp)
		{
			const clipper::Symop& symop = sg.centering_symop(iSymOp);
			if(bShowMatrices)
				listSymOps->addItem(print_matrix(symop_to_matrix(symop)).c_str());
			else
				listSymOps->addItem(symop.format().c_str());
		}
	}*/

	RecalcBragg();
}

void SgListDlg::RecalcBragg()
{
	const QListWidgetItem* pItem = listSGs->currentItem();
	if(!pItem) return;

	const int h = spinH->value();
	const int k = spinK->value();
	const int l = spinL->value();

	//std::cout << h << k << l << std::endl;

	const t_vecSpaceGroups* pvecSG = get_space_groups_vec();
	const unsigned int iSG = pItem->data(Qt::UserRole).toUInt();
	if(iSG >= pvecSG->size())
		return;

	const SpaceGroup* sg = pvecSG->at(iSG);
	const bool bForbidden = !sg->HasReflection(h,k,l);;

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
