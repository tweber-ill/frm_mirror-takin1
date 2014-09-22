/*
 * Montereso
 * @author tweber
 * @date 2012, 22-sep-2014
 */

#include "res.h"
#include "helper/log.h"
#include "helper/string.h"

#include <fstream>
#include <vector>
#include <string>

int main(int argc, char **argv)
{
	if(argc <= 1)
	{
		log_err("No input file given.");
		return -1;
	}

	std::ifstream ifstr(argv[1]);
	if(!ifstr.is_open())
	{
		log_err("Cannot open \"", argv[1], "\".");
		return -1;
	}


	std::vector<double> vecQx, vecQy, vecQz, vecE;

	while(1)
	{
		if(ifstr.eof())
			break;

		std::string strLine;
		std::getline(ifstr, strLine);
		trim(strLine);

		if(strLine.length()==0 || strLine[0]=='#')
			continue;

		double dQx=0., dQy=0., dQz=0., dE=0.;
		std::istringstream istr(strLine);
		istr >> dQx >> dQy >> dQz >> dE;

		vecQx.push_back(dQx);
		vecQy.push_back(dQy);
		vecQz.push_back(dQz);
		vecE.push_back(dE);
	}

	log_info("Number of neutrons in file: ", vecQx.size());

	Resolution res = calc_res(vecQx.size(), vecQx.data(), vecQy.data(), vecQz.data(), vecE.data());

	return 0;
}
