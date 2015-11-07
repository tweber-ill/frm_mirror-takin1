/*
 * Form factor and scattering length tables
 * @author Tobias Weber
 * @date nov-2015
 * @license GPLv2
 */

#ifndef __TAKIN_FFACT_H__
#define __TAKIN_FFACT_H__

#include <string>
#include <vector>
#include <complex>
#include "tlibs/helper/array.h"
#include "tlibs/math/atoms.h"


// ----------------------------------------------------------------------------

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


// ----------------------------------------------------------------------------


class ScatlenList;

template<typename T=std::complex<double>>
struct Scatlen
{
	friend class ScatlenList;

	public:
		typedef T value_type;

	protected:
		std::string strAtom;
		value_type coh;
		value_type incoh;

	public:
		const std::string& GetAtomName() const { return strAtom; }

		value_type GetCoherent() const { return coh; }
		value_type GetIncoherent() const { return incoh; }
};


class ScatlenList
{
	public:
		typedef Scatlen<std::complex<double>> elem_type;
		typedef typename elem_type::value_type value_type;

	private:
		static void Init();

	protected:
		static std::vector<elem_type> s_vecElems, s_vecIsotopes;

	public:
		ScatlenList();
		virtual ~ScatlenList();

		unsigned int GetNumElems() const { return s_vecElems.size(); }
		const elem_type& GetElem(unsigned int i) const
		{ return s_vecElems[i]; }

		unsigned int GetNumIsotopes() const { return s_vecIsotopes.size(); }
		const elem_type& GetIsotope(unsigned int i) const
		{ return s_vecIsotopes[i]; }

		const elem_type* Find(const std::string& strElem) const;
};


#endif
