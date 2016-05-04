/*
 * exports
 * @author tweber
 * @date dec-2015
 * @license GPLv2
 */

#include "taz.h"
#include "tlibs/math/atoms.h"
#include "tlibs/file/x3d.h"
#include "tlibs/log/log.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QtSvg/QSvgGenerator>


using t_real = t_real_glob;

//--------------------------------------------------------------------------------
// image exports

void TazDlg::ExportReal()
{
	TasLayout *pTas = m_sceneReal.GetTasLayout();

	const t_real dZoom = pTas->GetZoom();
	pTas->SetZoom(1.);
	ExportSceneSVG(m_sceneReal);
	pTas->SetZoom(dZoom);
}

void TazDlg::ExportTof()
{
	TofLayout *pTof = m_sceneTof.GetTofLayout();

	const t_real dZoom = pTof->GetZoom();
	pTof->SetZoom(1.);
	ExportSceneSVG(m_sceneTof);
	pTof->SetZoom(dZoom);
}

void TazDlg::ExportRealLattice()
{
	RealLattice *pLatt = m_sceneRealLattice.GetLattice();

	const t_real dZoom = pLatt->GetZoom();
	pLatt->SetZoom(1.);
	ExportSceneSVG(m_sceneRealLattice);
	pLatt->SetZoom(dZoom);
}

void TazDlg::ExportRecip()
{
	ScatteringTriangle *pTri = m_sceneRecip.GetTriangle();

	const t_real dZoom = pTri->GetZoom();
	pTri->SetZoom(1.);
	ExportSceneSVG(m_sceneRecip);
	pTri->SetZoom(dZoom);
}

void TazDlg::ExportSceneSVG(QGraphicsScene& scene)
{
	QFileDialog::Option fileopt = QFileDialog::Option(0);
	if(!m_settings.value("main/native_dialogs", 1).toBool())
		fileopt = QFileDialog::DontUseNativeDialog;

	QString strDirLast = m_settings.value("main/last_dir_export", ".").toString();
	QString strFile = QFileDialog::getSaveFileName(this,
		"Export SVG", strDirLast, "SVG files (*.svg *.SVG)", nullptr, fileopt);
	if(strFile == "")
		return;

	QRectF rect = scene.sceneRect();

	QSvgGenerator svg;
	svg.setFileName(strFile);
	svg.setSize(QSize(rect.width(), rect.height()));
	//svg.setResolution(300);
	svg.setViewBox(QRectF(0, 0, rect.width(), rect.height()));
	svg.setDescription("Created with Takin");

	QPainter painter;
	painter.begin(&svg);
	scene.render(&painter);
	painter.end();

	std::string strDir = tl::get_dir(strFile.toStdString());
	m_settings.setValue("main/last_dir_export", QString(strDir.c_str()));
}

#ifdef USE_GIL
void TazDlg::ExportBZImage()
{
	QFileDialog::Option fileopt = QFileDialog::Option(0);
	if(!m_settings.value("main/native_dialogs", 1).toBool())
		fileopt = QFileDialog::DontUseNativeDialog;

	QString strDirLast = m_settings.value("main/last_dir_export", ".").toString();
	QString strFile = QFileDialog::getSaveFileName(this,
		"Export PNG", strDirLast, "PNG files (*.png *.PNG)", nullptr, fileopt);
	if(strFile == "")
		return;

	bool bOk = m_sceneRecip.ExportBZAccurate(strFile.toStdString().c_str());
	if(!bOk)
		QMessageBox::critical(this, "Error", "Could not export image.");

	if(bOk)
	{
		std::string strDir = tl::get_dir(strFile.toStdString());
		m_settings.setValue("main/last_dir_export", QString(strDir.c_str()));
	}
}

void TazDlg::ExportWSImage()
{
	QFileDialog::Option fileopt = QFileDialog::Option(0);
	if(!m_settings.value("main/native_dialogs", 1).toBool())
		fileopt = QFileDialog::DontUseNativeDialog;

	QString strDirLast = m_settings.value("main/last_dir_export", ".").toString();
	QString strFile = QFileDialog::getSaveFileName(this,
		"Export PNG", strDirLast, "PNG files (*.png *.PNG)", nullptr, fileopt);
	if(strFile == "")
		return;

	bool bOk = m_sceneRealLattice.ExportWSAccurate(strFile.toStdString().c_str());
	if(!bOk)
		QMessageBox::critical(this, "Error", "Could not export image.");

	if(bOk)
	{
		std::string strDir = tl::get_dir(strFile.toStdString());
		m_settings.setValue("main/last_dir_export", QString(strDir.c_str()));
	}
}
#else
void TazDlg::ExportBZImage() {}
void TazDlg::ExportWSImage() {}
#endif



//--------------------------------------------------------------------------------
// 3d model exports

