// clang -o gentab gentab.cpp -lstdc++ -lm -std=c++11 -lclipper-core
/**
 * Creates needed tables
 * @author tweber
 * @date nov-2015
 * @license GPLv2
 */

#include <iostream>
#include <sstream>

#include "tlibs/string/string.h"
#include "tlibs/file/prop.h"
#include "tlibs/log/log.h"
#include "tlibs/math/linalg.h"
#include "libs/spacegroups/spacegroup_clp.h"

#include <boost/numeric/ublas/io.hpp>
#include <boost/version.hpp>

namespace prop = boost::property_tree;
namespace algo = boost::algorithm;
namespace ublas = boost::numeric::ublas;

using t_real = double;
using t_cplx = std::complex<t_real>;
using t_mat = ublas::matrix<t_real>;


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
		const t_real a[5], c, b[5], d;  // d is always 0
	} sfdata[];

	const unsigned int numsfdata = 212;
}}
// ----------------------------------------------------------------------------

namespace dat = clipper::data;


bool gen_formfacts()
{
	tl::Prop<std::string> prop;
	prop.SetSeparator('.');

	prop.Add("ffacts.source", "Form factor coefficients extracted from Clipper");
	prop.Add("ffacts.source_url", "http://www.ysbl.york.ac.uk/~cowtan/clipper/");
	prop.Add("ffacts.num_atoms", tl::var_to_str(dat::numsfdata));

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

		prop.Add(strAtom + ".name", std::string(dat::sfdata[iFF].atomname));
		prop.Add(strAtom + ".a", strA);
		prop.Add(strAtom + ".b", strB);
		prop.Add(strAtom + ".c", tl::var_to_str(dat::sfdata[iFF].c));
	}


	if(!prop.Save("res/ffacts.xml.gz"))
	{
		tl::log_err("Error: Cannot write \"res/ffacts.xml.gz\".");
		return false;
	}
	return true;
}

// ============================================================================

t_cplx get_number(std::string str)
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


	t_cplx c;
	std::istringstream istr(str);
	istr >> c;

	t_real dR = c.real(), dI = c.imag();
	tl::set_eps_0(dR); tl::set_eps_0(dI);
	c.real(dR); c.imag(dI);
	return c;
}

bool gen_scatlens()
{
	std::ifstream ifstr("tmp/scatlens.html");
	if(!ifstr)
	{
		tl::log_err("Error: Cannot open \"tmp/scatlens.html\".");
		return false;
	}

	bool bTableStarted = 0;
	std::string strTable;
	while(!ifstr.eof())
	{
		std::string strLine;
		std::getline(ifstr, strLine);

		if(!bTableStarted)
		{
			std::vector<std::string> vecHdr;
			tl::get_tokens_seq<std::string, std::string>(strLine, "<th>", vecHdr, 0);
			if(vecHdr.size() < 9)
				continue;
			bTableStarted = 1;
		}
		else
		{
			// at end of table?
			if(tl::str_to_lower(strLine).find("/table") != std::string::npos)
				break;

			strTable += strLine;
		}
	}
	ifstr.close();



	std::vector<std::string> vecRows;
	tl::get_tokens_seq<std::string, std::string>(strTable, "<tr>", vecRows, 0);


	tl::Prop<std::string> prop;
	prop.SetSeparator('.');
	prop.Add("scatlens.source", "Scattering lengths and cross-sections extracted from NIST table");
	prop.Add("scatlens.source_url", "https://www.ncnr.nist.gov/resources/n-lengths/list.html");
	prop.Add("scatlens.num_atoms", tl::var_to_str(vecRows.size()));

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
		if(vecCol.size() < 9)
		{
			tl::log_warn("Invalid number of table entries in row \"", strRow, "\".");
			continue;
		}

		std::string strName = vecCol[1];
		tl::trim(strName);

		t_cplx cCoh = get_number(vecCol[3]);
		t_cplx cIncoh = get_number(vecCol[4]);
		t_real dXsecCoh = get_number(vecCol[5]).real();
		t_real dXsecIncoh = get_number(vecCol[6]).real();
		t_real dXsecScat = get_number(vecCol[7]).real();
		t_real dXsecAbsTherm = get_number(vecCol[8]).real();

		prop.Add(strAtom + ".name", strName);
		prop.Add(strAtom + ".coh", tl::var_to_str(cCoh));
		prop.Add(strAtom + ".incoh", tl::var_to_str(cIncoh));

		prop.Add(strAtom + ".xsec_coh", tl::var_to_str(dXsecCoh));
		prop.Add(strAtom + ".xsec_incoh", tl::var_to_str(dXsecIncoh));
		prop.Add(strAtom + ".xsec_scat", tl::var_to_str(dXsecScat));
		prop.Add(strAtom + ".xsec_abs", tl::var_to_str(dXsecAbsTherm));

		++iAtom;
	}


	if(!prop.Save("res/scatlens.xml.gz"))
	{
		tl::log_err("Error: Cannot write \"res/scatlens.xml.gz\".");
		return false;
	}
	return true;
}


