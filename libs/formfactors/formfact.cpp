/*
 * Form factor and scattering length tables
 * @author Tobias Weber
 * @date nov-2015
 * @license GPLv2
 */

#include "formfact.h"
#include "libs/globals.h"
#include "tlibs/math/math.h"
#include "tlibs/log/log.h"
#include "tlibs/file/prop.h"
#include "tlibs/string/string.h"


// =============================================================================

std::shared_ptr<FormfactList> FormfactList::s_inst = nullptr;
std::mutex FormfactList::s_mutex;


FormfactList::FormfactList()
{
	tl::log_debug("Loading atomic form factors.");

	tl::Prop<std::string> xml;
	if(!xml.Load(find_resource("res/ffacts.xml").c_str(), tl::PropType::XML))
		return;

	unsigned int iNumDat = xml.Query<unsigned int>("ffacts/num_atoms", 0);
	if(!iNumDat)
	{
		tl::log_err("No data in atomic form factor list.");
		return;
	}

	bool bIonStart = 0;
	for(unsigned int iSf=0; iSf<iNumDat; ++iSf)
	{
		elem_type ffact;
		std::string strAtom = "ffacts/atom_" + tl::var_to_str(iSf);

		ffact.strAtom = xml.Query<std::string>((strAtom + "/name").c_str(), "");
		tl::get_tokens<value_type, std::string, std::vector<value_type>>
			(xml.Query<std::string>((strAtom + "/a").c_str(), ""), " \t", ffact.a);
		tl::get_tokens<value_type, std::string, std::vector<value_type>>
			(xml.Query<std::string>((strAtom + "/b").c_str(), ""), " \t", ffact.b);
		ffact.c = xml.Query<value_type>((strAtom + "/c").c_str(), 0.);

		if(!bIonStart && ffact.strAtom.find_first_of("+-") != std::string::npos)
			bIonStart = 1;

		if(!bIonStart)
			s_vecAtoms.push_back(std::move(ffact));
		else
			s_vecIons.push_back(std::move(ffact));
	}

	s_strSrc = xml.Query<std::string>("ffacts/source", "");
	s_strSrcUrl = xml.Query<std::string>("ffacts/source_url", "");
}

FormfactList::~FormfactList()
{}

std::shared_ptr<const FormfactList> FormfactList::GetInstance()
{
	std::lock_guard<std::mutex> _guard(s_mutex);
	
	if(!s_inst)
		s_inst = std::shared_ptr<FormfactList>(new FormfactList());

	return s_inst;
}

const FormfactList::elem_type* FormfactList::Find(const std::string& strElem) const
{
	typedef typename decltype(s_vecAtoms)::const_iterator t_iter;

	// atoms
	t_iter iter = std::find_if(s_vecAtoms.begin(), s_vecAtoms.end(),
		[&strElem](const elem_type& elem)->bool
		{
			//std::cout << elem.GetAtomIdent() << std::endl;
			return elem.GetAtomIdent() == strElem;
		});
	if(iter != s_vecAtoms.end())
		return &*iter;

	// ions
	iter = std::find_if(s_vecIons.begin(), s_vecIons.end(),
		[&strElem](const elem_type& elem)->bool
		{
			//std::cout << elem.GetAtomIdent() << std::endl;
			return elem.GetAtomIdent() == strElem;
		});
	if(iter != s_vecIons.end())
		return &*iter;

	return nullptr;
}




// =============================================================================

std::shared_ptr<MagFormfactList> MagFormfactList::s_inst = nullptr;
std::mutex MagFormfactList::s_mutex;


MagFormfactList::MagFormfactList()
{
	tl::log_debug("Loading magnetic form factors.");

	tl::Prop<std::string, true> xml;
	if(!xml.Load(find_resource("res/magffacts.xml").c_str(), tl::PropType::XML))
		return;

	unsigned int iNumDat = xml.Query<unsigned int>("magffacts/num_atoms", 0);
	if(!iNumDat)
	{
		tl::log_err("No data in magnetic form factor list.");
		return;
	}
	for(unsigned int iSf=0; iSf<iNumDat; ++iSf)
	{
		elem_type ffact;
		std::string strAtom = "magffacts/j0/atom_" + tl::var_to_str(iSf);

		ffact.strAtom = xml.Query<std::string>((strAtom + "/name").c_str(), "");
		for(const std::string& strA : {"/A", "/B", "/C", "/D"})
			ffact.A0.push_back(xml.Query<value_type>((strAtom + strA).c_str(), 0.));
		for(const std::string& stra : {"/a", "/b", "/c"})
			ffact.a0.push_back(xml.Query<value_type>((strAtom + stra).c_str(), 0.));

		s_vecAtoms.push_back(std::move(ffact));
	}

	for(unsigned int iSf=0; iSf<iNumDat; ++iSf)
	{
		std::string strAtom = "magffacts/j2/atom_" + tl::var_to_str(iSf);
		std::string strAtomName = xml.Query<std::string>((strAtom + "/name").c_str(), "");

		MagFormfactList::elem_type* pElem =
			const_cast<MagFormfactList::elem_type*>(Find(strAtomName));
		if(!pElem)
		{
			tl::log_err("Mismatch in j0 and j2 form factor tables.");
			continue;
		}

		for(const std::string& strA : {"/A", "/B", "/C", "/D"})
			pElem->A2.push_back(xml.Query<value_type>((strAtom + strA).c_str(), 0.));
		for(const std::string& stra : {"/a", "/b", "/c"})
			pElem->a2.push_back(xml.Query<value_type>((strAtom + stra).c_str(), 0.));
	}

	s_strSrc = xml.Query<std::string>("magffacts/source", "");
	s_strSrcUrl = xml.Query<std::string>("magffacts/source_url", "");
}

