/*
 * Wrapper for clipper's form factor tables
 * @author Tobias Weber
 * @date nov-2015
 * @license GPLv2
 */

#ifndef __TAKIN_FFACT_H__
#define __TAKIN_FFACT_H__

#include <string>
#include <vector>
#include "tlibs/helper/array.h"
#include "tlibs/math/atoms.h"


class FormfactList;

template<typename T=double>
struct Formfact
{
	friend class FormfactList;

	protected:
		std::string strAtom;

		std::vector<T> a;
		std::vector<T> b;
		T c;

	public:
		const std::string& GetAtomName() const { return strAtom; }

		T GetFormfact(T G) const
		{
			return tl::formfact<T, std::vector>(G, a, b, c);
		}
};


class FormfactList
{
	private:
		static void Init();

	protected:
		static std::vector<Formfact<double>> s_vecFormfact;

	public:
		FormfactList();
		virtual ~FormfactList();

		unsigned int GetNumFormfacts() const { return s_vecFormfact.size(); }
		const Formfact<double>& GetFormfact(unsigned int iFormfact) const 
		{ return s_vecFormfact[iFormfact]; }
};


#endif
