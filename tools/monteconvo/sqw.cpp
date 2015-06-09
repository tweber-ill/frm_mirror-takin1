/*
 * monte carlo convolution tool
 * @author tweber
 * @date jun-2015
 * @copyright GPLv2
 */

// TODO

#include "sqw.h"
#include "tlibs/string/string.h"
#include "tlibs/helper/log.h"
#include <fstream>
#include <list>


bool Sqw::open(const char* pcFile)
{
	std::ifstream ifstr(pcFile);
	if(!ifstr.is_open())
		return false;

	std::list<std::vector<double>> lstPoints;
	std::size_t iCurPoint = 0;
	while(!ifstr.eof())
	{
		std::string strLine;
		std::getline(ifstr, strLine);
		tl::trim(strLine);
		
		if(strLine.length() == 0)
			continue;
		
		if(strLine[0] == '#')
		{
			strLine[0] = ' ';
			m_mapParams.insert(tl::split_first(strLine, std::string(":"), 1));
			continue;
		}
		
		std::vector<double> vecSqw;
		tl::get_tokens<double>(strLine, std::string(" \t"), vecSqw);
		if(vecSqw.size() != 5)
		{
			tl::log_err("Need h,k,l,E,S data.");
			return false;
		}
		
		lstPoints.push_back(vecSqw);
		++iCurPoint;
	}
	
	tl::log_info("Loaded ",  iCurPoint+1, " S(q,w) points.");
	m_kd.Load(lstPoints);
	tl::log_info("Generated k-d tree.");
	
	return true;
}


double Sqw::operator()(double dh, double dk, double dl, double dE) const
{
	return 0.;
}
