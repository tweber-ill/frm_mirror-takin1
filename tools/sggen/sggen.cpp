// gcc -o sggen sggen.cpp -std=c++11 -lstdc++ -lm -I../.. -I/usr/include/QtGui/ ../../helper/spacegroup_clp.cpp ../../helper/crystalsys.cpp -DUSE_CLP -DNO_QT -lclipper-core ../../tlibs/file/x3d.cpp

/**
 * generates atom positions based on space group
 * @author tweber
 * @date nov-2015
 * @license GPLv2
 */

#include <clipper/clipper.h>
#include <vector>
#include <sstream>
#include "tlibs/math/atoms.h"
#include "tlibs/file/x3d.h"
#include "tlibs/string/string.h"
#include "helper/spacegroup_clp.h"


typedef tl::ublas::vector<double> t_vec;
typedef tl::ublas::matrix<double> t_mat;

void gen_atoms()
{
	const std::vector<t_vec> vecColors =
	{
		tl::make_vec({1., 0., 0.}),
		tl::make_vec({0., 1., 0.}),
		tl::make_vec({0., 0., 1.}),
		tl::make_vec({1., 1., 0.}),
		tl::make_vec({0., 1., 1.}),
		tl::make_vec({1., 0., 1.}),
	};

	// to transform into program-specific coordinate systems
	const t_mat matGlobal = tl::make_mat(
	{	{-1., 0., 0., 0.},
		{ 0., 0., 1., 0.},
		{ 0., 1., 0., 0.},
		{ 0., 0., 0., 1.}	});


	std::string strSg;

	std::cout << "Enter spacegroup: ";
	std::getline(std::cin, strSg);
	clipper::Spgr_descr dsc(strSg);
	const int iSGNum = dsc.spacegroup_number();
	if(iSGNum <= 0)
	{
		std::cerr << "Error: Unknown spacegroup." << std::endl;
		return;
	}
	std::cout << "Spacegroup number: " << iSGNum << std::endl;



	int iAtom=0;
	std::vector<t_vec> vecAtoms;
	while(1)
	{
		std::cout << "Enter atom position " << (++iAtom) << ": ";
		std::string strAtom;
		std::getline(std::cin, strAtom);
		tl::trim(strAtom);
		if(strAtom == "")
			break;

		t_vec vec(4);
		std::istringstream istrAtom(strAtom);
		istrAtom >> vec[0] >> vec[1] >> vec[2];
		vec[3] = 1.;
		vecAtoms.push_back(vec);
	}



	clipper::Spacegroup sg(dsc);
	std::vector<t_mat> vecTrafos;
	get_symtrafos(sg, vecTrafos);



	tl::X3d x3d;

	std::cout << std::endl;
	for(int iAtom=0; iAtom<vecAtoms.size(); ++iAtom)
	{
		const t_vec& vecAtom = vecAtoms[iAtom];
		std::vector<t_vec> vecPos = tl::generate_atoms<t_mat, t_vec, std::vector>(vecTrafos, vecAtom);

		std::cout << "Atom " << (iAtom+1) << ":\n";
		for(const t_vec& vec : vecPos)
		{
			std::cout << "\t" << vec << "\n";

			// map back to 1st Brillouin zone
			t_vec vecCoord = vec;
			for(int iComp=0; iComp<vecCoord.size(); ++iComp)
			{
				while(vecCoord[iComp] > 0.5)
					vecCoord[iComp] -= 1;
				while(vecCoord[iComp] < -0.5)
					vecCoord[iComp] += 1.;
			}

			tl::X3dTrafo *pTrafo = new tl::X3dTrafo();
			pTrafo->SetTrans(matGlobal * vecCoord);
			tl::X3dSphere *pSphere = new tl::X3dSphere(0.1);
			pSphere->SetColor(vecColors[iAtom % vecColors.size()]);
			pTrafo->AddChild(pSphere);

			x3d.GetScene().AddChild(pTrafo);
		}
		std::cout << std::endl;
	}

	if(x3d.Save("out.x3d"))
		std::cout << "Wrote out.x3d." << std::endl;
}


int main()
{
	gen_atoms();
	return 0;
}
