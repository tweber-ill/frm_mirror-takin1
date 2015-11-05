/*
 * Wrapper for clipper spacegroups (alternate spacegroup implementation)
 * @author Tobias Weber
 * @date oct-2015
 * @license GPLv2
 */

#ifndef __TAKIN_SGCLP_H__
#define __TAKIN_SGCLP_H__

#ifndef USE_CLP
	#error USE_CLP is not defined!
#endif

#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <boost/numeric/ublas/matrix.hpp>
#include <clipper/clipper.h>
#include "crystalsys.h"

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
};




typedef std::map<std::string, SpaceGroup> t_mapSpaceGroups;
extern const t_mapSpaceGroups* get_space_groups();

extern void convert_hm_symbol(std::string& strHM);


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
		if(i!=mat.size1()-1) ostr << "\n";
	}

	return ostr.str();
}

#endif
