/*
 * Determine allowed reflexes from space group
 * @author Georg Brandl (author of original Python version "spacegroups.py")
 * @author Tobias Weber (C++ port, i.e. author of this version)
 * @date 19-mar-2014
 * @copyright GPLv2
 *
 * Georg's original Python version:
 * http://forge.frm2.tum.de/cgit/cgit.cgi/frm2/nicos/nicos-core.git/tree/lib/nicos/devices/tas/spacegroups.py
 */

#ifndef __SPACEGROUP_H__
#define __SPACEGROUP_H__

#include <string>
#include <array>
#include <map>

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

enum CrystalSystem
{
	CRYS_NOT_SET,

	CRYS_TRICLINIC,		// all params free
	CRYS_MONOCLINIC,	// beta=gamma=90
	CRYS_ORTHORHOMBIC,	// alpha=beta=gamma=90
	CRYS_TETRAGONAL,	// a=b, alpha=beta=gamma=90
	CRYS_TRIGONAL,		// a=b=c, alpha=beta=gamma
	CRYS_HEXAGONAL,		// a=b, gamma=120, alpha=beta=90
	CRYS_CUBIC			// a=b=c, alpha=beta=gamma=90
};

extern const char* get_crystal_system_name(CrystalSystem ty);

class SpaceGroup
{
	protected:
		std::string m_strName;
		std::array<unsigned char, 14> m_vecCond;		// conditions in the order as given in Refls
		CrystalSystem m_crystalsys;

	public:
		SpaceGroup(const std::array<unsigned char, 14>& vecCond, CrystalSystem ty=CRYS_NOT_SET)
					: m_vecCond(vecCond), m_crystalsys(ty)
		{}
		SpaceGroup(const std::string& strName, const std::array<unsigned char, 14>& vecCond, CrystalSystem ty=CRYS_NOT_SET)
					: m_strName(strName), m_vecCond(vecCond), m_crystalsys(ty)
		{}
		virtual ~SpaceGroup()
		{}

		bool HasReflection(int h, int k, int l) const;

		void SetName(const std::string& str) { m_strName = str; }
		const std::string& GetName() const { return m_strName; }

		CrystalSystem GetCrystalSystem() const { return m_crystalsys; }
		const char* GetCrystalSystemName() const { return get_crystal_system_name(m_crystalsys); }
		
		//const std::array<unsigned char, 14>& GetConds() const { return m_vecCond; }
};


typedef std::map<std::string, SpaceGroup> t_mapSpaceGroups;

extern void init_space_groups();
extern const t_mapSpaceGroups* get_space_groups();

#endif
