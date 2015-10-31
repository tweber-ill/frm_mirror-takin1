/*
 * Wrapper for clipper spacegroups (alternate spacegroup implementation)
 * @author Tobias Weber
 * @date oct-2015
 * @license GPLv2
 */

#ifndef __TAKIN_SGCLP_H__
#define __TAKIN_SGCLP_H__

#include <string>
#include <map>
#include <clipper/clipper.h>
#include "crystalsys.h"

#ifndef USE_CLP
	#error USE_CLP is not defined!
#endif


class SpaceGroup
{
	protected:
		clipper::Spacegroup *m_psg = nullptr;
		std::string m_strName;
		std::string m_strLaue;
		CrystalSystem m_crystalsys;

	public:
		SpaceGroup(unsigned int iNum);
		SpaceGroup(const SpaceGroup& sg);
		SpaceGroup(SpaceGroup&& sg);
		SpaceGroup() = delete;
		~SpaceGroup();

		bool HasReflection(int h, int k, int l) const;

		void SetName(const std::string& str) { m_strName = str; }
		const std::string& GetName() const { return m_strName; }

		const std::string& GetLaueGroup() const { return m_strLaue; }

		CrystalSystem GetCrystalSystem() const { return m_crystalsys; }
		const char* GetCrystalSystemName() const { return get_crystal_system_name(m_crystalsys); }
};


typedef std::map<std::string, SpaceGroup> t_mapSpaceGroups;
extern const t_mapSpaceGroups* get_space_groups();


extern std::string get_stdstring(const clipper::String& str);
extern void convert_hm_symbol(std::string& strHM);

#endif