void TazDlg::ExportUCModel()
{
	using t_mat = ublas::matrix<t_real>;
	using t_vec = ublas::vector<t_real>;


	if(m_vecAtoms.size() == 0)
	{
		QMessageBox::critical(this, "Error", "No atom positions defined for unit cell.");
		return;
	}


	SpaceGroup *pSpaceGroup = nullptr;
	int iSpaceGroupIdx = comboSpaceGroups->currentIndex();
	if(iSpaceGroupIdx != 0)
		pSpaceGroup = (SpaceGroup*)comboSpaceGroups->itemData(iSpaceGroupIdx).value<void*>();
	if(!pSpaceGroup)
	{
		QMessageBox::critical(this, "Error", "Invalid space group.");
		return;
	}

	const std::vector<t_mat>& vecTrafos = pSpaceGroup->GetTrafos();

	std::vector<t_vec> vecAtoms;
	std::vector<std::string> vecAtomNames;
	for(const AtomPos& atom : m_vecAtoms)
	{
		t_vec vecAtomPos = atom.vecPos;
		vecAtomPos.resize(4, 1);
		vecAtomPos[3] = 1.;
		vecAtoms.push_back(std::move(vecAtomPos));

		vecAtomNames.push_back(atom.strAtomName);
	}


	const t_real a = editA->text().toDouble();
	const t_real b = editB->text().toDouble();
	const t_real c = editC->text().toDouble();
	const t_real alpha = tl::d2r(editAlpha->text().toDouble());
	const t_real beta = tl::d2r(editBeta->text().toDouble());
	const t_real gamma = tl::d2r(editGamma->text().toDouble());
	const tl::Lattice<t_real> lattice(a,b,c, alpha,beta,gamma);
	const t_mat matA = lattice.GetMetric();


	const std::vector<t_vec> vecColors =
	{
		tl::make_vec({1., 0., 0.}), tl::make_vec({0., 1., 0.}), tl::make_vec({0., 0., 1.}),
		tl::make_vec({1., 1., 0.}), tl::make_vec({0., 1., 1.}), tl::make_vec({1., 0., 1.}),
		tl::make_vec({0.5, 0., 0.}), tl::make_vec({0., 0.5, 0.}), tl::make_vec({0., 0., 0.5}),
		tl::make_vec({0.5, 0.5, 0.}), tl::make_vec({0., 0.5, 0.5}), tl::make_vec({0.5, 0., 0.5}),
		tl::make_vec({1., 1., 1.}), tl::make_vec({0., 0., 0.}), tl::make_vec({0.5, 0.5, 0.5}),
	};

	// to transform into program-specific coordinate systems
	const t_mat matGlobal = tl::make_mat(
	{	{-1., 0., 0., 0.},
		{ 0., 0., 1., 0.},
		{ 0., 1., 0., 0.},
		{ 0., 0., 0., 1.}	});


	QFileDialog::Option fileopt = QFileDialog::Option(0);
	if(!m_settings.value("main/native_dialogs", 1).toBool())
		fileopt = QFileDialog::DontUseNativeDialog;

	QString strDirLast = m_settings.value("main/last_dir_export", ".").toString();
	QString strFile = QFileDialog::getSaveFileName(this,
		"Export X3D", strDirLast, "X3D files (*.x3d *.X3D)", nullptr, fileopt);
	if(strFile == "")
		return;


	tl::X3d x3d;
	std::ostringstream ostrComment;

	std::vector<t_vec> vecAllAtomsFrac;
	for(std::size_t iAtom=0; iAtom<vecAtoms.size(); ++iAtom)
	{
		const t_vec& vecAtom = vecAtoms[iAtom];
		const std::string& strAtomName = vecAtomNames[iAtom];
		
		std::vector<t_vec> vecOtherAtoms = vecAtoms;
		vecOtherAtoms.erase(vecOtherAtoms.begin() + iAtom);

		const t_real dUCSize = 1.;
		std::vector<t_vec> vecPos =
			tl::generate_atoms<t_mat, t_vec, std::vector>(vecTrafos, vecAtom, -dUCSize*0.5, dUCSize*0.5);

		std::size_t iGeneratedAtoms = 0;
		for(std::size_t iPos=0; iPos<vecPos.size(); ++iPos)
		{
			t_vec vecCoord = vecPos[iPos];
			vecCoord.resize(3,1);
			
			// is the atom position in the unit cell still free?
			if(std::find_if(vecAllAtomsFrac.begin(), vecAllAtomsFrac.end(),
				[&vecCoord](const t_vec& _v) -> bool
				{ return tl::vec_equal(_v, vecCoord, g_dEps); }) == vecAllAtomsFrac.end() 
				&& // and is it not at a given initial atom position?
				std::find_if(vecOtherAtoms.begin(), vecOtherAtoms.end(),
				[&vecCoord](const t_vec& _v) -> bool
				{ return tl::vec_equal(_v, vecCoord, g_dEps); }) == vecOtherAtoms.end())
			{
				vecAllAtomsFrac.push_back(vecCoord);
				vecCoord = matA * vecCoord;
				vecCoord.resize(4,1); vecCoord[3] = 1.;

				tl::X3dTrafo *pTrafo = new tl::X3dTrafo();
				pTrafo->SetTrans(matGlobal * vecCoord);
				tl::X3dSphere *pSphere = new tl::X3dSphere(0.1);
				pSphere->SetColor(vecColors[iAtom % vecColors.size()]);
				pTrafo->AddChild(pSphere);

				x3d.GetScene().AddChild(pTrafo);
				++iGeneratedAtoms;
			}
			else
			{
				tl::log_warn("Position ", vecCoord, " is already occupied,",
					" skipping current ", strAtomName, " atom.");
			}
			
			ostrComment << "Unit cell contains " << iGeneratedAtoms /*vecPos.size()*/ 
				<< " " << strAtomName << " atoms (color: " 
				<< vecColors[iAtom % vecColors.size()] <<  ").\n";
		}
	}

	// only for cubic unit cells!
	//tl::X3dCube *pCube = new tl::X3dCube(a,b,c);
	//pCube->SetColor(tl::make_vec({1., 1., 1., 0.75}));
	//x3d.GetScene().AddChild(pCube);

	tl::log_info(ostrComment.str());
	x3d.SetComment(std::string("\nCreated with Takin.\n\n") + ostrComment.str());

	bool bOk = x3d.Save(strFile.toStdString().c_str());

	if(bOk)
	{
		std::string strDir = tl::get_dir(strFile.toStdString());
		m_settings.setValue("main/last_dir_export", QString(strDir.c_str()));
	}
	else
	{
		QMessageBox::critical(this, "Error", "Error exporting x3d file.");
	}
}
