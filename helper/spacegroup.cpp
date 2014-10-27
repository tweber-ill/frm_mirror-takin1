/*
 * Determine allowed reflexes from space group
 * @author Georg Brandl (author of original Python version "spacegroups.py")
 * @author Tobias Weber (C++ port, i.e. author of this version)
 * @date 19-mar-2014
 *
 * Georg's original Python version:
 * http://forge.frm2.tum.de/cgit/cgit.cgi/frm2/nicos/nicos-core.git/tree/nicos/devices/tas/spacegroups.py
 *
 * Data sets from "spacegroups.py" are from PowderCell by W. Kraus and G. Nolze:
 * http://www.bam.de/de/service/publikationen/powder_cell_a.htm
 */

#include "spacegroup.h"
#include <cstdlib>

const char* get_crystal_system_name(CrystalSystem ty)
{
	switch(ty)
	{
		case CRYS_NOT_SET: return "<not set>";
		case CRYS_TRICLINIC: return "triclinic";
		case CRYS_MONOCLINIC: return "monoclinic";
		case CRYS_ORTHORHOMBIC: return "orthorhombic";
		case CRYS_TETRAGONAL: return "tetragonal";
		case CRYS_TRIGONAL: return "trigonal";
		case CRYS_HEXAGONAL: return "hexagonal";
		case CRYS_CUBIC: return "cubic";
	}

	return "<unknown>";
}

// Python-like modulo
// from: http://stackoverflow.com/questions/4003232/how-to-code-a-modulo-operator-in-c-c-obj-c-that-handles-negative-numbers
template<typename t_int=int> static inline t_int pymod(t_int a, t_int b)
{
	t_int m = a%b;
	if(m < 0)
		m += b;
	return m;
}

