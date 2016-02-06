/*
 * Loads tabulated spacegroups
 * @author Tobias Weber
 * @date feb-2016
 * @license GPLv2
 */

#include "spacegroup.h"
#include "globals.h"
#include <sstream>


static t_mapSpaceGroups g_mapSpaceGroups;
static t_vecSpaceGroups g_vecSpaceGroups;

bool init_space_groups()
{
	using t_mat = typename SpaceGroup::t_mat;
	using t_vec = typename SpaceGroup::t_vec;

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
		vecTrafos.reserve(iNumTrafos);
		for(unsigned int iTrafo=0; iTrafo<iNumTrafos; ++iTrafo)
		{
			std::ostringstream ostrTrafo;
			ostrTrafo << strGroup << "/trafo_" << iTrafo;
			std::string strTrafo = ostrTrafo.str();

			std::istringstream istrMat(tl::trimmed(xml.Query<std::string>(strTrafo.c_str())));
			t_mat mat;
			istrMat >> mat;

			vecTrafos.push_back(mat);
		}

		SpaceGroup sg;
		sg.SetNr(iSgNr);
		sg.SetName(strName);
		sg.SetLaueGroup(strLaue);
		sg.SetTrafos(vecTrafos);

		t_val pairSg(sg.GetName(), std::move(sg));
		const SpaceGroup *ptheSg = &g_mapSpaceGroups.insert(std::move(pairSg)).first->second;
		g_vecSpaceGroups.push_back(ptheSg);
	}

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
