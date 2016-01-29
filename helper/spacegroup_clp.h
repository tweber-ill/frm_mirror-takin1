/*
 * Wrapper for clipper spacegroups (alternate spacegroup implementation)
 * @author Tobias Weber
 * @date oct-2015
 * @license GPLv2
 */

#ifndef __TAKIN_SGCLP_H__
#define __TAKIN_SGCLP_H__

#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <algorithm>
#include <iterator>
#include <boost/numeric/ublas/matrix.hpp>
#include <clipper/clipper.h>
#include "crystalsys.h"
#include "tlibs/string/string.h"


namespace ublas = boost::numeric::ublas;


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

		void GetSymTrafos(std::vector<ublas::matrix<double>>& vecTrafos) const;
};




typedef std::map<std::string, SpaceGroup> t_mapSpaceGroups;
extern const t_mapSpaceGroups* get_space_groups();
extern void init_space_groups();


template<class T=clipper::ftype>
ublas::matrix<T> symop_to_matrix(const clipper::Symop& symop)
{
	const clipper::Mat33<T>& mat = symop.rot();
	const clipper::Vec3<T>& vec = symop.trn();

	ublas::matrix<T> matRet(4,4);
	for(int i=0; i<3; ++i)
		for(int j=0; j<3; ++j)
			matRet(i,j) = mat(i,j);

	for(int i=0; i<3; ++i)
	{
		matRet(i,3) = vec[i];
		matRet(3,i) = 0.;
	}

	matRet(3,3) = 1.;
	return matRet;
}


template<class T=clipper::ftype>
void get_symtrafos(const clipper::Spacegroup& sg, std::vector<ublas::matrix<T>>& vecTrafos)
{
	const int iNumSymOps = sg.num_symops();

	vecTrafos.clear();
	vecTrafos.reserve(iNumSymOps);

	for(int iSymOp=0; iSymOp<iNumSymOps; ++iSymOp)
	{
		const clipper::Symop& symop = sg.symop(iSymOp);
		ublas::matrix<T> mat = symop_to_matrix<T>(symop);

		vecTrafos.push_back(mat);
	}
}


template<class T=clipper::ftype>
std::string print_matrix(const ublas::matrix<T>& mat)
{
	std::ostringstream ostr;
	ostr.precision(4);

	for(int i=0; i<int(mat.size1()); ++i)
	{
		ostr << "(";

		for(int j=0; j<int(mat.size2()); ++j)
			ostr << std::setw(8) << mat(i,j);

		ostr << ")";
		if(i!=int(mat.size1())-1) ostr << "\n";
	}

	return ostr.str();
}

// convert e.g.: "P 21 3"  ->  "P2_13"
template<class t_str=std::string>
void convert_hm_symbol(t_str& strHM)
{
	std::vector<t_str> vecSyms;
	tl::get_tokens<t_str, t_str, decltype(vecSyms)>(strHM, " ", vecSyms);

	for(t_str& str : vecSyms)
	{
		bool bLastWasDigit = 0;
		for(std::size_t iC = 0; iC<str.length(); ++iC)
		{
			typename t_str::value_type c = str[iC];
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
	for(const t_str& str : vecSyms)
		strHM += str /*+ " "*/;
}


// get PG from SG, eg.: P2_13 -> 23
template<class t_str=std::string>
t_str get_pointgroup(const t_str& str)
{
	t_str strRet;

	// remove cell centering symbol
	std::remove_copy_if(str.begin(), str.end(),
		std::back_insert_iterator<t_str>(strRet),
		[](typename t_str::value_type c)->bool { return std::isupper(c); });

	// remove screw axes
	while(1)
	{
		std::size_t iPosScrew = strRet.find('_');
		if(iPosScrew == t_str::npos)
			break;
		strRet.erase(iPosScrew, 2);
	}

	// mirror plane
	std::replace_if(strRet.begin(), strRet.end(),
		[](typename t_str::value_type c)->bool
		{ return c=='a'||c=='b'||c=='c'||c=='d'||c=='e'||c=='f'||c=='n'; },
		'm' );
	return strRet;
}

#endif