// ============================================================================


bool gen_spacegroups()
{
	tl::Prop<std::string> prop;
	prop.SetSeparator('.');

	const unsigned int iNumSGs = 230;
	prop.Add("sgroups.source", "Space group data extracted from Clipper");
	prop.Add("sgroups.source_url", "http://www.ysbl.york.ac.uk/~cowtan/clipper/");
	prop.Add("sgroups.num_groups", tl::var_to_str(iNumSGs));

	for(unsigned int iSG=1; iSG<=iNumSGs; ++iSG)
	{
		SpaceGroupClp sg(iSG);

		std::ostringstream ostr;
		ostr << "sgroups.group_" << (iSG-1);
		std::string strGroup = ostr.str();

		prop.Add(strGroup + ".number", tl::var_to_str(iSG));
		prop.Add(strGroup + ".name", sg.GetName());
		//prop.Add(strGroup + ".pointgroup", get_pointgroup(sg.GetName()));
		prop.Add(strGroup + ".lauegroup", sg.GetLaueGroup());
		//prop.Add(strGroup + ".crystalsys", sg.GetCrystalSystem());
		//prop.Add(strGroup + ".crystalsysname", sg.GetCrystalSystemName());


		std::vector<t_mat> vecTrafos, vecInv, vecPrim, vecCenter;
		sg.GetSymTrafos(vecTrafos);
		sg.GetInvertingSymTrafos(vecInv);
		sg.GetPrimitiveSymTrafos(vecPrim);
		sg.GetCenteringSymTrafos(vecCenter);


		prop.Add(strGroup + ".num_trafos", tl::var_to_str(vecTrafos.size()));
		unsigned int iTrafo = 0;
		for(const t_mat& matTrafo : vecTrafos)
		{
			bool bIsInv = is_mat_in_container(vecInv, matTrafo);
			bool bIsPrim = is_mat_in_container(vecPrim, matTrafo);
			bool bIsCenter = is_mat_in_container(vecCenter, matTrafo);

			std::string strOpts = "; ";
			if(bIsPrim) strOpts += "p";
			if(bIsInv) strOpts += "i";
			if(bIsCenter) strOpts += "c";

			std::ostringstream ostrTrafo;
			ostrTrafo << strGroup << ".trafo_" << iTrafo;
			std::string strTrafo = ostrTrafo.str();

			prop.Add(strTrafo, tl::var_to_str(matTrafo) + strOpts);

			++iTrafo;
		}
	}


	if(!prop.Save("res/sgroups.xml.gz"))
	{
		tl::log_err("Error: Cannot write \"res/sgroups.xml.gz\".");
		return false;
	}

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

	std::cout << "Generating space group table ... ";
	if(gen_spacegroups())
		std::cout << "OK" << std::endl;

	return 0;
}
