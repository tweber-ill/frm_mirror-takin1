/*
 * Loads tabulated spacegroups
 * @author Tobias Weber
 * @date feb-2016
 * @license GPLv2
 */

#include "spacegroup.h"
#include "libs/globals.h"
#include <sstream>


static t_mapSpaceGroups g_mapSpaceGroups;
static t_vecSpaceGroups g_vecSpaceGroups;
static std::string s_strSrc, s_strUrl;

bool init_space_groups()
{
	using t_mat = typename SpaceGroup::t_mat;
	//using t_vec = typename SpaceGroup::t_vec;

	if(!g_mapSpaceGroups.empty())
	{
		tl::log_warn("Space Groups have already been initialised.");
		return false;
	}

	tl::Prop<std::string> xml;
	//if(!xml.Load("/home/tw/Projects/tastools/res/sgroups.xml", tl::PropType::XML))
	if(!xml.Load(find_resource("res/sgroups.xml").c_str(), tl::PropType::XML))
		return false;

	unsigned int iNumSGs = xml.Query<unsigned int>("sgroups/num_groups", 0);
	if(iNumSGs < 230)
		tl::log_warn("Less than 230 space groups are defined!");


	g_vecSpaceGroups.reserve(iNumSGs);
	typedef t_mapSpaceGroups::value_type t_val;

	for(unsigned int iSg=0; iSg<iNumSGs; ++iSg)
	{
		std::ostringstream ostrGroup;
		ostrGroup << "sgroups/group_" << iSg;
		std::string strGroup = ostrGroup.str();

		unsigned int iSgNr = xml.Query<unsigned int>((strGroup+"/number").c_str());
		std::string strName = tl::trimmed(xml.Query<std::string>((strGroup+"/name").c_str()));
		std::string strLaue = tl::trimmed(xml.Query<std::string>((strGroup+"/lauegroup").c_str()));
		unsigned int iNumTrafos = xml.Query<unsigned int>((strGroup+"/num_trafos").c_str());

		std::vector<t_mat> vecTrafos;
		std::vector<unsigned int> vecInvTrafos, vecPrimTrafos, vecCenterTrafos;
		vecTrafos.reserve(iNumTrafos);

		for(unsigned int iTrafo=0; iTrafo<iNumTrafos; ++iTrafo)
		{
			std::ostringstream ostrTrafo;
			ostrTrafo << strGroup << "/trafo_" << iTrafo;
			std::string strTrafo = ostrTrafo.str();

			std::string strTrafoVal = xml.Query<std::string>(strTrafo.c_str());
			std::pair<std::string, std::string> pairSg = tl::split_first(strTrafoVal, std::string(";"), 1);

			std::istringstream istrMat(pairSg.first);
			t_mat mat;
			istrMat >> mat;

			for(typename std::string::value_type c : pairSg.second)
			{
				if(std::tolower(c)=='p') vecPrimTrafos.push_back(iTrafo);
				if(std::tolower(c)=='i') vecInvTrafos.push_back(iTrafo);
				if(std::tolower(c)=='c') vecCenterTrafos.push_back(iTrafo);
			}

			vecTrafos.push_back(std::move(mat));
		}

		SpaceGroup sg;
		sg.SetNr(iSgNr);
		sg.SetName(strName);
		sg.SetLaueGroup(strLaue);
		sg.SetTrafos(std::move(vecTrafos));
		sg.SetInvTrafos(std::move(vecInvTrafos));
		sg.SetPrimTrafos(std::move(vecPrimTrafos));
		sg.SetCenterTrafos(std::move(vecCenterTrafos));

		g_mapSpaceGroups.insert(t_val(sg.GetName(), std::move(sg)));
	}

	g_vecSpaceGroups.reserve(g_mapSpaceGroups.size());
	for(const t_val& pairSg : g_mapSpaceGroups)
		g_vecSpaceGroups.push_back(&pairSg.second);

	std::sort(g_vecSpaceGroups.begin(), g_vecSpaceGroups.end(),
		[](const SpaceGroup* sg1, const SpaceGroup* sg2) -> bool
		{ return sg1->GetNr() <= sg2->GetNr(); });


	s_strSrc = xml.Query<std::string>("sgroups/source", "");
	s_strUrl = xml.Query<std::string>("sgroups/source_url", "");

	return true;
}

const t_mapSpaceGroups* get_space_groups()
{
	if(g_mapSpaceGroups.empty())
	{
		tl::log_warn("Space Groups had not been initialised properly.");
		init_space_groups();
	}

	return &g_mapSpaceGroups;
}

const t_vecSpaceGroups* get_space_groups_vec()
{
	if(g_vecSpaceGroups.empty())
	{
		tl::log_warn("Space Groups had not been initialised properly.");
		init_space_groups();
	}

	return &g_vecSpaceGroups;
}

extern const std::string& get_sgsource(bool bUrl)
{
	return bUrl ? s_strUrl : s_strSrc ;
}
