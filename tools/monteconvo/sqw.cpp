/*
 * monte carlo convolution tool
 * @author tweber
 * @date jun-2015
 * @license GPLv2
 */

#include "sqw.h"
#include "tlibs/string/string.h"
#include "tlibs/helper/log.h"
#include "tlibs/math/linalg.h"
#include "tlibs/math/neutrons.hpp"
#include <fstream>
#include <list>


double SqwElast::operator()(double dh, double dk, double dl, double dE) const
{
	ublas::vector<double> vecCur = tl::make_vec({dh, dk, dl, dE});
	ublas::vector<double> vecPt = tl::make_vec({std::round(dh),
		std::round(dk), std::round(dl), 0.});

	if(ublas::norm_2(vecPt-vecCur) < 0.001)
		return 1.;
	return 0.;
}


//------------------------------------------------------------------------------


SqwKdTree::SqwKdTree(const char* pcFile)
{
	if(pcFile)
		m_bOk = open(pcFile);
}

bool SqwKdTree::open(const char* pcFile)
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

	tl::log_info("Loaded ",  iCurPoint, " S(q,w) points.");
	m_kd.Load(lstPoints, 4);
	tl::log_info("Generated k-d tree.");

	//std::ofstream ofstrkd("kd.dbg");
	//m_kd.GetRootNode()->print(ofstrkd);
	return true;
}


double SqwKdTree::operator()(double dh, double dk, double dl, double dE) const
{
	std::vector<double> vechklE = {dh, dk, dl, dE};
	if(!m_kd.IsPointInGrid(vechklE))
		return 0.;

	std::vector<double> vec = m_kd.GetNearestNode(vechklE);

	/*double dDist = std::sqrt(std::pow(vec[0]-vechklE[0], 2.) +
			std::pow(vec[1]-vechklE[1], 2.) +
			std::pow(vec[2]-vechklE[2], 2.) +
			std::pow(vec[3]-vechklE[3], 2.));
	tl::log_info("Distance to node: ", dDist);*/

	//tl::log_info("Nearest node: ", vec[0], ", ", vec[1], ", ", vec[2], ", ", vec[3], ", ", vec[4]);
	return vec[4];
}


//------------------------------------------------------------------------------


double SqwPhonon::disp(double dq, double da, double df)
{
	return std::abs(da*std::sin(dq*df));
}

SqwPhonon::SqwPhonon(const ublas::vector<double>& vecBragg,
	const ublas::vector<double>& vecTA1,
	const ublas::vector<double>& vecTA2,
	double dLA_amp, double dLA_freq, double dLA_E_HWHM, double dLA_q_HWHM,
	double dTA1_amp, double dTA1_freq, double dTA1_E_HWHM, double dTA1_q_HWHM,
	double dTA2_amp, double dTA2_freq, double dTA2_E_HWHM, double dTA2_q_HWHM)
		: m_vecBragg(vecBragg), m_vecLA(vecBragg),
			m_vecTA1(vecTA1), m_vecTA2(vecTA2),
			m_dLA_amp(dLA_amp), m_dLA_freq(dLA_freq),
			m_dTA1_amp(dTA1_amp), m_dTA1_freq(dTA1_freq),
			m_dTA2_amp(dTA2_amp), m_dTA2_freq(dTA2_freq)
{
	const unsigned int iNumqs = 1000;

	m_vecLA /= ublas::norm_2(m_vecLA);
	m_vecTA1 /= ublas::norm_2(m_vecTA1);
	m_vecTA2 /= ublas::norm_2(m_vecTA2);

	tl::log_info("LA: ", m_vecLA);
	tl::log_info("TA1: ", m_vecTA1);
	tl::log_info("TA2: ", m_vecTA2);

	std::list<std::vector<double>> lst;
	for(double dq=-1.; dq<1.; dq+=1./double(iNumqs))
	{
		ublas::vector<double> vecQLA = dq*m_vecLA;
		ublas::vector<double> vecQTA1 = dq*m_vecTA1;
		ublas::vector<double> vecQTA2 = dq*m_vecTA2;

		vecQLA += m_vecBragg;
		vecQTA1 += m_vecBragg;
		vecQTA2 += m_vecBragg;

		double dELA = disp(dq, m_dLA_amp, m_dLA_freq);
		double dETA1 = disp(dq, m_dTA1_amp, m_dTA1_freq);
		double dETA2 = disp(dq, m_dTA2_amp, m_dTA2_freq);

		lst.push_back(std::vector<double>({vecQLA[0], vecQLA[1], vecQLA[2], dELA, 1., dLA_E_HWHM, dLA_q_HWHM}));
		lst.push_back(std::vector<double>({vecQTA1[0], vecQTA1[1], vecQTA1[2], dETA1, 1., dTA1_E_HWHM, dTA1_q_HWHM}));
		lst.push_back(std::vector<double>({vecQTA2[0], vecQTA2[1], vecQTA2[2], dETA2, 1., dTA2_E_HWHM, dTA2_q_HWHM}));
	}

	tl::log_info("Generated ", lst.size(), " S(q,w) points.");
	m_kd.Load(lst, 3);
	tl::log_info("Generated k-d tree.");

	m_bOk = 1;
}

double SqwPhonon::operator()(double dh, double dk, double dl, double dE) const
{
	std::vector<double> vechklE = {dh, dk, dl, dE};
	if(!m_kd.IsPointInGrid(vechklE))
		return 0.;

	std::vector<double> vec = m_kd.GetNearestNode(vechklE);
	//std::cout << "query: " << dh << " " << dk << " " << dl << " " << dE << std::endl;
	//std::cout << "nearest: " << vec[0] << " " << vec[1] << " " << vec[2] << " " << vec[3] << std::endl;

	double dE0 = vec[3];
	double dS = vec[4];
	double dT = 80.;
	double dE_HWHM = vec[5];
	double dQ_HWHM = vec[6];

	double dqDist = std::sqrt(std::pow(vec[0]-vechklE[0], 2.)
		+ std::pow(vec[1]-vechklE[1], 2.)
		+ std::pow(vec[2]-vechklE[2], 2.));

	return dS * std::abs(tl::DHO_model(dE, dT, dE0, dE_HWHM, 1., 0.)) * tl::gauss_model(dqDist, 0., dQ_HWHM*tl::HWHM2SIGMA, 1., 0.);
}
