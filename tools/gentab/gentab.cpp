// clang -o gentab gentab.cpp -lstdc++ -lm -std=c++11 -lclipper-core
/**
 * Creates needed tables
 * @author tweber
 * @date nov-2015
 * @license GPLv2
 */

#include <iostream>
#include <sstream>

#include "../../tlibs/string/string.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace prop = boost::property_tree;



// ============================================================================

// ----------------------------------------------------------------------------
// ugly, but can't be helped for the moment:
// directly link to the internal clipper coefficients table
// that lives in clipper/clipper/core/atomsf.cpp
namespace clipper { namespace data
{
        extern const struct SFData
        {
                const char atomname[8];
                const double a[5], c, b[5], d;  // d is always 0
        } sfdata[];

        const unsigned int numsfdata = 212;
}}
// ----------------------------------------------------------------------------

namespace dat = clipper::data;

bool gen_formfacts()
{
	prop::ptree prop;
	prop.add("num_atoms", tl::var_to_str(dat::numsfdata));

	for(unsigned int iFF=0; iFF<dat::numsfdata; ++iFF)
	{
		std::ostringstream ostr;
		ostr << "atom_" << iFF;
		std::string strAtom = ostr.str();

		std::string strA, strB;
		for(int i=0; i<5; ++i)
		{
			strA += tl::var_to_str(dat::sfdata[iFF].a[i]) + " ";
			strB += tl::var_to_str(dat::sfdata[iFF].b[i]) + " ";
		}

		prop.add(strAtom + ".name", std::string(dat::sfdata[iFF].atomname));
		prop.add(strAtom + ".a", strA);
		prop.add(strAtom + ".b", strB);
		prop.add(strAtom + ".c", tl::var_to_str(dat::sfdata[iFF].c));
	}


	std::ofstream ofstr("ffacts.xml");
	if(!ofstr)
	{
		std::cerr << "Error: Cannot open \"ffacts.xml\"." << std::endl;
		return false;
	}
	prop::write_xml(ofstr, prop,
		prop::xml_writer_settings<typename decltype(prop)::key_type>('\t',1));

	return true;
}

// ============================================================================



int main()
{
	std::cout << "Generating form factor coefficient table ... ";
	if(gen_formfacts())
		std::cout << "OK" << std::endl;

	return 0;
}
