/*
 * monte carlo convolution tool
 * @author tweber
 * @date jun-2015
 * @license GPLv2
 */

#ifndef __MCONV_SQW_H__
#define __MCONV_SQW_H__

#include <string>
#include <unordered_map>
#include "tlibs/math/kd.h"

class SqwBase
{
protected:
	bool m_bOk = false;

public:
	virtual double operator()(double dh, double dk, double dl, double dE) const = 0;
	bool IsOk() const { return m_bOk; }
};


class SqwKdTree : public SqwBase
{
protected:
	std::unordered_map<std::string, std::string> m_mapParams;
	Kd<double> m_kd;

public:
	SqwKdTree(const char* pcFile = nullptr);
	virtual ~SqwKdTree() = default;

	bool open(const char* pcFile);
	virtual double operator()(double dh, double dk, double dl, double dE) const override;
};

#endif
