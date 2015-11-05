/*
 * Wrapper for clipper's form factor tables
 * @author Tobias Weber
 * @date nov-2015
 * @license GPLv2
 */

#include "formfact.h"
#include "tlibs/math/math.h"
#include "tlibs/helper/log.h"


#ifdef USE_CLP


// ----------------------------------------------------------------------------
// ugly, but can't be helped for the moment:
// directly link to the internal clipper coefficients table
// that lives in clipper/clipper/core/atomsf.cpp
namespace clipper { namespace data
{
	extern const struct SFData
	{
		const char atomname[8];
		const double a[5], c, b[5], d;	// d is always 0
	} sfdata[];

	const unsigned int numsfdata = 212;
}}
// ----------------------------------------------------------------------------



void FormfactList::Init()
{
	Formfact<double> ffact;

	for(unsigned int iSf=0; iSf<clipper::data::numsfdata; ++iSf)
	{
		// use d==0 as check
		if(!tl::float_equal(clipper::data::sfdata[iSf].d, 0.))
		{
			tl::log_err("Data corruption in form factor coefficient table!");
			break;
		}

		ffact.strAtom = clipper::data::sfdata[iSf].atomname;
		ffact.a = tl::wrapper_array<double>((double*)clipper::data::sfdata[iSf].a, 5);
		ffact.b = tl::wrapper_array<double>((double*)clipper::data::sfdata[iSf].b, 5);
		ffact.c = clipper::data::sfdata[iSf].c;

		s_vecFormfact.push_back(std::move(ffact));
	}
}

#else

void FormfactList::Init()
{
	tl::log_err("No atom form factor coefficient table available!");
}

#endif


std::vector<Formfact<double>> FormfactList::s_vecFormfact;

// -----------------------------------------------------------------------------


FormfactList::FormfactList()
{
	if(!s_vecFormfact.size())
		Init();
}

FormfactList::~FormfactList()
{}
