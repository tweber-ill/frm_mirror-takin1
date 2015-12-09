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

#include <boost/version.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
namespace prop = boost::property_tree;
namespace algo = boost::algorithm;


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


using t_prop = prop::basic_ptree<std::string, std::string>;

// in later boost versions, the template arg is now the string type, not the char type
#if BOOST_VERSION >= 105700
	using t_writer = typename t_prop::key_type;
#else
	using t_writer = typename t_prop::key_type::value_type;
#endif


bool gen_formfacts()
{
	t_prop prop;

	prop.add("ffacts.source", "Form factor coefficients from Clipper");
	prop.add("ffacts.source_url", "http://www.ysbl.york.ac.uk/~cowtan/clipper/");
	prop.add("ffacts.num_atoms", tl::var_to_str(dat::numsfdata));

	for(unsigned int iFF=0; iFF<dat::numsfdata; ++iFF)
	{
		std::ostringstream ostr;
		ostr << "ffacts.atom_" << iFF;
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


	std::ofstream ofstr("res/ffacts.xml");
	if(!ofstr)
	{
		std::cerr << "Error: Cannot write \"res/ffacts.xml\"." << std::endl;
		return false;
	}

	prop::write_xml(ofstr, prop, prop::xml_writer_settings<t_writer>('\t',1));
	return true;
}

// ============================================================================

void formatnumber(std::string& str)
{
	tl::string_rm<std::string>(str, "(", ")");

	if(tl::string_rm<std::string>(str, "<i>", "</i>"))
	{	// complex number
		std::size_t iSign = str.find_last_of("+-");
		str.insert(iSign, ", ");
		str.insert(0, "(");
		str.push_back(')');
	}

	tl::trim(str);
	if(str == "---") str = "";
}

bool gen_scatlens()
{
	std::ifstream ifstr("tmp/scatlens.html");
	if(!ifstr)
	{
		std::cerr << "Error: Cannot open \"tmp/scatlens.html\"." << std::endl;
		return false;
	}

	std::string strTable;
	unsigned int iLine = 0;
	while(!ifstr.eof())
	{
		std::string strLine;
		std::getline(ifstr, strLine);
		if(iLine >= 115 && iLine <= 485)
			strTable += strLine;
		++iLine;
	}



	std::vector<std::string> vecRows;
	tl::get_tokens_seq<std::string, std::string>(strTable, "<tr>", vecRows, 0);


	prop::ptree prop;
	prop.add("scatlens.source", "Scattering lengths and cross-sections from NIST");
	prop.add("scatlens.source_url", "https://www.ncnr.nist.gov/resources/n-lengths/list.html");
	prop.add("scatlens.num_atoms", tl::var_to_str(vecRows.size()));

	unsigned int iAtom = 0;
	for(const std::string& strRow : vecRows)
	{
		if(strRow.length() == 0)
			continue;

		std::ostringstream ostr;
		ostr << "scatlens.atom_" << iAtom;
		std::string strAtom = ostr.str();


		std::vector<std::string> vecCol;
		tl::get_tokens_seq<std::string, std::string>(strRow, "<td>", vecCol, 0);

		std::string strName = vecCol[1];  tl::trim(strName);
		std::string strCoh = vecCol[3];   formatnumber(strCoh);
		std::string strIncoh = vecCol[4]; formatnumber(strIncoh);

		std::string strXsecCoh = vecCol[5]; formatnumber(strXsecCoh);
		std::string strXsecIncoh = vecCol[6]; formatnumber(strXsecIncoh);
		std::string strXsecScat = vecCol[7]; formatnumber(strXsecScat);
		std::string strXsecAbsTherm = vecCol[8]; formatnumber(strXsecAbsTherm);

		prop.add(strAtom + ".name", strName);
		prop.add(strAtom + ".coh", strCoh);
		prop.add(strAtom + ".incoh", strIncoh);

		prop.add(strAtom + ".xsec_coh", strXsecCoh);
		prop.add(strAtom + ".xsec_incoh", strXsecIncoh);
		prop.add(strAtom + ".xsec_scat", strXsecScat);
		prop.add(strAtom + ".xsec_abs", strXsecAbsTherm);

		++iAtom;
	}


	std::ofstream ofstr("res/scatlens.xml");
	if(!ofstr)
	{
		std::cerr << "Error: Cannot write \"res/scatlens.xml\"." << std::endl;
		return false;
	}

	prop::write_xml(ofstr, prop, prop::xml_writer_settings<t_writer>('\t',1));
	return true;
}


// ============================================================================


int main()
{
	std::cout << "Generating form factor coefficient table ... ";
	if(gen_formfacts())
		std::cout << "OK" << std::endl;

	std::cout << "Generating scattering length table ... ";
	if(gen_scatlens())
		std::cout << "OK" << std::endl;

	return 0;
}