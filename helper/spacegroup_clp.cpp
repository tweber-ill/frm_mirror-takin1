/*
 * Wrapper for clipper spacegroups (alternate spacegroup implementation)
 * @author Tobias Weber
 * @date oct-2015
 * @license GPLv2
 */

#include "spacegroup_clp.h"
#include "tlibs/string/string.h"
#include <sstream>
#include <ctype.h>

std::string get_stdstring(const clipper::String& str)
{
	std::ostringstream ostr;
	ostr << str;
	return ostr.str();
}

// convert e.g.: "P 21 3"  ->  "P2_13"
void convert_hm_symbol(std::string& strHM)
{
	std::vector<std::string> vecSyms;
	tl::get_tokens<std::string, std::string, decltype(vecSyms)>(strHM, " ", vecSyms);

	for(std::string& str : vecSyms)
	{
		bool bLastWasDigit = 0;
		for(std::size_t iC = 0; iC<str.length(); ++iC)
		{
			std::string::value_type c = str[iC];

			bool bCurIsDigit = std::isdigit(c);

			if(bCurIsDigit && bLastWasDigit)
			{
				str.insert(iC, "_");
				bLastWasDigit = 0;
			}
			else
			{
				bLastWasDigit = bCurIsDigit;
			}
		}
	}

	strHM = "";
	for(const std::string& str : vecSyms)
		strHM += str /*+ " "*/;
}


// -----------------------------------------------------------------------------


SpaceGroup::SpaceGroup(unsigned int iNum)
	: m_psg(new clipper::Spacegroup(clipper::Spgr_descr(iNum)))
{
	m_strName = get_stdstring(m_psg->symbol_hm());
	convert_hm_symbol(m_strName);

	m_strLaue = get_stdstring(m_psg->symbol_laue());
	m_crystalsys = get_crystal_system_from_laue_group(m_strLaue.c_str());
}

SpaceGroup::~SpaceGroup()
{
	if(m_psg) { delete m_psg; m_psg = nullptr; }
}

SpaceGroup::SpaceGroup(const SpaceGroup& sg)
	: m_psg(new clipper::Spacegroup(sg.m_psg->descr())),
	m_strName(sg.GetName()), m_strLaue(sg.GetLaueGroup()),
	m_crystalsys(sg.GetCrystalSystem())
{}

SpaceGroup::SpaceGroup(SpaceGroup&& sg)
{
	m_psg = sg.m_psg;
	sg.m_psg = nullptr;

	m_strName = std::move(sg.m_strName);
	m_strLaue = std::move(sg.m_strLaue);
	m_crystalsys = std::move(sg.m_crystalsys);
}

bool SpaceGroup::HasReflection(int h, int k, int l) const
{
	if(!m_psg) return false;

	clipper::HKL_class hkl = m_psg->hkl_class(clipper::HKL(h,k,l));
	return !hkl.sys_abs();
}


// -----------------------------------------------------------------------------


static t_mapSpaceGroups g_mapSpaceGroups;

static void init_space_groups()
{
	typedef t_mapSpaceGroups::value_type t_val;

	for(int iSg=1; iSg<=230; ++iSg)
	{
		SpaceGroup sg(iSg);
		g_mapSpaceGroups.insert(t_val(sg.GetName(), std::move(sg)));
	}
}

const t_mapSpaceGroups* get_space_groups()
{
	if(g_mapSpaceGroups.empty())
		init_space_groups();

	return &g_mapSpaceGroups;
}