MagFormfactList::~MagFormfactList()
{}

std::shared_ptr<const MagFormfactList> MagFormfactList::GetInstance()
{
	std::lock_guard<std::mutex> _guard(s_mutex);
	
	if(!s_inst)
		s_inst = std::shared_ptr<MagFormfactList>(new MagFormfactList());

	return s_inst;
}

const MagFormfactList::elem_type* MagFormfactList::Find(const std::string& strElem) const
{
	typedef typename decltype(s_vecAtoms)::const_iterator t_iter;

	t_iter iter = std::find_if(s_vecAtoms.begin(), s_vecAtoms.end(),
		[&strElem](const elem_type& elem) -> bool
		{
			return elem.GetAtomIdent() == strElem;
		});
	if(iter != s_vecAtoms.end())
		return &*iter;

	return nullptr;
}



// =============================================================================


std::shared_ptr<ScatlenList> ScatlenList::s_inst = nullptr;
std::mutex ScatlenList::s_mutex;


ScatlenList::ScatlenList()
{
	tl::log_debug("Loading neutron scattering lengths.");

	tl::Prop<std::string> xml;
	if(!xml.Load(find_resource("res/scatlens.xml").c_str(), tl::PropType::XML))
		return;

	const unsigned int iNumDat = xml.Query<unsigned int>("scatlens/num_atoms", 0);
	if(!iNumDat)
	{
		tl::log_err("No data in scattering length list.");
		return;
	}

	for(unsigned int iSl=0; iSl<iNumDat; ++iSl)
	{
		ScatlenList::elem_type slen;
		std::string strAtom = "scatlens/atom_" + tl::var_to_str(iSl);

		slen.strAtom = xml.Query<std::string>((strAtom + "/name").c_str(), "");
		slen.coh = xml.Query<ScatlenList::value_type>((strAtom + "/coh").c_str(), 0.);
		slen.incoh = xml.Query<ScatlenList::value_type>((strAtom + "/incoh").c_str(), 0.);

		slen.xsec_coh = xml.Query<ScatlenList::value_type>((strAtom + "/xsec_coh").c_str(), 0.);
		slen.xsec_incoh = xml.Query<ScatlenList::value_type>((strAtom + "/xsec_incoh").c_str(), 0.);
		slen.xsec_scat = xml.Query<ScatlenList::value_type>((strAtom + "/xsec_scat").c_str(), 0.);
		slen.xsec_abs = xml.Query<ScatlenList::value_type>((strAtom + "/xsec_abs").c_str(), 0.);


		if(std::isdigit(slen.strAtom[0]))
			s_vecIsotopes.push_back(std::move(slen));
		else
			s_vecElems.push_back(std::move(slen));
	}

	s_strSrc = xml.Query<std::string>("scatlens/source", "");
	s_strSrcUrl = xml.Query<std::string>("scatlens/source_url", "");
}

ScatlenList::~ScatlenList()
{}

std::shared_ptr<const ScatlenList> ScatlenList::GetInstance()
{
	std::lock_guard<std::mutex> _guard(s_mutex);
	
	if(!s_inst)
		s_inst = std::shared_ptr<ScatlenList>(new ScatlenList());

	return s_inst;
}

const ScatlenList::elem_type* ScatlenList::Find(const std::string& strElem) const
{
	typedef typename decltype(s_vecElems)::const_iterator t_iter;

	// elements
	t_iter iter = std::find_if(s_vecElems.begin(), s_vecElems.end(),
		[&strElem](const elem_type& elem)->bool
		{
			//std::cout << elem.GetAtomIdent() << std::endl;
			return elem.GetAtomIdent() == strElem;
		});
	if(iter != s_vecElems.end())
		return &*iter;

	// isotopes
	iter = std::find_if(s_vecIsotopes.begin(), s_vecIsotopes.end(),
		[&strElem](const elem_type& elem)->bool
		{
			//std::cout << elem.GetAtomIdent() << std::endl;
			return elem.GetAtomIdent() == strElem;
		});
	if(iter != s_vecIsotopes.end())
		return &*iter;

	return nullptr;
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

	std::cout << elem.GetAtomIdent() << std::endl;
	std::cout << elem.GetCoherent() << std::endl;
	std::cout << elem.GetIncoherent() << std::endl;

	std::cout << isot.GetAtomIdent() << std::endl;
	std::cout << isot.GetCoherent() << std::endl;
	std::cout << isot.GetIncoherent() << std::endl;

	return 0;
}
*/
