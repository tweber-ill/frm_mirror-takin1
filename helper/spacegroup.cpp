/*
 * Determine allowed reflexes from space group
 * @author Georg Brandl (author of original Python version "spacegroups.py"
 * @author Tobias Weber (ported to C++, i.e. this version)
 * @date 19-mar-2014
 *
 * Georg's original Python version:
 * http://forge.frm2.tum.de/cgit/cgit.cgi/frm2/nicos/nicos-core.git/tree/lib/nicos/devices/tas/spacegroups.py
 */

#include "spacegroup.h"
#include <cstdlib>

// -> function "check_refcond" in Georg's Python code
static std::vector<bool (*)(int h, int k, int l)> g_vecConds =
{
	/*00*/ [] (int h, int k, int l) -> bool { return 1; },
	/*01*/ [] (int h, int k, int l) -> bool { return h%2 == 0; },
	/*02*/ [] (int h, int k, int l) -> bool { return k%2 == 0; },
	/*03*/ [] (int h, int k, int l) -> bool { return l%2 == 0; },
	/*04*/ [] (int h, int k, int l) -> bool { return (k+l)%2 == 0; },
	/*05*/ [] (int h, int k, int l) -> bool { return (h+l)%2 == 0; },
	/*06*/ [] (int h, int k, int l) -> bool { return (h+k)%2 == 0; },
	/*07*/ [] (int h, int k, int l) -> bool { return (h%2 == k%2) && (k%2 == l%2); },
	/*08*/ [] (int h, int k, int l) -> bool { return (k+l)%4 == 0; },
	/*09*/ [] (int h, int k, int l) -> bool { return (h+l)%4 == 0; },
	/*10*/ [] (int h, int k, int l) -> bool { return (h+k)%4 == 0; },
	/*11*/ [] (int h, int k, int l) -> bool { return (2*h+l)%2 == 0; },
	/*12*/ [] (int h, int k, int l) -> bool { return (2*h+l)%4 == 0; },
	/*13*/ [] (int h, int k, int l) -> bool { return (h+k+l)%2 == 0; },
	/*14*/ [] (int h, int k, int l) -> bool { return (-h+k+l)%3 == 0; },
	/*15*/ [] (int h, int k, int l) -> bool { return (h-k+l)%3 == 0; },
	/*16*/ [] (int h, int k, int l) -> bool { return h%4 == 0; },
	/*17*/ [] (int h, int k, int l) -> bool { return k%4 == 0; },
	/*18*/ [] (int h, int k, int l) -> bool { return l%3 == 0; },
	/*19*/ [] (int h, int k, int l) -> bool { return l%4 == 0; },
	/*20*/ [] (int h, int k, int l) -> bool { return l%6 == 0; },
	/*21*/ [] (int h, int k, int l) -> bool { return (std::abs(h) >= std::abs(k)) && (std::abs(k) >= std::abs(l)); },
	/*22*/ [] (int h, int k, int l) -> bool { return (2*h+k)%2 == 0; },
	/*23*/ [] (int h, int k, int l) -> bool { return (2*h+k)%4 == 0; },
	/*24*/ [] (int h, int k, int l) -> bool { return (h+2*k)%2 == 0; },
	/*25*/ [] (int h, int k, int l) -> bool { return (h+2*k)%4 == 0; },
	/*26*/ [] (int h, int k, int l) -> bool { return h%2 == 0 && k%2 == 0; },
	/*27*/ [] (int h, int k, int l) -> bool { return k%2 == 0 && l%2 == 0; },
	/*28*/ [] (int h, int k, int l) -> bool { return h%2 == 0 && l%2 == 0; },
	/*29*/ [] (int h, int k, int l) -> bool { return (k+l)%4 == 0 && k%2 == 0 && l%2 == 0; },
	/*30*/ [] (int h, int k, int l) -> bool { return (h+l)%4 == 0 && h%2 == 0 && l%2 == 0; },
	/*31*/ [] (int h, int k, int l) -> bool { return (h+k)%4 == 0 && h%2 == 0 && k%2 == 0; },
};

// -> function "can_reflect" in Georg's Python code
bool SpaceGroup::HasReflection(int h, int k, int l) const
{
	bool bRef = g_vecConds[m_vecCond[REFL_HKL]](h,k,l);

	if(h == 0)
	{
		bRef = bRef && g_vecConds[m_vecCond[REFL_0KL]](h, k, l);
		if(k == 0)
			bRef = bRef && g_vecConds[m_vecCond[REFL_00L]](h, k, l);
		else if(l == 0)
			bRef = bRef && g_vecConds[m_vecCond[REFL_0K0]](h, k, l);
		else if(k == l)
			bRef = bRef && g_vecConds[m_vecCond[REFL_0KK]](h, k, l);
	}
	if(k == 0)
	{
		bRef = bRef && g_vecConds[m_vecCond[REFL_H0L]](h, k, l);
		if(l == 0)
			bRef = bRef && g_vecConds[m_vecCond[REFL_H00]](h, k, l);
		else if(h == l)
			bRef = bRef && g_vecConds[m_vecCond[REFL_H0H]](h, k, l);
	}
	if(l == 0)
	{
		bRef = bRef && g_vecConds[m_vecCond[REFL_HK0]](h, k, l);
		if(h == k)
			bRef = bRef && g_vecConds[m_vecCond[REFL_HH0]](h, k, l);
	}
	if(h == k)
	{
		bRef = bRef && g_vecConds[m_vecCond[REFL_HHL]](h, k, l);
		if(h == l)
			bRef = bRef && g_vecConds[m_vecCond[REFL_HHH]](h, k, l);
	}
	if(k == l)
		bRef = bRef && g_vecConds[m_vecCond[REFL_HKK]](h, k, l);
	if(h == l)
		bRef = bRef && g_vecConds[m_vecCond[REFL_HKH]](h, k, l);

	return bRef;
}



static t_mapSpaceGroups g_mapSpaceGroups;


// -> sg_by_hm and sg_by_num in Georg's Python code
void init_space_groups()
{
	g_mapSpaceGroups =
	{
			{"P2_13", SpaceGroup({3, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0})}
	};

	// set the names for all space groups
	for(auto& pair : g_mapSpaceGroups)
		pair.second.SetName(pair.first);
}

const t_mapSpaceGroups& get_space_groups()
{
	if(g_mapSpaceGroups.size() == 0)
		init_space_groups();

	return g_mapSpaceGroups;
}
