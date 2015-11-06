/*
 * Wrapper for clipper's form factor tables
 * @author Tobias Weber
 * @date nov-2015
 * @license GPLv2
 */

#include "formfact.h"
#include "tlibs/math/math.h"
#include "tlibs/helper/log.h"
#include "tlibs/file/xml.h"
#include "tlibs/string/string.h"


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
