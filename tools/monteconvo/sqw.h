/*
 * monte carlo convolution tool
 * @author tweber
 * @date jun-2015
 * @copyright GPLv2
 */
 
#ifndef __MCONV_SQW_H__
#define __MCONV_SQW_H__

#include <string>
#include <unordered_map>
#include "tlibs/math/kd.h"


class Sqw
{
protected:
	const std::unordered_map<std::string, std::string>* m_pmapNeutr = nullptr;
	std::unordered_map<std::string, std::string> m_mapParams;
	Kd<double> m_kd;
	
public:
	Sqw() = default;
	virtual ~Sqw() = default;
	
	void SetNeutronParams(const std::unordered_map<std::string, std::string>* pmapNeutr)
	{ m_pmapNeutr = pmapNeutr; }
	
	bool open(const char* pcFile);
	double operator()(double dh, double dk, double dl, double dE) const;
};

#endif
