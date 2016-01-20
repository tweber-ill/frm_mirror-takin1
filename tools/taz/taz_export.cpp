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


//--------------------------------------------------------------------------------
// image exports

void TazDlg::ExportReal() { ExportSceneSVG(m_sceneReal); }
void TazDlg::ExportRecip() { ExportSceneSVG(m_sceneRecip); }

void TazDlg::ExportSceneSVG(QGraphicsScene& scene)
{
	QString strDirLast = m_settings.value("main/last_dir_export", ".").toString();
	QString strFile = QFileDialog::getSaveFileName(this,
		"Export SVG", strDirLast, "SVG files (*.svg *.SVG)");
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
	QString strDirLast = m_settings.value("main/last_dir_export", ".").toString();
	QString strFile = QFileDialog::getSaveFileName(this,
		"Export PNG", strDirLast, "PNG files (*.png *.PNG)");
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
	QString strDirLast = m_settings.value("main/last_dir_export", ".").toString();
	QString strFile = QFileDialog::getSaveFileName(this,
		"Export PNG", strDirLast, "PNG files (*.png *.PNG)");
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
	using t_mat = ublas::matrix<double>;
	using t_vec = ublas::vector<double>;


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

	std::vector<t_mat> vecTrafos;
	pSpaceGroup->GetSymTrafos(vecTrafos);

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


	const double a = editA->text().toDouble();
	const double b = editB->text().toDouble();
	const double c = editC->text().toDouble();
	const double alpha = tl::d2r(editAlpha->text().toDouble());
	const double beta = tl::d2r(editBeta->text().toDouble());
	const double gamma = tl::d2r(editGamma->text().toDouble());
	const tl::Lattice<double> lattice(a,b,c, alpha,beta,gamma);
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



	QString strDirLast = m_settings.value("main/last_dir_export", ".").toString();
	QString strFile = QFileDialog::getSaveFileName(this,
		"Export X3D", strDirLast, "X3D files (*.x3d *.X3D)");
	if(strFile == "")
		return;


	tl::X3d x3d;
	std::ostringstream ostrComment;

	for(std::size_t iAtom=0; iAtom<vecAtoms.size(); ++iAtom)
	{
		const t_vec& vecAtom = vecAtoms[iAtom];
		const std::string& strAtomName = vecAtomNames[iAtom];

		const double dUCSize = 1.;
		std::vector<t_vec> vecPos =
			tl::generate_atoms<t_mat, t_vec, std::vector>(vecTrafos, vecAtom, -dUCSize*0.5, dUCSize*0.5);
		ostrComment << "Unit cell contains " << vecPos.size() << " " << strAtomName
			<< " atoms (color: " << vecColors[iAtom % vecColors.size()] <<  ").\n";

		for(std::size_t iPos=0; iPos<vecPos.size(); ++iPos)
		{
			t_vec vecCoord = vecPos[iPos];

			vecCoord.resize(3,1);
			vecCoord = matA * vecCoord;
			vecCoord.resize(4,1); vecCoord[3] = 1.;

			tl::X3dTrafo *pTrafo = new tl::X3dTrafo();
			pTrafo->SetTrans(matGlobal * vecCoord);
			tl::X3dSphere *pSphere = new tl::X3dSphere(0.1);
			pSphere->SetColor(vecColors[iAtom % vecColors.size()]);
			pTrafo->AddChild(pSphere);

			x3d.GetScene().AddChild(pTrafo);
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