// -> function "check_refcond" in Georg's Python code
// -> http://www.ccp14.ac.uk/ccp/web-mirrors/powdcell/a_v/v_1/powder/details/extinct.htm
static std::array<bool (*)(int h, int k, int l), 32> g_vecConds =
{
	/*00, P*/ [] (int, int, int) -> bool { return 1; },
	
	/*01*/ [] (int h, int, int) -> bool { return h%2 == 0; },
	/*02*/ [] (int, int k, int) -> bool { return k%2 == 0; },
	/*03*/ [] (int, int, int l) -> bool { return l%2 == 0; },

	/*04, A*/ [] (int, int k, int l) -> bool { return (k+l)%2 == 0; },
	/*05, B*/ [] (int h, int, int l) -> bool { return (h+l)%2 == 0; },
	/*06, C*/ [] (int h, int k, int) -> bool { return (h+k)%2 == 0; },

	/*07*/ [] (int h, int k, int l) -> bool { return (pymod(h,2)==pymod(k,2)) && (pymod(k,2)==pymod(l,2)); },

	/*08*/ [] (int, int k, int l) -> bool { return (k+l)%4 == 0; },
	/*09*/ [] (int h, int, int l) -> bool { return (h+l)%4 == 0; },
	/*10*/ nullptr,

	/*11*/ nullptr,
	/*12*/ [] (int h, int, int l) -> bool { return (2*h+l)%4 == 0; },
	
	/*13, I*/ [] (int h, int k, int l) -> bool { return (h+k+l)%2 == 0; },
	/*14, R*/ [] (int h, int k, int l) -> bool { return (-h+k+l)%3 == 0; },
	/*15, R*/ nullptr,

	/*16*/ [] (int h, int, int) -> bool { return h%4 == 0; },
	/*17*/ [] (int, int k, int) -> bool { return k%4 == 0; },
	/*18*/ [] (int, int, int l) -> bool { return l%3 == 0; },
	/*19*/ [] (int, int, int l) -> bool { return l%4 == 0; },
	/*20*/ [] (int, int, int l) -> bool { return l%6 == 0; },

	/*21*/ nullptr,
	/*22*/ nullptr,
	/*23*/ [] (int h, int k, int) -> bool { return (2*h+k)%4 == 0; },
	/*24*/ nullptr,
	/*25*/ [] (int h, int k, int) -> bool { return (h+2*k)%4 == 0; },

	/*26*/ [] (int h, int k, int) -> bool { return h%2 == 0 && k%2 == 0; },
	/*27*/ [] (int, int k, int l) -> bool { return k%2 == 0 && l%2 == 0; },
	/*28*/ [] (int h, int, int l) -> bool { return h%2 == 0 && l%2 == 0; },

	/*29*/ [] (int, int k, int l) -> bool { return (k+l)%4 == 0 && k%2 == 0 && l%2 == 0; },
	/*30*/ [] (int h, int, int l) -> bool { return (h+l)%4 == 0 && h%2 == 0 && l%2 == 0; },
	/*31*/ [] (int h, int k, int) -> bool { return (h+k)%4 == 0 && h%2 == 0 && k%2 == 0; },
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
// -> http://www.ccp14.ac.uk/ccp/web-mirrors/powdcell/a_v/v_1/powder/details/pcwspgr.htm
// -> matched against CFML_Sym_Table.f90 in CrysFML
void init_space_groups()
{
	g_mapSpaceGroups =
	{
		// 1 - 2
		{"P1",  SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRICLINIC)},
		{"P-1", SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRICLINIC)},
		{"C-1", SpaceGroup({{0, 2, 2, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRICLINIC)},
		{"C1",  SpaceGroup({{0, 2, 2, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRICLINIC)},
		{"B-1", SpaceGroup({{3, 0, 3, 1, 5, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRICLINIC)},
		{"B1",  SpaceGroup({{3, 0, 3, 1, 5, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRICLINIC)},
		{"I-1", SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRICLINIC)},
		{"I1",  SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRICLINIC)},
		{"A-1", SpaceGroup({{3, 2, 4, 0, 3, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRICLINIC)},
		{"A1",  SpaceGroup({{3, 2, 4, 0, 3, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRICLINIC)},
		{"F1",  SpaceGroup({{3, 2, 27, 1, 28, 26, 7, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRICLINIC)},
		{"F-1", SpaceGroup({{3, 2, 27, 1, 28, 26, 7, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRICLINIC)},

		// 3 - 15
		{"P121",     SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P2",       SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P211",     SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P112",     SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P112/m",   SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P2/m",     SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P2/m11",   SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P12/m1",   SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"Pm",       SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"Pm11",     SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P11m",     SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P1m1",     SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P12/a1",   SpaceGroup({{0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P1a1",     SpaceGroup({{0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P11a",     SpaceGroup({{0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P112/a",   SpaceGroup({{0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P2_1/m11", SpaceGroup({{0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P2_111",   SpaceGroup({{0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P2_1",     SpaceGroup({{0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P12_1/m1", SpaceGroup({{0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P2_1/m",   SpaceGroup({{0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P12_11",   SpaceGroup({{0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"Pb11",     SpaceGroup({{0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P2/b11",   SpaceGroup({{0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P11b",     SpaceGroup({{0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P112/b",   SpaceGroup({{0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P12_1/a1", SpaceGroup({{0, 2, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P2_1/b11", SpaceGroup({{0, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P11n",     SpaceGroup({{0, 2, 0, 1, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P112/n",   SpaceGroup({{0, 2, 0, 1, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"C121",     SpaceGroup({{0, 2, 2, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"C2",       SpaceGroup({{0, 2, 2, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"Cm",       SpaceGroup({{0, 2, 2, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"C2/m",     SpaceGroup({{0, 2, 2, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"C1m1",     SpaceGroup({{0, 2, 2, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"C12/m1",   SpaceGroup({{0, 2, 2, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"C211",     SpaceGroup({{0, 2, 2, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"C2/m11",   SpaceGroup({{0, 2, 2, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"Cm11",     SpaceGroup({{0, 2, 2, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P112_1",   SpaceGroup({{3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P112_1/m", SpaceGroup({{3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P1c1",     SpaceGroup({{3, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P12/c1",   SpaceGroup({{3, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"Pc",       SpaceGroup({{3, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P2/c",     SpaceGroup({{3, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P12/n1",   SpaceGroup({{3, 0, 0, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P1n1",     SpaceGroup({{3, 0, 0, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"Pc11",     SpaceGroup({{3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P2/c11",   SpaceGroup({{3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P2_1/c11", SpaceGroup({{3, 0, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P112_1/a", SpaceGroup({{3, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"B2/m11",   SpaceGroup({{3, 0, 3, 1, 5, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"B211",     SpaceGroup({{3, 0, 3, 1, 5, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"B112/m",   SpaceGroup({{3, 0, 3, 1, 5, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"Bm11",     SpaceGroup({{3, 0, 3, 1, 5, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"B11m",     SpaceGroup({{3, 0, 3, 1, 5, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"B112",     SpaceGroup({{3, 0, 3, 1, 5, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P112_1/b", SpaceGroup({{3, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P2_1/c",   SpaceGroup({{3, 2, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P12_1/c1", SpaceGroup({{3, 2, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P12_1/n1", SpaceGroup({{3, 2, 0, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P112_1/n", SpaceGroup({{3, 2, 0, 1, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"C12/c1",   SpaceGroup({{3, 2, 2, 1, 28, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"C1c1",     SpaceGroup({{3, 2, 2, 1, 28, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"Cc",       SpaceGroup({{3, 2, 2, 1, 28, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"C1n1",     SpaceGroup({{3, 2, 2, 1, 28, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"C12/n1",   SpaceGroup({{3, 2, 2, 1, 28, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"C2/c",     SpaceGroup({{3, 2, 2, 1, 28, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"B11n",     SpaceGroup({{3, 2, 3, 1, 5, 26, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"B11b",     SpaceGroup({{3, 2, 3, 1, 5, 26, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"B112/n",   SpaceGroup({{3, 2, 3, 1, 5, 26, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"B112/b",   SpaceGroup({{3, 2, 3, 1, 5, 26, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"Pn11",     SpaceGroup({{3, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P2/n11",   SpaceGroup({{3, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"A1m1",     SpaceGroup({{3, 2, 4, 0, 3, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"A112/m",   SpaceGroup({{3, 2, 4, 0, 3, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"A12/m1",   SpaceGroup({{3, 2, 4, 0, 3, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"A11m",     SpaceGroup({{3, 2, 4, 0, 3, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"A112",     SpaceGroup({{3, 2, 4, 0, 3, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"A121",     SpaceGroup({{3, 2, 4, 0, 3, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"P2_1/n11", SpaceGroup({{3, 2, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"Cn11",     SpaceGroup({{3, 2, 4, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"A112/n",   SpaceGroup({{3, 2, 4, 1, 3, 26, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"A112/a",   SpaceGroup({{3, 2, 4, 1, 3, 26, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"A11n",     SpaceGroup({{3, 2, 4, 1, 3, 26, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"A11a",     SpaceGroup({{3, 2, 4, 1, 3, 26, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"I121",     SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"I12/m1",   SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"Im11",     SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"I211",     SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"I11m",     SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"I1m1",     SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"I2/m11",   SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"I112",     SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"I112/m",   SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"I112/a",   SpaceGroup({{3, 2, 4, 1, 5, 26, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"I11a",     SpaceGroup({{3, 2, 4, 1, 5, 26, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"I11b",     SpaceGroup({{3, 2, 4, 1, 5, 26, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"I112/b",   SpaceGroup({{3, 2, 4, 1, 5, 26, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"I12/a1",   SpaceGroup({{3, 2, 4, 1, 28, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"I12/c1",   SpaceGroup({{3, 2, 4, 1, 28, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"I1a1",     SpaceGroup({{3, 2, 4, 1, 28, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"A12/n1",   SpaceGroup({{3, 2, 4, 1, 28, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"A1n1",     SpaceGroup({{3, 2, 4, 1, 28, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"A1a1",     SpaceGroup({{3, 2, 4, 1, 28, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"A12/a1",   SpaceGroup({{3, 2, 4, 1, 28, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"I1c1",     SpaceGroup({{3, 2, 4, 1, 28, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"Ib11",     SpaceGroup({{3, 2, 27, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"I2/c11",   SpaceGroup({{3, 2, 27, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"Ic11",     SpaceGroup({{3, 2, 27, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"I2/b11",   SpaceGroup({{3, 2, 27, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"B2/b11",   SpaceGroup({{3, 2, 27, 1, 5, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"Bb11",     SpaceGroup({{3, 2, 27, 1, 5, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"B2/n11",   SpaceGroup({{3, 2, 27, 1, 5, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"Bn11",     SpaceGroup({{3, 2, 27, 1, 5, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"Cc11",     SpaceGroup({{3, 2, 27, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"C2/n11",   SpaceGroup({{3, 2, 27, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"C2/c11",   SpaceGroup({{3, 2, 27, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		{"F121",     SpaceGroup({{3, 2, 27, 1, 28, 26, 7, 0, 0, 0, 0, 0, 0, 0}}, CRYS_MONOCLINIC)},
		
		// 16 - 74
		{"P2mm",             SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pmm2",             SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pm2m",             SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pmmm",             SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"P222",             SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"P2_122",           SpaceGroup({{0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pmaa",             SpaceGroup({{0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"P2aa",             SpaceGroup({{0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pmma",             SpaceGroup({{0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"P2_1ma",           SpaceGroup({{0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pm2a",             SpaceGroup({{0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pma2",             SpaceGroup({{0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pmam",             SpaceGroup({{0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"P2_1am",           SpaceGroup({{0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"P22_12",           SpaceGroup({{0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pb2_1m",           SpaceGroup({{0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pbm2",             SpaceGroup({{0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pbmm",             SpaceGroup({{0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"P2mb",             SpaceGroup({{0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pmmb",             SpaceGroup({{0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pm2_1b",           SpaceGroup({{0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"P2_12_12",         SpaceGroup({{0, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pm2_1n",           SpaceGroup({{0, 2, 0, 1, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"P2_1mn",           SpaceGroup({{0, 2, 0, 1, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pmmn",             SpaceGroup({{0, 2, 0, 1, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"P2_1ab",           SpaceGroup({{0, 2, 0, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pmab",             SpaceGroup({{0, 2, 0, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pman",             SpaceGroup({{0, 2, 0, 1, 1, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"P2an",             SpaceGroup({{0, 2, 0, 1, 1, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pbmb",             SpaceGroup({{0, 2, 2, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pb2b",             SpaceGroup({{0, 2, 2, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pbmn",             SpaceGroup({{0, 2, 2, 1, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pb2n",             SpaceGroup({{0, 2, 2, 1, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pb2_1a",           SpaceGroup({{0, 2, 2, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pbma",             SpaceGroup({{0, 2, 2, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pbam",             SpaceGroup({{0, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pba2",             SpaceGroup({{0, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pbaa",             SpaceGroup({{0, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pbab",             SpaceGroup({{0, 2, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pban",             SpaceGroup({{0, 2, 2, 1, 1, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Cmm2",             SpaceGroup({{0, 2, 2, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"C222",             SpaceGroup({{0, 2, 2, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Cmmm",             SpaceGroup({{0, 2, 2, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Cm2m",             SpaceGroup({{0, 2, 2, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"C2mm",             SpaceGroup({{0, 2, 2, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Cm2a",             SpaceGroup({{0, 2, 2, 1, 1, 26, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"C2mb",             SpaceGroup({{0, 2, 2, 1, 1, 26, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Cmmb",             SpaceGroup({{0, 2, 2, 1, 1, 26, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Cmma",             SpaceGroup({{0, 2, 2, 1, 1, 26, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Cmme",             SpaceGroup({{0, 2, 2, 1, 1, 26, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"P222_1",           SpaceGroup({{3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pmcm",             SpaceGroup({{3, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"P2cm",             SpaceGroup({{3, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pmc2_1",           SpaceGroup({{3, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"P2_122_1",         SpaceGroup({{3, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pmca",             SpaceGroup({{3, 0, 0, 1, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"P2_1ca",           SpaceGroup({{3, 0, 0, 1, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pmn2_1",           SpaceGroup({{3, 0, 0, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"P2_1nm",           SpaceGroup({{3, 0, 0, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pmnm",             SpaceGroup({{3, 0, 0, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pmna",             SpaceGroup({{3, 0, 0, 1, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"P2na",             SpaceGroup({{3, 0, 0, 1, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pcmm",             SpaceGroup({{3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pcm2_1",           SpaceGroup({{3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pc2m",             SpaceGroup({{3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pcc2",             SpaceGroup({{3, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pccm",             SpaceGroup({{3, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Bma2",             SpaceGroup({{3, 0, 3, 1, 28, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Bmam",             SpaceGroup({{3, 0, 3, 1, 28, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Bmcm",             SpaceGroup({{3, 0, 3, 1, 28, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"B2cm",             SpaceGroup({{3, 0, 3, 1, 28, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pcma",             SpaceGroup({{3, 0, 3, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pc2a",             SpaceGroup({{3, 0, 3, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pca2_1",           SpaceGroup({{3, 0, 3, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pcam",             SpaceGroup({{3, 0, 3, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pcaa",             SpaceGroup({{3, 0, 3, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pcca",             SpaceGroup({{3, 0, 3, 1, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pcnm",             SpaceGroup({{3, 0, 3, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pcn2",             SpaceGroup({{3, 0, 3, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pcna",             SpaceGroup({{3, 0, 3, 1, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Bmmm",             SpaceGroup({{3, 0, 3, 1, 5, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"B222",             SpaceGroup({{3, 0, 3, 1, 5, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Bm2m",             SpaceGroup({{3, 0, 3, 1, 5, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"B2mm",             SpaceGroup({{3, 0, 3, 1, 5, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Bmm2",             SpaceGroup({{3, 0, 3, 1, 5, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"P22_12_1",         SpaceGroup({{3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"P2cb",             SpaceGroup({{3, 2, 0, 0, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pmcb",             SpaceGroup({{3, 2, 0, 0, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"P2_12_12_1",       SpaceGroup({{3, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pmcn",             SpaceGroup({{3, 2, 0, 1, 3, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"P2_1cn",           SpaceGroup({{3, 2, 0, 1, 3, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"P2_1nb",           SpaceGroup({{3, 2, 0, 1, 5, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pmnb",             SpaceGroup({{3, 2, 0, 1, 5, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pmnn",             SpaceGroup({{3, 2, 0, 1, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"P2nn",             SpaceGroup({{3, 2, 0, 1, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pbc2_1",           SpaceGroup({{3, 2, 2, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pbcm",             SpaceGroup({{3, 2, 2, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pbcb",             SpaceGroup({{3, 2, 2, 0, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"C222_1",           SpaceGroup({{3, 2, 2, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pbnm",             SpaceGroup({{3, 2, 2, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pbn2_1",           SpaceGroup({{3, 2, 2, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pbna",             SpaceGroup({{3, 2, 2, 1, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pbnb",             SpaceGroup({{3, 2, 2, 1, 5, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pbnn",             SpaceGroup({{3, 2, 2, 1, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pbcn",             SpaceGroup({{3, 2, 2, 1, 3, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pbca",             SpaceGroup({{3, 2, 2, 1, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Cmc2_1",           SpaceGroup({{3, 2, 2, 1, 28, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Cmcm",             SpaceGroup({{3, 2, 2, 1, 28, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"C2cm",             SpaceGroup({{3, 2, 2, 1, 28, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"C2cb",             SpaceGroup({{3, 2, 2, 1, 28, 26, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Cmce",             SpaceGroup({{3, 2, 2, 1, 28, 26, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Cmca",             SpaceGroup({{3, 2, 2, 1, 28, 26, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pcmb",             SpaceGroup({{3, 2, 3, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pc2_1b",           SpaceGroup({{3, 2, 3, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pccb",             SpaceGroup({{3, 2, 3, 0, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pcmn",             SpaceGroup({{3, 2, 3, 1, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pc2_1n",           SpaceGroup({{3, 2, 3, 1, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pcan",             SpaceGroup({{3, 2, 3, 1, 1, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"B22_12",           SpaceGroup({{3, 2, 3, 1, 5, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pcnb",             SpaceGroup({{3, 2, 3, 1, 5, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pcab",             SpaceGroup({{3, 2, 3, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pccn",             SpaceGroup({{3, 2, 3, 1, 3, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pcnn",             SpaceGroup({{3, 2, 3, 1, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Bmmb",             SpaceGroup({{3, 2, 3, 1, 5, 26, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Bm2_1b",           SpaceGroup({{3, 2, 3, 1, 5, 26, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"B2mb",             SpaceGroup({{3, 2, 3, 1, 5, 26, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"B2cb",             SpaceGroup({{3, 2, 3, 1, 28, 26, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Bmab",             SpaceGroup({{3, 2, 3, 1, 28, 26, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pnmm",             SpaceGroup({{3, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pn2_1m",           SpaceGroup({{3, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pnm2_1",           SpaceGroup({{3, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pnmb",             SpaceGroup({{3, 2, 4, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pn2b",             SpaceGroup({{3, 2, 4, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pncm",             SpaceGroup({{3, 2, 4, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pnc2",             SpaceGroup({{3, 2, 4, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pncb",             SpaceGroup({{3, 2, 4, 0, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Am2m",             SpaceGroup({{3, 2, 4, 0, 3, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"A2mm",             SpaceGroup({{3, 2, 4, 0, 3, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Ammm",             SpaceGroup({{3, 2, 4, 0, 3, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"A222",             SpaceGroup({{3, 2, 4, 0, 3, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Amm2",             SpaceGroup({{3, 2, 4, 0, 3, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pn2_1a",           SpaceGroup({{3, 2, 4, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pnma",             SpaceGroup({{3, 2, 4, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pnmn",             SpaceGroup({{3, 2, 4, 1, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pn2n",             SpaceGroup({{3, 2, 4, 1, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pnam",             SpaceGroup({{3, 2, 4, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pna2_1",           SpaceGroup({{3, 2, 4, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pnaa",             SpaceGroup({{3, 2, 4, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pnab",             SpaceGroup({{3, 2, 4, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pnan",             SpaceGroup({{3, 2, 4, 1, 1, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pnca",             SpaceGroup({{3, 2, 4, 1, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"A2_122",           SpaceGroup({{3, 2, 4, 1, 3, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pncn",             SpaceGroup({{3, 2, 4, 1, 3, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Am2a",             SpaceGroup({{3, 2, 4, 1, 3, 26, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"A2_1ma",           SpaceGroup({{3, 2, 4, 1, 3, 26, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Amma",             SpaceGroup({{3, 2, 4, 1, 3, 26, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pnn2",             SpaceGroup({{3, 2, 4, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pnnm",             SpaceGroup({{3, 2, 4, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pnna",             SpaceGroup({{3, 2, 4, 1, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pnnb",             SpaceGroup({{3, 2, 4, 1, 5, 2, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Pnnn",             SpaceGroup({{3, 2, 4, 1, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"I2_12_12_1",       SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Imm2",             SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Immm",             SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"I222",             SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"I2mm",             SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Im2m",             SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Immb",             SpaceGroup({{3, 2, 4, 1, 5, 26, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Im2a",             SpaceGroup({{3, 2, 4, 1, 5, 26, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"I2mb",             SpaceGroup({{3, 2, 4, 1, 5, 26, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Imma",             SpaceGroup({{3, 2, 4, 1, 5, 26, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"I2_1/m2_1/m2_1/a", SpaceGroup({{3, 2, 4, 1, 5, 26, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Amam",             SpaceGroup({{3, 2, 4, 1, 28, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Ama2",             SpaceGroup({{3, 2, 4, 1, 28, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"A2_1am",           SpaceGroup({{3, 2, 4, 1, 28, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Ima2",             SpaceGroup({{3, 2, 4, 1, 28, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Imam",             SpaceGroup({{3, 2, 4, 1, 28, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Imcm",             SpaceGroup({{3, 2, 4, 1, 28, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"I2cm",             SpaceGroup({{3, 2, 4, 1, 28, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Amaa",             SpaceGroup({{3, 2, 4, 1, 28, 26, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"A2aa",             SpaceGroup({{3, 2, 4, 1, 28, 26, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"I2cb",             SpaceGroup({{3, 2, 4, 1, 28, 26, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Imcb",             SpaceGroup({{3, 2, 4, 1, 28, 26, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Abm2",             SpaceGroup({{3, 2, 27, 0, 3, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Abmm",             SpaceGroup({{3, 2, 27, 0, 3, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Ac2m",             SpaceGroup({{3, 2, 27, 0, 3, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Aem2",             SpaceGroup({{3, 2, 27, 0, 3, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Acmm",             SpaceGroup({{3, 2, 27, 0, 3, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Ccm2_1",           SpaceGroup({{3, 2, 27, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Cc2m",             SpaceGroup({{3, 2, 27, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Ccmm",             SpaceGroup({{3, 2, 27, 1, 1, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Cc2a",             SpaceGroup({{3, 2, 27, 1, 1, 26, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Ccmb",             SpaceGroup({{3, 2, 27, 1, 1, 26, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Abma",             SpaceGroup({{3, 2, 27, 1, 3, 26, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Ac2a",             SpaceGroup({{3, 2, 27, 1, 3, 26, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Bbm2",             SpaceGroup({{3, 2, 27, 1, 5, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Bbmm",             SpaceGroup({{3, 2, 27, 1, 5, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Bb2_1m",           SpaceGroup({{3, 2, 27, 1, 5, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Ibm2",             SpaceGroup({{3, 2, 27, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Ic2m",             SpaceGroup({{3, 2, 27, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Ibmm",             SpaceGroup({{3, 2, 27, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Icmm",             SpaceGroup({{3, 2, 27, 1, 5, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Bb2b",             SpaceGroup({{3, 2, 27, 1, 5, 26, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Bbmb",             SpaceGroup({{3, 2, 27, 1, 5, 26, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Icma",             SpaceGroup({{3, 2, 27, 1, 5, 26, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Ic2a",             SpaceGroup({{3, 2, 27, 1, 5, 26, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Bba2",             SpaceGroup({{3, 2, 27, 1, 28, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Bbcm",             SpaceGroup({{3, 2, 27, 1, 28, 1, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Acam",             SpaceGroup({{3, 2, 27, 1, 28, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Aba2",             SpaceGroup({{3, 2, 27, 1, 28, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Aea2",             SpaceGroup({{3, 2, 27, 1, 28, 2, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Cccm",             SpaceGroup({{3, 2, 27, 1, 28, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Ccc2",             SpaceGroup({{3, 2, 27, 1, 28, 6, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Iba2",             SpaceGroup({{3, 2, 27, 1, 28, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Ibam",             SpaceGroup({{3, 2, 27, 1, 28, 6, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Abaa",             SpaceGroup({{3, 2, 27, 1, 28, 26, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Acaa",             SpaceGroup({{3, 2, 27, 1, 28, 26, 4, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Bbab",             SpaceGroup({{3, 2, 27, 1, 28, 26, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Bbcb",             SpaceGroup({{3, 2, 27, 1, 28, 26, 5, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Ccce",             SpaceGroup({{3, 2, 27, 1, 28, 26, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Cccb",             SpaceGroup({{3, 2, 27, 1, 28, 26, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Ccca",             SpaceGroup({{3, 2, 27, 1, 28, 26, 6, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"F222",             SpaceGroup({{3, 2, 27, 1, 28, 26, 7, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Fmm2",             SpaceGroup({{3, 2, 27, 1, 28, 26, 7, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Fm2m",             SpaceGroup({{3, 2, 27, 1, 28, 26, 7, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Fmmm",             SpaceGroup({{3, 2, 27, 1, 28, 26, 7, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"F2mm",             SpaceGroup({{3, 2, 27, 1, 28, 26, 7, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Ibca",             SpaceGroup({{3, 2, 27, 1, 28, 26, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Icab",             SpaceGroup({{3, 2, 27, 1, 28, 26, 13, 0, 0, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"F2dd",             SpaceGroup({{19, 17, 0, 16, 30, 31, 7, 0, 31, 0, 30, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Fd2d",             SpaceGroup({{19, 17, 29, 16, 0, 31, 7, 29, 31, 0, 0, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Fdd2",             SpaceGroup({{19, 17, 29, 16, 30, 0, 7, 29, 0, 0, 30, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},
		{"Fddd",             SpaceGroup({{19, 17, 29, 16, 30, 31, 7, 29, 31, 0, 30, 0, 0, 0}}, CRYS_ORTHORHOMBIC)},

		// 75 - 142
		{"P4mm",           SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4",             SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P422",           SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4/m",           SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4/mmm",         SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P-4m2",          SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P-42m",          SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P-4",            SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4/m2/m2/m",     SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P42_12",         SpaceGroup({{0, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P-42_1m",        SpaceGroup({{0, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4/n2_1/m2/m",   SpaceGroup({{0, 2, 0, 1, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4/nmm",         SpaceGroup({{0, 2, 0, 1, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4/n",           SpaceGroup({{0, 2, 0, 1, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4/m2_1/b2/m",   SpaceGroup({{0, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4/mbm",         SpaceGroup({{0, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4bm",           SpaceGroup({{0, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P-4b2",          SpaceGroup({{0, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4/n2/b2/m",     SpaceGroup({{0, 2, 2, 1, 1, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4/nbm",         SpaceGroup({{0, 2, 2, 1, 1, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4_222",         SpaceGroup({{3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4_2/m",         SpaceGroup({{3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4_2",           SpaceGroup({{3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4_2/m2/m2/c",   SpaceGroup({{3, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_TETRAGONAL)},
		{"P-42c",          SpaceGroup({{3, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_TETRAGONAL)},
		{"P4_2/mmc",       SpaceGroup({{3, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_TETRAGONAL)},
		{"P4_2mc",         SpaceGroup({{3, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_TETRAGONAL)},
		{"P-4c2",          SpaceGroup({{3, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4_2/mcm",       SpaceGroup({{3, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4_2cm",         SpaceGroup({{3, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4_2/m2/c2/m",   SpaceGroup({{3, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4/mcc",         SpaceGroup({{3, 0, 3, 0, 3, 0, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_TETRAGONAL)},
		{"P4/m2/c2/c",     SpaceGroup({{3, 0, 3, 0, 3, 0, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_TETRAGONAL)},
		{"P4cc",           SpaceGroup({{3, 0, 3, 0, 3, 0, 0, 2, 0, 3, 1, 0, 0, 1}}, CRYS_TETRAGONAL)},
		{"P4_22_12",       SpaceGroup({{3, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4_2/n",         SpaceGroup({{3, 2, 0, 1, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P-42_1c",        SpaceGroup({{3, 2, 0, 1, 0, 0, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_TETRAGONAL)},
		{"P4_2/n2_1/m2/c", SpaceGroup({{3, 2, 0, 1, 0, 6, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_TETRAGONAL)},
		{"P4_2/nmc",       SpaceGroup({{3, 2, 0, 1, 0, 6, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_TETRAGONAL)},
		{"P4_2bc",         SpaceGroup({{3, 2, 2, 1, 1, 0, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_TETRAGONAL)},
		{"P4_2/mbc",       SpaceGroup({{3, 2, 2, 1, 1, 0, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_TETRAGONAL)},
		{"P4_2/nbc",       SpaceGroup({{3, 2, 2, 1, 1, 6, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_TETRAGONAL)},
		{"P4_2/n2/b2/c",   SpaceGroup({{3, 2, 2, 1, 1, 6, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_TETRAGONAL)},
		{"P4_2/m2_1/b2/c", SpaceGroup({{3, 2, 2, 1, 1, 0, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_TETRAGONAL)},
		{"P4_2/n2_1/c2/m", SpaceGroup({{3, 2, 3, 1, 3, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4_2/ncm",       SpaceGroup({{3, 2, 3, 1, 3, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4/n2_1/c2/c",   SpaceGroup({{3, 2, 3, 1, 3, 6, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_TETRAGONAL)},
		{"P4/ncc",         SpaceGroup({{3, 2, 3, 1, 3, 6, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_TETRAGONAL)},
		{"P-4n2",          SpaceGroup({{3, 2, 4, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4_2/m2_1/n2/m", SpaceGroup({{3, 2, 4, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4_2/mnm",       SpaceGroup({{3, 2, 4, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4_2nm",         SpaceGroup({{3, 2, 4, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4nc",           SpaceGroup({{3, 2, 4, 1, 5, 0, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_TETRAGONAL)},
		{"P4/m2_1/n2/c",   SpaceGroup({{3, 2, 4, 1, 5, 0, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_TETRAGONAL)},
		{"P4/mnc",         SpaceGroup({{3, 2, 4, 1, 5, 0, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_TETRAGONAL)},
		{"P4_2/nnm",       SpaceGroup({{3, 2, 4, 1, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4_2/n2/n2/m",   SpaceGroup({{3, 2, 4, 1, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4/nnc",         SpaceGroup({{3, 2, 4, 1, 5, 6, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_TETRAGONAL)},
		{"P4/n2/n2/c",     SpaceGroup({{3, 2, 4, 1, 5, 6, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_TETRAGONAL)},
		{"I-42m",          SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 3, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"I-4m2",          SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 3, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"I4",             SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 3, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"I-4",            SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 3, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"I4/mmm",         SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 3, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"I422",           SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 3, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"I4mm",           SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 3, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"I4/m",           SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 3, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"I4/m2/m2/m",     SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 3, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"I-4c2",          SpaceGroup({{3, 2, 27, 1, 28, 6, 13, 0, 0, 3, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"I4/mcm",         SpaceGroup({{3, 2, 27, 1, 28, 6, 13, 0, 0, 3, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"I4cm",           SpaceGroup({{3, 2, 27, 1, 28, 6, 13, 0, 0, 3, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"I4/m2/c2/m",     SpaceGroup({{3, 2, 27, 1, 28, 6, 13, 0, 0, 3, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4_3",           SpaceGroup({{19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4_122",         SpaceGroup({{19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4_1",           SpaceGroup({{19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4_322",         SpaceGroup({{19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4_32_12",       SpaceGroup({{19, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"P4_12_12",       SpaceGroup({{19, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"I4_1",           SpaceGroup({{19, 2, 4, 1, 5, 6, 13, 0, 0, 3, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"I4_122",         SpaceGroup({{19, 2, 4, 1, 5, 6, 13, 0, 0, 3, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"I4_1md",         SpaceGroup({{19, 2, 4, 1, 5, 6, 13, 0, 1, 12, 0, 0, 0, 12}}, CRYS_TETRAGONAL)},
		{"I-42d",          SpaceGroup({{19, 2, 4, 1, 5, 6, 13, 0, 1, 12, 0, 0, 0, 12}}, CRYS_TETRAGONAL)},
		{"I4_1/a",         SpaceGroup({{19, 2, 4, 1, 5, 26, 13, 0, 0, 3, 0, 0, 0, 0}}, CRYS_TETRAGONAL)},
		{"I4_1/a2/m2/d",   SpaceGroup({{19, 2, 4, 1, 5, 26, 13, 0, 0, 12, 0, 0, 0, 12}}, CRYS_TETRAGONAL)},
		{"I4_1/amd",       SpaceGroup({{19, 2, 4, 1, 5, 26, 13, 0, 0, 12, 0, 0, 0, 12}}, CRYS_TETRAGONAL)},
		{"I4_1cd",         SpaceGroup({{19, 2, 27, 1, 28, 6, 13, 2, 1, 12, 1, 0, 0, 12}}, CRYS_TETRAGONAL)},
		{"I4_1/a2/c2/d",   SpaceGroup({{19, 2, 27, 1, 28, 26, 13, 0, 0, 12, 0, 0, 0, 12}}, CRYS_TETRAGONAL)},
		{"I4_1/acd",       SpaceGroup({{19, 2, 27, 1, 28, 26, 13, 0, 0, 12, 0, 0, 0, 12}}, CRYS_TETRAGONAL)},

		// 143 - 167
		{"P321",    SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"P-3m1",   SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"P312",    SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"P31m",    SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"P-31m",   SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"P-3",     SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"P3m1",    SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"P3",      SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"P-312/m", SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"P-32/m1", SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"P-31c",   SpaceGroup({{3, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 3}}, CRYS_TRIGONAL)},
		{"P31c",    SpaceGroup({{3, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 3}}, CRYS_TRIGONAL)},
		{"P-312/c", SpaceGroup({{3, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 3}}, CRYS_TRIGONAL)},
		{"P-3c1",   SpaceGroup({{3, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"P3c1",    SpaceGroup({{3, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"P-32/c1", SpaceGroup({{3, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"P3_121",  SpaceGroup({{18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"P3_212",  SpaceGroup({{18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"P3_112",  SpaceGroup({{18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"P3_2",    SpaceGroup({{18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"P3_1",    SpaceGroup({{18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"P3_221",  SpaceGroup({{18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"R3m",     SpaceGroup({{18, 0, 0, 0, 0, 0, 14, 0, 0, 18, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"R32",     SpaceGroup({{18, 0, 0, 0, 0, 0, 14, 0, 0, 18, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"R-3",     SpaceGroup({{18, 0, 0, 0, 0, 0, 14, 0, 0, 18, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"R-3m",    SpaceGroup({{18, 0, 0, 0, 0, 0, 14, 0, 0, 18, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"R3",      SpaceGroup({{18, 0, 0, 0, 0, 0, 14, 0, 0, 18, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"R-32/m",  SpaceGroup({{18, 0, 0, 0, 0, 0, 14, 0, 0, 18, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"R-32/c",  SpaceGroup({{20, 0, 0, 0, 0, 0, 14, 0, 0, 18, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"R-3c",    SpaceGroup({{20, 0, 0, 0, 0, 0, 14, 0, 0, 18, 0, 0, 0, 0}}, CRYS_TRIGONAL)},
		{"R3c",     SpaceGroup({{20, 0, 0, 0, 0, 0, 14, 0, 0, 18, 0, 0, 0, 0}}, CRYS_TRIGONAL)},

		// 168 - 194
		{"P-6m2",        SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P6/m",         SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P6mm",         SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P6",           SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P-6",          SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P6/m2/m2/m",   SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P6/mmm",       SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P622",         SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P-62m",        SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P6_322",       SpaceGroup({{3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P6_3",         SpaceGroup({{3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P6_3/m",       SpaceGroup({{3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P-62c",        SpaceGroup({{3, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P6_3/m2/m2/c", SpaceGroup({{3, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_HEXAGONAL)},
		{"P6_3mc",       SpaceGroup({{3, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_HEXAGONAL)},
		{"P6_3/mmc",     SpaceGroup({{3, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_HEXAGONAL)},
		{"P-6c2",        SpaceGroup({{3, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P6_3/m2/c2/m", SpaceGroup({{3, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P6_3cm",       SpaceGroup({{3, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P6_3/mcm",     SpaceGroup({{3, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P6/m2/c2/c",   SpaceGroup({{3, 0, 3, 0, 3, 0, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_HEXAGONAL)},
		{"P6cc",         SpaceGroup({{3, 0, 3, 0, 3, 0, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_HEXAGONAL)},
		{"P6/mcc",       SpaceGroup({{3, 0, 3, 0, 3, 0, 0, 0, 0, 3, 0, 0, 0, 1}}, CRYS_HEXAGONAL)},
		{"P6_422",       SpaceGroup({{18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P6_2",         SpaceGroup({{18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P6_4",         SpaceGroup({{18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P6_222",       SpaceGroup({{18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P6_522",       SpaceGroup({{20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P6_122",       SpaceGroup({{20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P6_1",         SpaceGroup({{20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},
		{"P6_5",         SpaceGroup({{20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_HEXAGONAL)},

		// 195 - 230
		{"Pm-3",        SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_CUBIC)},
		{"Pm-3m",       SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_CUBIC)},
		{"P23",         SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_CUBIC)},
		{"P432",        SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_CUBIC)},
		{"P-43m",       SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_CUBIC)},
		{"P4/m-32/m",   SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_CUBIC)},
		{"P2/m-3",      SpaceGroup({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_CUBIC)},
		{"P4_232",      SpaceGroup({{3, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_CUBIC)},
		{"P2_13",       SpaceGroup({{3, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_CUBIC)},
		{"P-43n",       SpaceGroup({{3, 2, 0, 1, 0, 0, 0, 0, 0, 3, 0, 1, 2, 1}}, CRYS_CUBIC)},
		{"Pm-3n",       SpaceGroup({{3, 2, 0, 1, 0, 0, 0, 0, 0, 3, 0, 1, 2, 1}}, CRYS_CUBIC)},
		{"P4_2/m-32/n", SpaceGroup({{3, 2, 0, 1, 0, 0, 0, 0, 0, 3, 0, 1, 2, 1}}, CRYS_CUBIC)},
		{"Pa-3",        SpaceGroup({{3, 2, 2, 1, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_CUBIC)},
		{"P2_1/a-3",    SpaceGroup({{3, 2, 2, 1, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_CUBIC)},
		{"Pn-3",        SpaceGroup({{3, 2, 4, 1, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_CUBIC)},
		{"P4_2/n-32/m", SpaceGroup({{3, 2, 4, 1, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_CUBIC)},
		{"P2/n-3",      SpaceGroup({{3, 2, 4, 1, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_CUBIC)},
		{"Pn-3m",       SpaceGroup({{3, 2, 4, 1, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_CUBIC)},
		{"Pn-3n",       SpaceGroup({{3, 2, 4, 1, 5, 6, 0, 0, 0, 3, 0, 1, 2, 1}}, CRYS_CUBIC)},
		{"P4/n-32/n",   SpaceGroup({{3, 2, 4, 1, 5, 6, 0, 0, 0, 3, 0, 1, 2, 1}}, CRYS_CUBIC)},
		{"I432",        SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 3, 0, 1, 2, 0}}, CRYS_CUBIC)},
		{"I23",         SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 3, 0, 1, 2, 0}}, CRYS_CUBIC)},
		{"I2_13",       SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 3, 0, 1, 2, 0}}, CRYS_CUBIC)},
		{"I-43m",       SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 3, 0, 1, 2, 0}}, CRYS_CUBIC)},
		{"Im-3m",       SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 3, 0, 1, 2, 0}}, CRYS_CUBIC)},
		{"I4/m-32/m",   SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 3, 0, 1, 2, 0}}, CRYS_CUBIC)},
		{"Im-3",        SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 3, 0, 1, 2, 0}}, CRYS_CUBIC)},
		{"I2/m-3",      SpaceGroup({{3, 2, 4, 1, 5, 6, 13, 0, 0, 3, 0, 1, 2, 0}}, CRYS_CUBIC)},
		{"F23",         SpaceGroup({{3, 2, 27, 1, 28, 26, 7, 0, 0, 5, 0, 6, 6, 0}}, CRYS_CUBIC)},
		{"Fm-3m",       SpaceGroup({{3, 2, 27, 1, 28, 26, 7, 0, 0, 5, 0, 6, 6, 0}}, CRYS_CUBIC)},
		{"F-43m",       SpaceGroup({{3, 2, 27, 1, 28, 26, 7, 0, 0, 5, 0, 6, 6, 0}}, CRYS_CUBIC)},
		{"F432",        SpaceGroup({{3, 2, 27, 1, 28, 26, 7, 0, 0, 5, 0, 6, 6, 0}}, CRYS_CUBIC)},
		{"F4/m-32/m",   SpaceGroup({{3, 2, 27, 1, 28, 26, 7, 0, 0, 5, 0, 6, 6, 0}}, CRYS_CUBIC)},
		{"Fm-3",        SpaceGroup({{3, 2, 27, 1, 28, 26, 7, 0, 0, 5, 0, 6, 6, 0}}, CRYS_CUBIC)},
		{"F2/m-3",      SpaceGroup({{3, 2, 27, 1, 28, 26, 7, 0, 0, 5, 0, 6, 6, 0}}, CRYS_CUBIC)},
		{"F-43c",       SpaceGroup({{3, 2, 27, 1, 28, 26, 7, 0, 0, 28, 0, 26, 26, 1}}, CRYS_CUBIC)},
		{"Fm-3c",       SpaceGroup({{3, 2, 27, 1, 28, 26, 7, 0, 0, 28, 0, 26, 26, 1}}, CRYS_CUBIC)},
		{"F4/m-32/c",   SpaceGroup({{3, 2, 27, 1, 28, 26, 7, 0, 0, 28, 0, 26, 26, 1}}, CRYS_CUBIC)},
		{"I2_1/a-3",    SpaceGroup({{3, 2, 27, 1, 28, 26, 13, 0, 0, 3, 0, 1, 2, 0}}, CRYS_CUBIC)},
		{"Ia-3",        SpaceGroup({{3, 2, 27, 1, 28, 26, 13, 0, 0, 3, 0, 1, 2, 0}}, CRYS_CUBIC)},
		{"P4_332",      SpaceGroup({{19, 17, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_CUBIC)},
		{"P4_132",      SpaceGroup({{19, 17, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, CRYS_CUBIC)},
		{"I-43d",       SpaceGroup({{19, 17, 4, 16, 5, 6, 13, 0, 12, 12, 0, 25, 23, 12}}, CRYS_CUBIC)},
		{"I4_132",      SpaceGroup({{19, 17, 4, 16, 5, 6, 13, 0, 0, 3, 0, 1, 2, 0}}, CRYS_CUBIC)},
		{"F4_1/d-32/m", SpaceGroup({{19, 17, 8, 16, 9, 31, 7, 0, 0, 5, 0, 6, 6, 0}}, CRYS_CUBIC)},
		{"Fd-3m",       SpaceGroup({{19, 17, 8, 16, 9, 31, 7, 0, 0, 5, 0, 6, 6, 0}}, CRYS_CUBIC)},
		{"Fd-3c",       SpaceGroup({{19, 17, 8, 16, 9, 31, 7, 0, 0, 28, 0, 26, 26, 1}}, CRYS_CUBIC)},
		{"F4_1/d-32/c", SpaceGroup({{19, 17, 8, 16, 9, 31, 7, 0, 0, 28, 0, 26, 26, 1}}, CRYS_CUBIC)},
		{"Fd-3",        SpaceGroup({{19, 17, 8, 16, 30, 31, 7, 0, 0, 5, 0, 6, 6, 0}}, CRYS_CUBIC)},
		{"F2/d-3",      SpaceGroup({{19, 17, 8, 16, 30, 31, 7, 0, 0, 5, 0, 6, 6, 0}}, CRYS_CUBIC)},
		{"F4_132",      SpaceGroup({{19, 17, 27, 16, 28, 26, 7, 0, 0, 5, 0, 6, 6, 0}}, CRYS_CUBIC)},
		{"I4_1/a-32/d", SpaceGroup({{19, 17, 27, 16, 28, 26, 13, 0, 0, 12, 0, 25, 23, 12}}, CRYS_CUBIC)},
		{"Ia-3d",       SpaceGroup({{19, 17, 27, 16, 28, 26, 13, 0, 0, 12, 0, 25, 23, 12}}, CRYS_CUBIC)},
	};

	// set the names for all space groups
	for(auto& pair : g_mapSpaceGroups)
		pair.second.SetName(pair.first);
}

const t_mapSpaceGroups* get_space_groups()
{
	if(g_mapSpaceGroups.empty())
		init_space_groups();

	return &g_mapSpaceGroups;
}


/*
// check against Georg's Python spacegroup module:
//
//from spacegroups import *
//
//def get_type(num):
//	if num>=1 and num<=2:
//		return "triclinic"
//	elif num>=3 and num<=15:
//		return "monoclinic"
//	elif num>=16 and num<=74:
//		return "orthorhombic"
//	elif num>=75 and num<=142:
//		return "tetragonal"
//	elif num>=143 and num<=167:
//		return "trigonal"
//	elif num>=168 and num<=194:
//		return "hexagonal"
//	elif num>=195 and num<=230:
//		return "cubic"
//
//sgs = []
//
//for sg in sg_by_hm:
//	numsg = sg_by_hm[sg][0]
//	sgs.append([sg, get_type(numsg), sg_by_num[sg_by_hm[sg]]])
//
//sgs = sorted(sgs)
//for sg in sgs:
//	print(sg[0] + " " + sg[1] + " " + str(sg[2]))
//

#include <iostream>
#include <set>
#include <algorithm>
#include <iterator>

int main()
{
	init_space_groups();
	
	std::set<unsigned char> setConds;
	std::set<unsigned char> setAll;
	for(unsigned char ch=0; ch<32; ++ch)
		setAll.insert(ch);
		
	std::cout << g_mapSpaceGroups.size() << " spacegroups.\n";

	for(const t_mapSpaceGroups::value_type& sg : g_mapSpaceGroups)
	{
		const std::string& strName = sg.first;
		const SpaceGroup& thesg = sg.second;
		
		std::cout << thesg.GetName() << " " << thesg.GetCrystalSystemName();
		std::cout << " " << "[";
		
		for(unsigned int iCond=0; iCond<thesg.GetConds().size(); ++iCond)
		{
			unsigned char ch = thesg.GetConds()[iCond];
			setConds.insert(ch);
			
			std::cout << unsigned(ch);
			if(iCond != thesg.GetConds().size()-1)
				std::cout << ", ";
		}
		std::cout << "]\n";
	}

	//std::cout << "Unused conditions: ";
	//std::set_difference(setAll.begin(), setAll.end(), setConds.begin(), setConds.end(),
	//					std::ostream_iterator<unsigned>(std::cout, ", "));
	std::cout << std::endl;
	return 0;
}*/
