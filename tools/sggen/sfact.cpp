// gcc -o sfact sfact.cpp -std=c++11 -lstdc++ -lm -I../.. -I/usr/include/QtGui/ ../../helper/spacegroup_clp.cpp ../../helper/crystalsys.cpp ../../helper/globals.cpp ../../helper/formfact.cpp ../../tlibs/file/xml.cpp ../../tlibs/helper/log.cpp -DUSE_CLP -DNO_QT -lclipper-core -lboost_system -lboost_filesystem

/**
 * generates structure factors
 * @author tweber
 * @date nov-2015
 * @license GPLv2
 */

#include <clipper/clipper.h>
#include <vector>
#include <sstream>
#include "tlibs/math/atoms.h"
#include "tlibs/string/string.h"
#include "helper/spacegroup_clp.h"
#include "helper/formfact.h"


typedef tl::ublas::vector<double> t_vec;
typedef tl::ublas::matrix<double> t_mat;

void gen_atoms_sfact()
{
	ScatlenList lst;
	FormfactList lstff;


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



	std::cout << std::endl;
	int iAtom=0;
	std::vector<std::string> vecElems;
	std::vector<t_vec> vecAtoms;
	while(1)
	{
		std::cout << "Enter element " << (++iAtom) << " name: ";
		std::string strElem;
		std::getline(std::cin, strElem);
		tl::trim(strElem);
		if(strElem == "")
			break;

		std::cout << "Enter atom position " << (iAtom) << ": ";
		std::string strAtom;
		std::getline(std::cin, strAtom);
		tl::trim(strAtom);
		if(strAtom == "")
			break;


		vecElems.push_back(strElem);

		t_vec vec(4);
		std::istringstream istrAtom(strAtom);
		istrAtom >> vec[0] >> vec[1] >> vec[2];
		vec[3] = 1.;
		vecAtoms.push_back(vec);
	}



	clipper::Spacegroup sg(dsc);
	std::vector<t_mat> vecTrafos;
	get_symtrafos(sg, vecTrafos);

	std::vector<t_vec> vecAllAtoms;
	std::vector<std::complex<double>> vecScatlens;
	std::vector<double> vecFormfacts;

	for(int iAtom=0; iAtom<int(vecAtoms.size()); ++iAtom)
	{
		const t_vec& vecAtom = vecAtoms[iAtom];
		std::vector<t_vec> vecPos = tl::generate_atoms<t_mat, t_vec, std::vector>(vecTrafos, vecAtom);

		const ScatlenList::elem_type* pElem = lst.Find(vecElems[iAtom]);
		const FormfactList::elem_type* pElemff = lstff.Find(vecElems[iAtom]);

		if(pElem == nullptr)
		{
			std::cerr << "Error: cannot get scattering length for "
				<< vecElems[iAtom] << "." << std::endl;
			return;
		}
		if(pElemff == nullptr)
		{
			std::cerr << "Error: cannot get form factor for "
				<< vecElems[iAtom] << "." << std::endl;
			return;
		}
		std::complex<double> b = pElem->GetCoherent() / 10.;
		double dG = 0.;		// TODO
		double dFF = pElemff->GetFormfact(dG);

		for(t_vec vecAtom : vecPos)
		{
			vecAtom.resize(3,1);
			vecAllAtoms.push_back(vecAtom);
			vecScatlens.push_back(b);
			vecFormfacts.push_back(dFF);
		}
	}


	while(1)
	{
		std::cout << std::endl;

		double h=0., k=0., l=0.;
		std::cout << "Enter hkl: ";
		std::cin >> h >> k >> l;

		std::complex<double> F = tl::structfact<double, std::complex<double>, ublas::vector<double>, std::vector>
			(vecAllAtoms, tl::make_vec({h,k,l})*2.*M_PI, vecScatlens);
		std::complex<double> Fx = tl::structfact<double, double, ublas::vector<double>, std::vector>
			(vecAllAtoms, tl::make_vec({h,k,l})*2.*M_PI, vecFormfacts);

		std::cout << "Neutron structure factor: " << std::endl;
		double dFsq = (std::conj(F)*F).real();
		std::cout << "F = " << F << std::endl;
		std::cout << "|F| = " << std::sqrt(dFsq) << std::endl;
		std::cout << "|F|^2 = " << dFsq << std::endl;
		std::cout << std::endl;
		std::cout << "X-ray structure factor: " << std::endl;
		double dFxsq = (std::conj(Fx)*Fx).real();
		std::cout << "Fx = " << Fx << std::endl;
		std::cout << "|Fx| = " << std::sqrt(dFxsq) << std::endl;
		std::cout << "|Fx|^2 = " << dFxsq << std::endl;
	}
}


int main()
{
	try
	{
		gen_atoms_sfact();
	}
	catch(const clipper::Message_fatal& err)
	{
		std::cerr << "Error in spacegroup." << std::endl;
	}
	return 0;
}
