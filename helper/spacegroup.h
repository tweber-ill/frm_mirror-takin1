/*
 * Loads tabulated spacegroups
 * @author Tobias Weber
 * @date feb-2016
 * @license GPLv2
 */

#ifndef __SG_TAB_H__
#define __SG_TAB_H__

#include "sghelper.h"

#include <map>
#include "crystalsys.h"
#include "sghelper.h"
#include "tlibs/string/string.h"
#include "tlibs/log/log.h"
#include "tlibs/file/prop.h"

namespace ublas = boost::numeric::ublas;


class SpaceGroup
{
public:
	using t_real = double;
	using t_mat = ublas::matrix<t_real>;
	using t_vec = ublas::vector<t_real>;

protected:
	unsigned int m_iNr = 0;
	std::string m_strName;
	std::string m_strLaue, m_strPoint;
	CrystalSystem m_crystalsys;
	std::string m_strCrystalSysName;
	std::vector<t_mat> m_vecTrafos;

public:
	SpaceGroup() = default;
	~SpaceGroup() = default;
	SpaceGroup(const SpaceGroup& sg)
		: m_iNr(sg.m_iNr), m_strName(sg.m_strName), m_strLaue(sg.m_strLaue),
		m_strPoint(sg.m_strPoint), m_crystalsys(sg.m_crystalsys),
		m_strCrystalSysName(sg.m_strCrystalSysName), m_vecTrafos(sg.m_vecTrafos)
	{}
	SpaceGroup(SpaceGroup&& sg)
		: m_iNr(sg.m_iNr), m_strName(std::move(sg.m_strName)), m_strLaue(std::move(sg.m_strLaue)),
		m_strPoint(std::move(sg.m_strPoint)), m_crystalsys(std::move(sg.m_crystalsys)),
		m_strCrystalSysName(std::move(sg.m_strCrystalSysName)), m_vecTrafos(std::move(sg.m_vecTrafos))
	{}

	bool HasReflection(int h, int k, int l) const
	{
		return is_reflection_allowed(h,k,l, m_vecTrafos);
	}

	void SetNr(unsigned int iNr) { m_iNr = iNr; }
	unsigned int GetNr() const { return m_iNr; }

	void SetName(const std::string& str)
	{
		m_strName = str;
		m_strPoint = get_pointgroup(m_strName);
	}
	const std::string& GetName() const { return m_strName; }

	void SetCrystalSystem(const CrystalSystem& crys)
	{
		m_crystalsys = crys;
		m_strCrystalSysName = get_crystal_system_name(m_crystalsys);
	}
	CrystalSystem GetCrystalSystem() const { return m_crystalsys; }
	const std::string& GetCrystalSystemName() const { return m_strCrystalSysName; }

	void SetLaueGroup(const std::string& str)
	{
		m_strLaue = str;
		SetCrystalSystem(get_crystal_system_from_laue_group(m_strLaue.c_str()));
	}
	const std::string& GetLaueGroup() const { return m_strLaue; }
	const std::string& GetPointGroup() const { return m_strPoint; }

	void SetTrafos(std::vector<t_mat>&& vecTrafos) { m_vecTrafos = std::move(vecTrafos); }
	void SetTrafos(const std::vector<t_mat>& vecTrafos) { m_vecTrafos = vecTrafos; }
	const std::vector<t_mat>& GetTrafos() const { return m_vecTrafos; }
};


typedef std::vector<const SpaceGroup*> t_vecSpaceGroups;
typedef std::map<std::string, SpaceGroup> t_mapSpaceGroups;
extern const t_mapSpaceGroups* get_space_groups();
extern const t_vecSpaceGroups* get_space_groups_vec();
extern bool init_space_groups();


#endif
