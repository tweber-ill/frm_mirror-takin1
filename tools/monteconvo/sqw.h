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
#include <boost/numeric/ublas/vector.hpp>
#include "tlibs/math/kd.h"


namespace ublas = boost::numeric::ublas;

class SqwBase
{
protected:
	bool m_bOk = false;

public:
	virtual double operator()(double dh, double dk, double dl, double dE) const = 0;
	bool IsOk() const { return m_bOk; }
};


// -----------------------------------------------------------------------------


// Test S(q,w): only bragg peaks
class SqwElast : public SqwBase
{
public:
	SqwElast() { SqwBase::m_bOk = true; }
	virtual double operator()(double dh, double dk, double dl, double dE) const override;
};


// -----------------------------------------------------------------------------


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


// -----------------------------------------------------------------------------


class SqwPhonon : public SqwBase
{
protected:
	static double disp(double dq, double da, double df);

protected:
	Kd<double> m_kd;
	ublas::vector<double> m_vecBragg;

	ublas::vector<double> m_vecLA;
	ublas::vector<double> m_vecTA1;
	ublas::vector<double> m_vecTA2;

	double m_dLA_amp, m_dLA_freq;
	double m_dTA1_amp, m_dTA1_freq;
	double m_dTA2_amp, m_dTA2_freq;

public:
	SqwPhonon(const ublas::vector<double>& vecBragg,
		const ublas::vector<double>& vecTA1,
		const ublas::vector<double>& vecTA2,
		double dLA_amp, double dLA_freq, double dLA_E_HWHM, double dLA_q_HWHM,
		double dTA1_amp, double dTA1_freq, double dTA1_E_HWHM, double dTA1_q_HWHM,
		double dTA2_amp, double dTA2_freq, double dTA2_E_HWHM, double dTA2_q_HWHM);

	virtual ~SqwPhonon() = default;

	virtual double operator()(double dh, double dk, double dl, double dE) const override;


	const ublas::vector<double>& GetBragg() const { return m_vecBragg; }
	const ublas::vector<double>& GetLA() const { return m_vecLA; }
	const ublas::vector<double>& GetTA1() const { return m_vecTA1; }
	const ublas::vector<double>& GetTA2() const { return m_vecTA2; }
};

#endif
