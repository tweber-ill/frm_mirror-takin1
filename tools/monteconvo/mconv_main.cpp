/*
 * monte carlo convolution tool
 * @author tweber
 * @date jun-2015
 * @copyright GPLv2
 */
 
#include <iostream>
#include <sstream>
#include <fstream>
#include <unordered_map>

#include "tlibs/string/string.h"
#include "tlibs/helper/log.h"
#include "sqw.h"


static inline void usage(const char* pcProg)
{
	std::ostringstream ostr;
	ostr << "Usage: " << pcProg 
		<< " <mc neutron file> <S(Q,w) file>";

	tl::log_err("No input files given.\n", ostr.str());
}


static inline int monteconvo(const char* pcNeutrons, const char* pcSqw)
{
	std::ifstream ifstrNeutr(pcNeutrons);
	if(!ifstrNeutr.is_open())
	{
		tl::log_err("Cannot open neutrons file \"", pcNeutrons, "\".");
		return -1;
	}
	
	Sqw sqw;
	if(!sqw.open(pcSqw))
	{
		tl::log_err("Cannot open Sqw file \"", pcSqw, "\".");
		return -2;
	}
	
	unsigned int iCurNeutr = 0;
	std::unordered_map<std::string, std::string> mapNeutrParams;
	double dS = 0.;
	double dhklE[4] = {0., 0., 0., 0.};

	while(!ifstrNeutr.eof())
	{
		std::string strLine;
		std::getline(ifstrNeutr, strLine);
		tl::trim(strLine);
		
		if(strLine.size() == 0)
			continue;
		
		if(strLine[0] == '#')
		{
			strLine[0] = ' ';
			mapNeutrParams.insert(tl::split_first(strLine, std::string(":"), 1));
			continue;
		}
		
		/*if(mapNeutrParams["coord_sys"] != "rlu")
		{
			tl::log_err("Need rlu coordinate system.");
			return -3;
		}*/

		std::vector<double> vecNeutr;
		tl::get_tokens<double>(strLine, std::string(" \t"), vecNeutr);
		if(vecNeutr.size() != 4)
		{
			tl::log_err("Need h,k,l,E data.");
			return -3;
		}
		
		//tl::log_info("Neutron ", iCurNeutr, ": ", vecNeutr[0], ", ", vecNeutr[1], ", ", vecNeutr[2], ", ", vecNeutr[3]);
	
		for(int i=0; i<4; ++i) dhklE[i] += vecNeutr[i];
		sqw.SetNeutronParams(&mapNeutrParams);
		dS += sqw(vecNeutr[0], vecNeutr[1], vecNeutr[2], vecNeutr[3]);
		
		++iCurNeutr;
	}

	for(int i=0; i<4; ++i) dhklE[i] /= double(iCurNeutr+1);
	
	tl::log_info("Loaded ",  iCurNeutr+1, " MC neutrons.");
	
	std::cout << "S(" 
			<<  dhklE[0] << ", " << dhklE[1] <<  ", " << dhklE[2] << ", " << dhklE[3]
			<< ") = " << dS << std::endl;
	return 0;
}


int main(int argc, char** argv)
{
	::setlocale(LC_ALL, "C");

	if(argc < 3)
	{
		usage(argv[0]);
		return -1;
	}
	
	return monteconvo(argv[1], argv[2]);
}
