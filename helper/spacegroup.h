/*
 * Determine allowed reflexes from space group
 * @author Georg Brandl (author of original Python version "spacegroups.py")
 * @author Tobias Weber (C++ port, i.e. author of this version)
 * @date 19-mar-2014
 *
 * Georg's original Python version:
 * http://forge.frm2.tum.de/cgit/cgit.cgi/frm2/nicos/nicos-core.git/tree/lib/nicos/devices/tas/spacegroups.py
 */

#ifndef __SPACEGROUP_H__
#define __SPACEGROUP_H__

#include <string>
#include <vector>
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

class SpaceGroup
{
	protected:
		std::string m_strName;
		std::vector<unsigned int> m_vecCond;	// conditions in the order as given in Refls

	public:
		SpaceGroup(const std::vector<unsigned int>& vecCond)
					: m_vecCond(vecCond)
		{}
		SpaceGroup(const std::string& strName, const std::vector<unsigned int>& vecCond)
					: m_strName(strName), m_vecCond(vecCond)
		{}
		virtual ~SpaceGroup()
		{}

		bool HasReflection(int h, int k, int l) const;

		void SetName(const std::string& str) { m_strName = str; }
		const std::string& GetName() const { return m_strName; }
};


typedef std::map<std::string, SpaceGroup> t_mapSpaceGroups;

extern void init_space_groups();
extern const t_mapSpaceGroups* get_space_groups();

#endif