// gcc -I../../ -o scanseries scanseries.cpp ../../tlibs/log/log.cpp -std=c++11 -lboost_system -lboost_filesystem -lstdc++
/**
 * processes multiple convo fit results
 * @author tweber
 * @date dec-2015
 * @license GPLv2
 */

#include <fstream>
#include <sstream>
#include <iostream>
#include "tlibs/string/string.h"
#include "tlibs/file/file.h"
#include "tlibs/log/log.h"


using t_map = std::map<std::string, std::pair<double, double>>;

bool get_fileprops(const char* pcFile, t_map& mapProps)
{
	std::ifstream ifstr(pcFile);
	if(!ifstr)
	{
		tl::log_err("Cannot open file \"", pcFile, "\".");
		return 0;
	}

	std::string strLine;
	while(std::getline(ifstr, strLine))
	{
		tl::trim(strLine);
		if(!strLine.size() || strLine[0] != '#')
			continue;

		strLine = strLine.substr(1);
		std::pair<std::string, std::string> strKeyVal = 
			tl::split_first(strLine, std::string("="), 1);

		std::string& strKey = strKeyVal.first;
		std::string& _strVal = strKeyVal.second;

		if(!strKey.size())
			continue;

		std::pair<std::string, std::string> strValErr =
			tl::split_first(_strVal, std::string("+-"), 1, 1);

		std::string& strVal = strValErr.first;
		std::string& strErr = strValErr.second;

		//std::cout << strKey << ": " << strVal << ", " << strErr << std::endl;
		if(strVal.length())
		{
			double dVal = tl::str_to_var<double>(strVal);
			double dErr = tl::str_to_var<double>(strErr);

			mapProps.insert(t_map::value_type(strKey, {dVal, dErr}));
		}
	}

	return 1;
}


int main(int argc, char** argv)
{
	if(argc<=1)
	{
		std::cerr << "Usage, e.g.:\n";
		std::cerr << "\t" << argv[0] << " T TA2_E_HWHM TA2_amp" << std::endl;
		return -1;
	}

	std::vector<std::string> vecCols;
	for(int iArg=1; iArg<argc; ++iArg)
		vecCols.push_back(argv[iArg]);



	const char* pcOut = "out/result.dat";
	std::ofstream ofstr(pcOut);
	if(!ofstr)
	{
		tl::log_err("Cannot open \"", pcOut, "\".");
		return -1;
	}
	ofstr.precision(16);


	for(std::size_t iCol=0; iCol<vecCols.size(); ++iCol)
	{
		if(iCol==0)
			ofstr << std::left << std::setw(42) << ("# " + vecCols[iCol]);
		else
			ofstr << std::left << std::setw(42) << vecCols[iCol];
	}
	ofstr << "\n";


	for(unsigned iNr=1; 1; ++iNr)
	{
		std::ostringstream ostrScFile;
		ostrScFile << "out/sc" << iNr << ".dat";
		std::ostringstream ostrModFile;
		ostrModFile << "out/mod" << iNr << ".dat";


		if(!tl::file_exists(ostrScFile.str().c_str()) ||
			!tl::file_exists(ostrModFile.str().c_str()))
			break;

		tl::log_info("Processing dataset ", iNr, ".");
		t_map map;
		get_fileprops(ostrScFile.str().c_str(), map);
		get_fileprops(ostrModFile.str().c_str(), map);

		for(const std::string& strCol : vecCols)
		{
			t_map::const_iterator iter = map.find(strCol);
			ofstr << std::left << std::setw(20) << iter->second.first << " ";
			ofstr << std::left << std::setw(20) << iter->second.second << " ";
		}

		ofstr << "\n";
	}

	tl::log_info("Wrote \"", pcOut, "\".");
	return 0;
}
