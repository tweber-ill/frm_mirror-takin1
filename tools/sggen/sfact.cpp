// gcc -o sfact sfact.cpp -std=c++11 -lstdc++ -lm -I../.. -I/usr/include/QtGui/ ../../helper/spacegroup_clp.cpp ../../helper/crystalsys.cpp ../../helper/globals.cpp ../../helper/formfact.cpp ../../tlibs/log/log.cpp -DNO_QT -lclipper-core -lboost_system -lboost_filesystem

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
#include "tlibs/math/lattice.h"
#include "tlibs/math/neutrons.hpp"
#include "tlibs/math/linalg_ops.h"
#include "tlibs/string/string.h"
#include "libs/spacegroups/spacegroup_clp.h"
#include "libs/formfactors/formfact.h"


typedef tl::ublas::vector<double> t_vec;
typedef tl::ublas::matrix<double> t_mat;

void gen_atoms_sfact()
{
	ScatlenList lst;
	FormfactList lstff;


	double a,b,c, alpha,beta,gamma;
	std::cout << "Enter unit cell lattice constants: ";
	std::cin >> a >> b >> c;
	std::cout << "Enter unit cell angles: ";
	std::cin >> alpha >> beta >> gamma;

	alpha = alpha/180.*M_PI;
	beta = beta/180.*M_PI;
	gamma = gamma/180.*M_PI;

	const tl::Lattice<double> lattice(a,b,c, alpha,beta,gamma);
	const double dVol = lattice.GetVol();
	const t_mat matA = lattice.GetMetric();
	const t_mat matB = lattice.GetRecip().GetMetric();
	std::cout << "A = " << matA << std::endl;
	std::cout << "B = " << matB << std::endl;



	std::cout << std::endl;
	std::string strSg;
	std::cout << "Enter spacegroup: ";
	std::cin.ignore();
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
		std::cout << "Enter element " << (++iAtom) << " name (or <Enter> to finish): ";
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
	std::cout << vecTrafos.size() << " symmetry operations in spacegroup." << std::endl;

	std::vector<unsigned int> vecNumAtoms;
	std::vector<t_vec> vecAllAtoms;
	std::vector<std::complex<double>> vecScatlens;
	std::vector<double> vecFormfacts;
	std::vector<int> vecAtomIndices;

	double dSigAbs = 0.;

	for(int iAtom=0; iAtom<int(vecAtoms.size()); ++iAtom)
	{
		const t_vec& vecAtom = vecAtoms[iAtom];
		std::vector<t_vec> vecPos = tl::generate_atoms<t_mat, t_vec, std::vector>(vecTrafos, vecAtom);
		vecNumAtoms.push_back(vecPos.size());
		std::cout << "Generated " << vecPos.size() << " " << vecElems[iAtom] << " atoms." << std::endl;
		for(const t_vec& vec : vecPos)
			std::cout << vec << std::endl;

		const ScatlenList::elem_type* pElem = lst.Find(vecElems[iAtom]);

		if(pElem == nullptr)
		{
			std::cerr << "Error: cannot get scattering length for "
				<< vecElems[iAtom] << "." << std::endl;
			return;
		}

		std::complex<double> b = pElem->GetCoherent() /*/ 10.*/;

		dSigAbs += tl::macro_xsect(pElem->GetXSecCoherent().real()*tl::barns, 
			vecNumAtoms[iAtom],
			dVol*tl::angstrom*tl::angstrom*tl::angstrom) * tl::cm;
		//dSigAbs += pElem->GetXSecCoherent().real()*1e-24 * vecNumAtoms[iAtom] / (dVol*1e-24);

		for(t_vec vecThisAtom : vecPos)
		{
			vecThisAtom.resize(3,1);
			vecAllAtoms.push_back(matA * vecThisAtom);
			vecScatlens.push_back(b);
			vecAtomIndices.push_back(iAtom);
		}
	}

	const double dLam0 = 1.8;	// thermal
	const double dLam = 4.5;
	std::cout << "\nMacroscopic absorption cross-section for lambda = 4.5 A: "
		<< dSigAbs*dLam/dLam0 << " / cm." << std::endl;

	//for(const t_vec& vecAt : vecAllAtoms) std::cout << vecAt << std::endl;
	//for(const std::complex<double>& cb : vecScatlens) std::cout << cb << std::endl;


	while(1)
	{
		std::cout << std::endl;

		double h=0., k=0., l=0.;
		std::cout << "Enter hkl: ";
		std::cin >> h >> k >> l;

		t_vec vecG = matB * tl::make_vec({h,k,l});
		double dG = ublas::norm_2(vecG);
		std::cout << "G = " << dG << " / A" << std::endl;


		vecFormfacts.clear();
		for(unsigned int iAtom=0; iAtom<vecAllAtoms.size(); ++iAtom)
		{
			//const t_vec& vecAtom = vecAllAtoms[iAtom];
			const FormfactList::elem_type* pElemff = lstff.Find(vecElems[vecAtomIndices[iAtom]]);

			if(pElemff == nullptr)
			{
				std::cerr << "Error: cannot get form factor for "
					<< vecElems[iAtom] << "." << std::endl;
				return;
			}

			double dFF = pElemff->GetFormfact(dG);
			vecFormfacts.push_back(dFF);
		}


		std::complex<double> F = tl::structfact<double, std::complex<double>, ublas::vector<double>, std::vector>
			(vecAllAtoms, vecG, vecScatlens);
		std::complex<double> Fx = tl::structfact<double, double, ublas::vector<double>, std::vector>
			(vecAllAtoms, vecG, vecFormfacts);


		std::cout << std::endl;
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
