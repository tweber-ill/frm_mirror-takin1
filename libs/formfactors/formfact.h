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
#include "tlibs/math/mag.h"
#include "libs/globals.h"

using t_real_ff = ::t_real_glob;


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
		const std::string& GetAtomIdent() const { return strAtom; }

		T GetFormfact(T G) const
		{
			return tl::formfact<T, std::vector>(G, a, b, c);
		}
};


class FormfactList
{
	public:
		typedef Formfact<t_real_ff> elem_type;
		typedef typename elem_type::value_type value_type;

	private:
		static void Init();

	protected:
		static std::vector<elem_type> s_vecAtoms, s_vecIons;
		static std::string s_strSrc, s_strSrcUrl;

	public:
		FormfactList();
		virtual ~FormfactList();

		std::size_t GetNumAtoms() const { return s_vecAtoms.size(); }
		const elem_type& GetAtom(std::size_t iFormfact) const
		{ return s_vecAtoms[iFormfact]; }

		std::size_t GetNumIons() const { return s_vecIons.size(); }
		const elem_type& GetIon(std::size_t iFormfact) const
		{ return s_vecIons[iFormfact]; }

		const elem_type* Find(const std::string& strElem) const;

		static const std::string& GetSource() { return s_strSrc; }
		static const std::string& GetSourceUrl() { return s_strSrcUrl; }
};


// ----------------------------------------------------------------------------


class MagFormfactList;

template<typename T=double>
struct MagFormfact
{
	friend class MagFormfactList;

	public:
		typedef T value_type;

	protected:
		std::string strAtom;
		T A0,a0, B0,b0, C0,c0, D0;
		T A2,a2, B2,b2, C2,c2, D2;

	public:
		const std::string& GetAtomIdent() const { return strAtom; }

		T GetFormfact(T Q, T L, T S, T J) const
		{
			return tl::mag_formfact<T>(Q, L, S, J,
				A0,a0, B0,b0, C0,c0, D0,
				A2,a2, B2,b2, C2,c2, D2);
		}
};


class MagFormfactList
{
	public:
		typedef MagFormfact<t_real_ff> elem_type;
		typedef typename elem_type::value_type value_type;

	private:
		static void Init();

	protected:
		static std::vector<elem_type> s_vecAtoms;
		static std::string s_strSrc, s_strSrcUrl;

	public:
		MagFormfactList();
		virtual ~MagFormfactList();

		std::size_t GetNumAtoms() const { return s_vecAtoms.size(); }
		const elem_type& GetAtom(std::size_t iFormfact) const
		{ return s_vecAtoms[iFormfact]; }

		const elem_type* Find(const std::string& strElem) const;

		static const std::string& GetSource() { return s_strSrc; }
		static const std::string& GetSourceUrl() { return s_strSrcUrl; }
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
		const std::string& GetAtomIdent() const { return strAtom; }

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
		typedef Scatlen<std::complex<t_real_ff>> elem_type;
		typedef typename elem_type::value_type value_type;

	private:
		static void Init();

	protected:
		static std::vector<elem_type> s_vecElems, s_vecIsotopes;
		static std::string s_strSrc, s_strSrcUrl;

	public:
		ScatlenList();
		virtual ~ScatlenList();

		std::size_t GetNumElems() const { return s_vecElems.size(); }
		const elem_type& GetElem(std::size_t i) const
		{ return s_vecElems[i]; }

		std::size_t GetNumIsotopes() const { return s_vecIsotopes.size(); }
		const elem_type& GetIsotope(std::size_t i) const
		{ return s_vecIsotopes[i]; }

		const elem_type* Find(const std::string& strElem) const;

		static const std::string& GetSource() { return s_strSrc; }
		static const std::string& GetSourceUrl() { return s_strSrcUrl; }
};


#endif
