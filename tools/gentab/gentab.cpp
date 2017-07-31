/**
 * Creates needed tables
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date nov-2015
 * @license GPLv2
 */

#include <iostream>
#include <sstream>
#include <set>
#include <limits>

#include "tlibs/string/string.h"
#include "tlibs/file/prop.h"
#include "tlibs/log/log.h"
#include "tlibs/math/linalg.h"
#include "libs/spacegroups/spacegroup_clp.h"

#ifndef USE_BOOST_REX
	#include <regex>
	namespace rex = ::std;
#else
	#include <boost/tr1/regex.hpp>
	namespace rex = ::boost;
#endif

#include <boost/numeric/ublas/io.hpp>
#include <boost/version.hpp>

namespace prop = boost::property_tree;
namespace algo = boost::algorithm;
namespace ublas = boost::numeric::ublas;

using t_real = double;
using t_cplx = std::complex<t_real>;
using t_mat = ublas::matrix<t_real>;


unsigned g_iPrec = std::numeric_limits<t_real>::max_digits10-1;

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

	prop.Add("ffacts.source", "Form factor coefficients extracted from Clipper.");
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
			strA += tl::var_to_str(dat::sfdata[iFF].a[i], g_iPrec) + " ";
			strB += tl::var_to_str(dat::sfdata[iFF].b[i], g_iPrec) + " ";
		}

		prop.Add(strAtom + ".name", std::string(dat::sfdata[iFF].atomname));
		prop.Add(strAtom + ".a", strA);
		prop.Add(strAtom + ".b", strB);
		prop.Add(strAtom + ".c", tl::var_to_str(dat::sfdata[iFF].c, g_iPrec));
	}


	if(!prop.Save("res/data/ffacts.xml.gz"))
	{
		tl::log_err("Cannot write \"res/data/ffacts.xml.gz\".");
		return false;
	}
	return true;
}


// ============================================================================


/**
 * periodic table of elements
 */
