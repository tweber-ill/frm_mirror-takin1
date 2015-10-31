/*
 * Determine allowed reflexes from space group
 * @author of original Python version "spacegroups.py": Georg Brandl
 * @author of this C++ port: Tobias Weber
 * @date 19-mar-2014
 * @license GPLv2
 *
 * Georg's original Python version:
 * http://forge.frm2.tum.de/cgit/cgit.cgi/frm2/nicos/nicos-core.git/tree/nicos/devices/tas/spacegroups.py
 */

#ifndef __SPACEGROUP_H__
#define __SPACEGROUP_H__

#include <string>
#include <array>
#include <map>
#include "crystalsys.h"

enum Refls
{
	REFL_00L = 0,
	REFL_0K0 = 1,
	REFL_0KL = 2,
	REFL_H00 = 3,
	REFL_H0L = 4,
	REFL_HK0 = 5,
	REFL_HKL = 6,
	REFL_0KK = 7,
	REFL_HH0 = 8,
	REFL_HHL = 9,
	REFL_H0H = 10,
	REFL_HKK = 11,
	REFL_HKH = 12,
	REFL_HHH = 13
};

class SpaceGroup
{
	protected:
		std::string m_strName;
		CrystalSystem m_crystalsys;

		// general conditions in the order as given in Refls
		std::array<unsigned char, 14> m_vecCond = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

		// TODO: special conditions (not relevant for BZ calculation) in the order as given in Refls
		//std::array<unsigned char, 14> m_vecCondSpec = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

	public:
		SpaceGroup(const std::array<unsigned char, 14>& vecCond, CrystalSystem ty=CRYS_NOT_SET)
					: m_crystalsys(ty), m_vecCond(vecCond)
		{}
		SpaceGroup(const std::string& strName,
					const std::array<unsigned char, 14>& vecCond,
					const std::array<unsigned char, 14>& vecCondSpec,
					CrystalSystem ty=CRYS_NOT_SET)
					: m_strName(strName), m_crystalsys(ty),
						m_vecCond(vecCond)/*, m_vecCondSpec(vecCondSpec)*/
		{}
		~SpaceGroup()
		{}

		bool HasReflection(int h, int k, int l, bool bGeneral=true) const;

		void SetName(const std::string& str) { m_strName = str; }
		const std::string& GetName() const { return m_strName; }

		CrystalSystem GetCrystalSystem() const { return m_crystalsys; }
		const char* GetCrystalSystemName() const { return get_crystal_system_name(m_crystalsys); }
};


typedef std::map<std::string, SpaceGroup> t_mapSpaceGroups;
extern const t_mapSpaceGroups* get_space_groups();

#endif
