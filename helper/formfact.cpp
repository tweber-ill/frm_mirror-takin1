/*
 * Form factor and scattering length tables
 * @author Tobias Weber
 * @date nov-2015
 * @license GPLv2
 */

#include "formfact.h"
#include "tlibs/math/math.h"
#include "tlibs/helper/log.h"
#include "tlibs/file/xml.h"
#include "tlibs/string/string.h"


// =============================================================================

void FormfactList::Init()
{
	tl::Xml xml;
	if(!xml.Load("res/ffacts.xml"))
		return;

	unsigned int iNumDat = xml.Query<unsigned int>("ffacts/num_atoms", 0);

	for(unsigned int iSf=0; iSf<iNumDat; ++iSf)
	{
		Formfact<double> ffact;
		std::string strAtom = "ffacts/atom_" + tl::var_to_str(iSf);

		ffact.strAtom = xml.QueryString((strAtom + "/name").c_str(), "");
		tl::get_tokens<double, std::string, std::vector<double>>
			(xml.QueryString((strAtom + "/a").c_str(), ""), " \t", ffact.a);
		tl::get_tokens<double, std::string, std::vector<double>>
			(xml.QueryString((strAtom + "/b").c_str(), ""), " \t", ffact.b);
		ffact.c = xml.Query<double>((strAtom + "/c").c_str(), 0.);

		s_vecFormfact.push_back(std::move(ffact));
	}
}

std::vector<Formfact<double>> FormfactList::s_vecFormfact;

// -----------------------------------------------------------------------------


FormfactList::FormfactList()
{
	if(!s_vecFormfact.size())
		Init();
}

FormfactList::~FormfactList()
{}


// =============================================================================


void ScatlenList::Init()
{
	tl::Xml xml;
	if(!xml.Load("res/scatlens.xml"))
		return;

	const unsigned int iNumDat = xml.Query<unsigned int>("scatlens/num_atoms", 0);

	for(unsigned int iSl=0; iSl<iNumDat; ++iSl)
	{
		ScatlenList::elem_type slen;
		std::string strAtom = "scatlens/atom_" + tl::var_to_str(iSl);

		slen.strAtom = xml.QueryString((strAtom + "/name").c_str(), "");
		slen.coh = xml.Query<ScatlenList::value_type>((strAtom + "/coh").c_str(), 0.);
		slen.incoh = xml.Query<ScatlenList::value_type>((strAtom + "/incoh").c_str(), 0.);

		if(std::isdigit(slen.strAtom[0]))
			s_vecIsotopes.push_back(std::move(slen));
		else
			s_vecElems.push_back(std::move(slen));
	}
}

std::vector<ScatlenList::elem_type> ScatlenList::s_vecElems;
std::vector<ScatlenList::elem_type> ScatlenList::s_vecIsotopes;

// -----------------------------------------------------------------------------


ScatlenList::ScatlenList()
{
	if(!s_vecElems.size())
		Init();
}

ScatlenList::~ScatlenList()
{}

const ScatlenList::elem_type* ScatlenList::Find(const std::string& strElem) const
{
	typedef typename decltype(s_vecElems)::const_iterator t_iter;

	// elements
	t_iter iter = std::find_if(s_vecElems.begin(), s_vecElems.end(),
		[&strElem](const elem_type& elem)->bool
		{
			//std::cout << elem.GetAtomName() << std::endl;
			return elem.GetAtomName() == strElem;
		});

	// isotopes
	if(iter == s_vecElems.end())
	{
	}

	if(iter == s_vecElems.end())
		return nullptr;

	return &*iter;
}


// =============================================================================

/*
// Test
// gcc -o form helper/formfact.cpp -lstdc++ -std=c++11 -I. tlibs/file/xml.cpp
int main()
{
	ScatlenList lst;
	std::cout << lst.GetNumElems() << std::endl;
	std::cout << lst.GetNumIsotopes() << std::endl;

	const ScatlenList::elem_type& elem = lst.GetElem(0);
	const ScatlenList::elem_type& isot = lst.GetIsotope(3);

	std::cout << elem.GetAtomName() << std::endl;
	std::cout << elem.GetCoherent() << std::endl;
	std::cout << elem.GetIncoherent() << std::endl;

	std::cout << isot.GetAtomName() << std::endl;
	std::cout << isot.GetCoherent() << std::endl;
	std::cout << isot.GetIncoherent() << std::endl;

	return 0;
}
*/