bool gen_elements()
{
	using t_propval = tl::Prop<std::string>::t_propval;

	tl::Prop<std::string> propIn, propOut;
	propIn.SetSeparator('/');
	propOut.SetSeparator('.');

	if(!propIn.Load("tmp/elements.xml", tl::PropType::XML))
	{
		tl::log_err("Cannot load periodic table of elements \"tmp/elements.xml\".");
		return false;
	}


	// iterate over all elements
	std::vector<t_propval> vecElems = propIn.GetFullChildNodes("/list");
	std::size_t iElem = 0;
	for(const t_propval& elem : vecElems)
	{
		if(elem.first != "atom") continue;

		try
		{
			tl::Prop<std::string> propelem(elem.second);

			std::string strName = propelem.Query<std::string>("<xmlattr>/id", "");
			if(strName == "" || strName == "Xx") continue;

			t_real dMass = t_real(0);
			int iNr = 0;
			std::string strConfig;

			// iterate over all properties
			for(auto iterVal=propelem.GetProp().begin(); iterVal!=propelem.GetProp().end(); ++iterVal)
			{
				tl::Prop<std::string> propVal(iterVal->second);
				std::string strKey = propVal.Query<std::string>("<xmlattr>/dictRef", "");
				std::string strVal = propVal.Query<std::string>("/", "");
				//std::cout << strKey << " = " << strVal << std::endl;

				if(strKey.find("atomicNumber") != std::string::npos)
					iNr = tl::str_to_var<int>(strVal);
				else if(strKey.find("exactMass") != std::string::npos)
					dMass = tl::str_to_var<t_real>(strVal);
				else if(strKey.find("electronicConfiguration") != std::string::npos)
					strConfig = strVal;
			}

			std::ostringstream ostr;
			ostr << "pte.elem_" << iElem;
			std::string strElem = ostr.str();

			propOut.Add(strElem + ".name", strName);
			propOut.Add(strElem + ".number", tl::var_to_str(iNr, g_iPrec));
			propOut.Add(strElem + ".mass", tl::var_to_str(dMass, g_iPrec));
			propOut.Add(strElem + ".orbials", strConfig);
		}
		catch(const std::exception& ex)
		{
			tl::log_err("Element ", iElem, ": ", ex.what());
		}

		++iElem;
	}


	propOut.Add("pte.num_elems", iElem);

	propOut.Add("pte.source", "Elements obtained from the "
		"<a href=http://dx.doi.org/10.1021/ci050400b>Blue Obelisk Data Repository</a>).");
	propOut.Add("pte.source_url", "https://github.com/egonw/bodr/blob/master/bodr/elements/elements.xml");

	if(!propOut.Save("res/data/elements.xml.gz"))
	{
		tl::log_err("Cannot write \"res/data/elements.xml.gz\".");
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

bool get_abundance_or_hl(const std::string& _str, t_real& dAbOrHL)
{
	bool bIsHL = (_str.find("a") != std::string::npos);
	std::string str = tl::remove_chars(_str, std::string("()a"));

	dAbOrHL = get_number(str).real();
	if(!bIsHL) dAbOrHL /= t_real(100.);
	return !bIsHL;
}


bool gen_scatlens()
{
	std::ifstream ifstr("tmp/scatlens.html");
	if(!ifstr)
	{
		tl::log_err("Cannot open \"tmp/scatlens.html\".");
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
	prop.Add("scatlens.source", "Scattering lengths and cross-sections extracted from NIST table"
		" (which itself is based on <a href=http://dx.doi.org/10.1080/10448639208218770>this paper</a>).");
	prop.Add("scatlens.source_url", "https://www.ncnr.nist.gov/resources/n-lengths/list.html");

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
		if(strName == "") continue;

		t_cplx cCoh = get_number(vecCol[3]);
		t_cplx cIncoh = get_number(vecCol[4]);
		t_real dXsecCoh = get_number(vecCol[5]).real();
		t_real dXsecIncoh = get_number(vecCol[6]).real();
		t_real dXsecScat = get_number(vecCol[7]).real();
		t_real dXsecAbsTherm = get_number(vecCol[8]).real();

		t_real dAbOrHL = t_real(0);
		bool bAb = get_abundance_or_hl(vecCol[2], dAbOrHL);

		prop.Add(strAtom + ".name", strName);
		prop.Add(strAtom + ".coh", tl::var_to_str(cCoh, g_iPrec));
		prop.Add(strAtom + ".incoh", tl::var_to_str(cIncoh, g_iPrec));

		prop.Add(strAtom + ".xsec_coh", tl::var_to_str(dXsecCoh, g_iPrec));
		prop.Add(strAtom + ".xsec_incoh", tl::var_to_str(dXsecIncoh, g_iPrec));
		prop.Add(strAtom + ".xsec_scat", tl::var_to_str(dXsecScat, g_iPrec));
		prop.Add(strAtom + ".xsec_abs", tl::var_to_str(dXsecAbsTherm, g_iPrec));

		if(bAb)
			prop.Add(strAtom + ".abund", tl::var_to_str(dAbOrHL, g_iPrec));
		else
			prop.Add(strAtom + ".hl", tl::var_to_str(dAbOrHL, g_iPrec));

		++iAtom;
	}

	prop.Add("scatlens.num_atoms", tl::var_to_str(iAtom));


	if(!prop.Save("res/data/scatlens.xml.gz"))
	{
		tl::log_err("Cannot write \"res/data/scatlens.xml.gz\".");
		return false;
	}
	return true;
}


bool gen_scatlens_npy()
{
	tl::Prop<std::string> propIn, propOut;
	propIn.SetSeparator('/');
	propOut.SetSeparator('.');

	if(!propIn.Load("tmp/scattering_lengths.json", tl::PropType::JSON))
	{
		tl::log_err("Cannot load scattering length table \"tmp/scattering_lengths.json\".");
		return false;
	}

	std::vector<std::string> vecNuclei = propIn.GetChildNodes("/");

	std::size_t iNucl = 0;
	for(const std::string& strNucl : vecNuclei)
	{
		t_cplx cCohb = propIn.Query<t_real>("/" + strNucl + "/Coh b");
		t_cplx cIncb = propIn.Query<t_real>("/" + strNucl + "/Inc b");
		t_real dAbsXs = propIn.Query<t_real>("/" + strNucl + "/Abs xs");
		t_real dCohXs = propIn.Query<t_real>("/" + strNucl + "/Coh xs");
		t_real dIncXs = propIn.Query<t_real>("/" + strNucl + "/Inc xs");
		t_real dScatXs = propIn.Query<t_real>("/" + strNucl + "/Scatt xs");
		std::string strAbund = propIn.Query<std::string>("/" + strNucl + "/conc");

		// complex?
		auto vecValsCohb = propIn.GetChildValues<t_real>("/" + strNucl + "/Coh b");
		auto vecValsIncb = propIn.GetChildValues<t_real>("/" + strNucl + "/Inc b");

		if(vecValsCohb.size() >= 2)
		{
			cCohb.real(vecValsCohb[0]);
			cCohb.imag(vecValsCohb[1]);
		}
		if(vecValsIncb.size() >= 2)
		{
			cIncb.real(vecValsIncb[0]);
			cIncb.imag(vecValsIncb[1]);
		}

		t_real dAbOrHL = t_real(0);
		bool bAb = get_abundance_or_hl(strAbund, dAbOrHL);

		std::ostringstream ostr;
		ostr << "scatlens.atom_" << iNucl;
		std::string strAtom = ostr.str();

		propOut.Add(strAtom + ".name", strNucl);
		propOut.Add(strAtom + ".coh", tl::var_to_str(cCohb, g_iPrec));
		propOut.Add(strAtom + ".incoh", tl::var_to_str(cIncb, g_iPrec));

		propOut.Add(strAtom + ".xsec_coh", tl::var_to_str(dCohXs, g_iPrec));
		propOut.Add(strAtom + ".xsec_incoh", tl::var_to_str(dIncXs, g_iPrec));
		propOut.Add(strAtom + ".xsec_scat", tl::var_to_str(dScatXs, g_iPrec));
		propOut.Add(strAtom + ".xsec_abs", tl::var_to_str(dAbsXs, g_iPrec));

		if(bAb)
			propOut.Add(strAtom + ".abund", tl::var_to_str(dAbOrHL, g_iPrec));
		else
			propOut.Add(strAtom + ".hl", tl::var_to_str(dAbOrHL, g_iPrec));

		++iNucl;
	}

	propOut.Add("scatlens.num_atoms", tl::var_to_str(vecNuclei.size()));

	propOut.Add("scatlens.source", "Scattering lengths and cross-sections extracted from NeutronPy by D. Fobes"
		" (which itself is based on <a href=http://dx.doi.org/10.1080/10448639208218770>this paper</a>).");
	propOut.Add("scatlens.source_url", "https://github.com/neutronpy/neutronpy/blob/master/neutronpy/database/scattering_lengths.json");

	if(!propOut.Save("res/data/scatlens.xml.gz"))
	{
		tl::log_err("Cannot write \"res/data/scatlens.xml.gz\".");
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
	prop.Add("sgroups.source", "Space group data extracted from Clipper.");
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

			prop.Add(strTrafo, tl::var_to_str(matTrafo, g_iPrec) + strOpts);

			++iTrafo;
		}
	}


	if(!prop.Save("res/data/sgroups.xml.gz"))
	{
		tl::log_err("Cannot write \"res/data/sgroups.xml.gz\".");
		return false;
	}

	return true;
}


// ============================================================================


bool gen_magformfacts()
{
	tl::Prop<std::string> propOut;
	propOut.SetSeparator('.');
	propOut.Add("magffacts.source", "Magnetic form factor coefficients extracted from ILL table.");
	propOut.Add("magffacts.source_url", "https://www.ill.eu/sites/ccsl/ffacts/");

	std::size_t iAtom=0;
	std::set<std::string> setAtoms;

	std::vector<std::string> vecFiles =
		{"tmp/j0_1.html", "tmp/j0_2.html",
		"tmp/j0_3.html" , "tmp/j0_4.html",

		"tmp/j2_1.html", "tmp/j2_2.html",
		"tmp/j2_3.html", "tmp/j2_4.html",};

	for(std::size_t iFile=0; iFile<vecFiles.size(); ++iFile)
	{
		const std::string& strFile = vecFiles[iFile];
		std::string strJ = iFile < 4 ? "j0" : "j2";

		// switching to j2 files
		if(iFile==4)
		{
			iAtom = 0;
			setAtoms.clear();
		}

		std::ifstream ifstr(strFile);
		if(!ifstr)
		{
			tl::log_err("Cannot open \"", strFile, "\".");
			return false;
		}

		std::string strTable;
		bool bTableStarted=0;
		while(!ifstr.eof())
		{
			std::string strLine;
			std::getline(ifstr, strLine);
			std::string strLineLower = tl::str_to_lower(strLine);

			if(bTableStarted)
			{
				strTable += strLine + "\n";
				if(strLineLower.find("</table") != std::string::npos)
				break;
			}
			else
			{
				std::size_t iPos = strLineLower.find("<table");
				if(iPos != std::string::npos)
				{
					std::string strSub = strLine.substr(iPos);
					strTable += strSub + "\n";

					bTableStarted = 1;
				}
			}
		}
		if(strTable.length() == 0)
		{
			tl::log_err("Invalid table: \"", strFile, "\".");
			return 0;
		}

		// removing attributes
		rex::basic_regex<char> rex("<([A-Za-z]*)[A-Za-z0-9\\=\\\"\\ ]*>", rex::regex::ECMAScript);
		strTable = rex::regex_replace(strTable, rex, "<$1>");

		tl::find_all_and_replace<std::string>(strTable, "<P>", "");
		tl::find_all_and_replace<std::string>(strTable, "<p>", "");
		//std::cout << strTable << std::endl;


		std::istringstream istrTab(strTable);
		tl::Prop<std::string> prop;
		prop.Load(istrTab, tl::PropType::XML);
		const auto& tab = prop.GetProp().begin()->second;

		auto iter = tab.begin(); ++iter;
		for(; iter!=tab.end(); ++iter)
		{
			auto iterElem = iter->second.begin();
			std::string strElem = iterElem++->second.data();
			tl::trim(strElem);
			if(*strElem.rbegin() == '0')
				strElem.resize(strElem.length()-1);
			else
				strElem += "+";

			if(setAtoms.find(strElem) != setAtoms.end())
			{
				tl::log_warn("Atom ", strElem, " already in set. Ignoring.");
				continue;
			}
			setAtoms.insert(strElem);

			t_real dA = tl::str_to_var<t_real>(iterElem++->second.data());
			t_real da = tl::str_to_var<t_real>(iterElem++->second.data());
			t_real dB = tl::str_to_var<t_real>(iterElem++->second.data());
			t_real db = tl::str_to_var<t_real>(iterElem++->second.data());
			t_real dC = tl::str_to_var<t_real>(iterElem++->second.data());
			t_real dc = tl::str_to_var<t_real>(iterElem++->second.data());
			t_real dD = tl::str_to_var<t_real>(iterElem->second.data());

			std::string strA = tl::var_to_str(dA, g_iPrec) + "; " +
				tl::var_to_str(dB, g_iPrec) + "; " +
				tl::var_to_str(dC, g_iPrec) + "; " +
				tl::var_to_str(dD, g_iPrec);
			std::string stra = tl::var_to_str(da, g_iPrec) + "; " +
				tl::var_to_str(db, g_iPrec) + "; " +
				tl::var_to_str(dc, g_iPrec);

			std::ostringstream ostrAtom;
			ostrAtom << "magffacts." + strJ + ".atom_" << iAtom;
			propOut.Add(ostrAtom.str() + ".name", strElem);
			propOut.Add(ostrAtom.str() + ".A", strA);
			propOut.Add(ostrAtom.str() + ".a", stra);
			++iAtom;
		}
	}

	propOut.Add("magffacts.num_atoms", tl::var_to_str(iAtom));

	if(!propOut.Save("res/data/magffacts.xml.gz"))
	{
		tl::log_err("Cannot write \"res/data/magffacts.xml.gz\".");
		return false;
	}

	return true;
}



bool gen_magformfacts_npy()
{
	tl::Prop<std::string> propIn, propOut;
	propIn.SetSeparator('/');
	propOut.SetSeparator('.');

	if(!propIn.Load("tmp/magnetic_form_factors.json", tl::PropType::JSON))
	{
		tl::log_err("Cannot load scattering length table \"tmp/magnetic_form_factors.json\".");
		return false;
	}

	std::vector<std::string> vecNuclei = propIn.GetChildNodes("/");

	std::size_t iNucl = 0;
	for(const std::string& strNucl : vecNuclei)
	{
		auto vecJ0 = propIn.GetChildValues<t_real>("/" + strNucl + "/j0");
		auto vecJ2 = propIn.GetChildValues<t_real>("/" + strNucl + "/j2");
		auto vecJ4 = propIn.GetChildValues<t_real>("/" + strNucl + "/j4");

		std::string strJ0A, strJ2A, strJ4A, strJ0a, strJ2a, strJ4a;;

		for(std::size_t iJ=0; iJ<vecJ0.size(); ++iJ)
		{
			t_real dVal = vecJ0[iJ];
			bool bEven = tl::is_even(iJ);
			if(bEven)
			{
				if(strJ0A != "") strJ0A += "; ";
				strJ0A += tl::var_to_str(dVal, g_iPrec);
			}
			else
			{
				if(strJ0a != "") strJ0a += "; ";
				strJ0a += tl::var_to_str(dVal, g_iPrec);
			}
		}
		for(std::size_t iJ=0; iJ<vecJ2.size(); ++iJ)
		{
			t_real dVal = vecJ2[iJ];
			bool bEven = tl::is_even(iJ);
			if(bEven)
			{
				if(strJ2A != "") strJ2A += "; ";
				strJ2A += tl::var_to_str(dVal, g_iPrec);
			}
			else
			{
				if(strJ2a != "") strJ2a += "; ";
				strJ2a += tl::var_to_str(dVal, g_iPrec);
			}
		}
		for(std::size_t iJ=0; iJ<vecJ4.size(); ++iJ)
		{
			t_real dVal = vecJ4[iJ];
			bool bEven = tl::is_even(iJ);
			if(bEven)
			{
				if(strJ4A != "") strJ4A += "; ";
				strJ4A += tl::var_to_str(dVal, g_iPrec);
			}
			else
			{
				if(strJ4a != "") strJ4a += "; ";
				strJ4a += tl::var_to_str(dVal, g_iPrec);
			}
		}

		std::ostringstream ostr;
		ostr << "atom_" << iNucl;
		std::string strAtom = ostr.str();

		propOut.Add("magffacts.j0." + strAtom + ".name", strNucl);
		propOut.Add("magffacts.j2." + strAtom + ".name", strNucl);
		propOut.Add("magffacts.j4." + strAtom + ".name", strNucl);

		propOut.Add("magffacts.j0." + strAtom + ".A", strJ0A);
		propOut.Add("magffacts.j0." + strAtom + ".a", strJ0a);

		propOut.Add("magffacts.j2." + strAtom + ".A", strJ2A);
		propOut.Add("magffacts.j2." + strAtom + ".a", strJ2a);

		propOut.Add("magffacts.j4." + strAtom + ".A", strJ4A);
		propOut.Add("magffacts.j4." + strAtom + ".a", strJ4a);

		++iNucl;
	}

	propOut.Add("magffacts.num_atoms", tl::var_to_str(vecNuclei.size()));

	propOut.Add("magffacts.source", "Magnetic form factor coefficients extracted from NeutronPy by D. Fobes");
	propOut.Add("magffacts.source_url", "https://github.com/neutronpy/neutronpy/blob/master/neutronpy/database/magnetic_form_factors.json");

	if(!propOut.Save("res/data/magffacts.xml.gz"))
	{
		tl::log_err("Cannot write \"res/data/magffacts.xml.gz\".");
		return false;
	}

	return true;
}


// ============================================================================


int main()
{
#ifdef NO_TERM_CMDS
	tl::Log::SetUseTermCmds(0);
#endif

	std::cout << "Generating periodic table of elements ... ";
	if(gen_elements())
		std::cout << "OK" << std::endl;

	std::cout << "Generating atomic form factor coefficient table ... ";
	if(gen_formfacts())
		std::cout << "OK" << std::endl;

	std::cout << "Generating scattering length table ... ";
	if(gen_scatlens_npy())
	{
		std::cout << "OK" << std::endl;
	}
	else
	{
		std::cout << "Generating scattering length table (alternative) ... ";
		if(gen_scatlens())
			std::cout << "OK" << std::endl;
	}

	std::cout << "Generating space group table ... ";
	if(gen_spacegroups())
		std::cout << "OK" << std::endl;

	std::cout << "Generating magnetic form factor coefficient table ... ";
	if(gen_magformfacts())
		std::cout << "OK" << std::endl;

	/*std::cout << "Generating magnetic form factor coefficient table ... ";
	if(gen_magformfacts_npy())
		std::cout << "OK" << std::endl;*/

	return 0;
}
