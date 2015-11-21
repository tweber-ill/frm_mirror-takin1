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

	public:
		typedef T value_type;

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
	public:
		typedef Formfact<double> elem_type;
		typedef typename elem_type::value_type value_type;

	private:
		static void Init();

	protected:
		static std::vector<elem_type> s_vecAtoms, s_vecIons;
		static std::string s_strSrc, s_strSrcUrl;

	public:
		FormfactList();
		virtual ~FormfactList();

		unsigned int GetNumAtoms() const { return s_vecAtoms.size(); }
		const elem_type& GetAtom(unsigned int iFormfact) const
		{ return s_vecAtoms[iFormfact]; }

		unsigned int GetNumIons() const { return s_vecIons.size(); }
		const elem_type& GetIon(unsigned int iFormfact) const
		{ return s_vecIons[iFormfact]; }

		const elem_type* Find(const std::string& strElem) const;

		const std::string& GetSource() const { return s_strSrc; }
		const std::string& GetSourceUrl() const { return s_strSrcUrl; }
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

		value_type xsec_coh;
		value_type xsec_incoh;
		value_type xsec_scat;
		value_type xsec_abs;

	public:
		const std::string& GetAtomName() const { return strAtom; }

		const value_type& GetCoherent() const { return coh; }
		const value_type& GetIncoherent() const { return incoh; }

		const value_type& GetXSecCoherent() const { return xsec_coh; }
		const value_type& GetXSecIncoherent() const { return xsec_incoh; }
		const value_type& GetXSecScatter() const { return xsec_scat; }
		const value_type& GetXSecAbsorption() const { return xsec_abs; }
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
		static std::string s_strSrc, s_strSrcUrl;

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

		const std::string& GetSource() const { return s_strSrc; }
		const std::string& GetSourceUrl() const { return s_strSrcUrl; }
};


#endif
