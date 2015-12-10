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
#include <list>
#include <tuple>
#include <memory>
#include <boost/numeric/ublas/vector.hpp>
#include "tlibs/math/kd.h"

namespace ublas = boost::numeric::ublas;


class SqwBase
{
public:
	// name, type, value
	using t_var = std::tuple<std::string, std::string, std::string>;

protected:
	bool m_bOk = false;

public:
	virtual double operator()(double dh, double dk, double dl, double dE) const = 0;
	bool IsOk() const { return m_bOk; }

	virtual std::vector<t_var> GetVars() const = 0;
	virtual void SetVars(const std::vector<t_var>&) = 0;
	
	virtual SqwBase* shallow_copy() const = 0;

	virtual ~SqwBase() {}
};


// -----------------------------------------------------------------------------

struct ElastPeak
{
	double h, k, l;
	double dSigQ, dSigE;
	double dS;
};

// Test S(q,w): only bragg peaks
class SqwElast : public SqwBase
{
protected:
	bool m_bLoadedFromFile = false;
	std::list<ElastPeak> m_lstPeaks;

public:
	SqwElast() { SqwBase::m_bOk = true; }
	SqwElast(const char* pcFile);
	virtual double operator()(double dh, double dk, double dl, double dE) const override;

	void AddPeak(double h, double k, double l, double dSigQ, double dSigE, double dS);

	virtual std::vector<SqwBase::t_var> GetVars() const override;
	virtual void SetVars(const std::vector<SqwBase::t_var>&) override;
	
	// TODO
	virtual SqwBase* shallow_copy() const { return nullptr; }
};


// -----------------------------------------------------------------------------


class SqwKdTree : public SqwBase
{
protected:
	std::unordered_map<std::string, std::string> m_mapParams;
	tl::Kd<double> m_kd;

public:
	SqwKdTree(const char* pcFile = nullptr);
	virtual ~SqwKdTree() = default;

	bool open(const char* pcFile);
	virtual double operator()(double dh, double dk, double dl, double dE) const override;

	virtual std::vector<SqwBase::t_var> GetVars() const override;
	virtual void SetVars(const std::vector<SqwBase::t_var>&) override;

	// TODO
	virtual SqwBase* shallow_copy() const { return nullptr; }
};


// -----------------------------------------------------------------------------


class SqwPhonon : public SqwBase
{
private:
	SqwPhonon() {};
	
protected:
	static double disp(double dq, double da, double df);

	void create();
	void destroy();

protected:
	std::shared_ptr<tl::Kd<double>> m_kd;
	unsigned int m_iNumqs = 250;
	unsigned int m_iNumArc = 50;
	double m_dArcMax = 10.;

	ublas::vector<double> m_vecBragg;

	ublas::vector<double> m_vecLA;
	ublas::vector<double> m_vecTA1;
	ublas::vector<double> m_vecTA2;

	double m_dLA_amp, m_dLA_freq, m_dLA_E_HWHM, m_dLA_q_HWHM;
	double m_dTA1_amp, m_dTA1_freq, m_dTA1_E_HWHM, m_dTA1_q_HWHM;
	double m_dTA2_amp, m_dTA2_freq, m_dTA2_E_HWHM, m_dTA2_q_HWHM;

	double m_dT = 100.;

public:
	SqwPhonon(const ublas::vector<double>& vecBragg,
		const ublas::vector<double>& vecTA1,
		const ublas::vector<double>& vecTA2,
		double dLA_amp, double dLA_freq, double dLA_E_HWHM, double dLA_q_HWHM,
		double dTA1_amp, double dTA1_freq, double dTA1_E_HWHM, double dTA1_q_HWHM,
		double dTA2_amp, double dTA2_freq, double dTA2_E_HWHM, double dTA2_q_HWHM,
		double dT);
	SqwPhonon(const char* pcFile);

	virtual ~SqwPhonon() = default;

	virtual double operator()(double dh, double dk, double dl, double dE) const override;


	const ublas::vector<double>& GetBragg() const { return m_vecBragg; }
	const ublas::vector<double>& GetLA() const { return m_vecLA; }
	const ublas::vector<double>& GetTA1() const { return m_vecTA1; }
	const ublas::vector<double>& GetTA2() const { return m_vecTA2; }

	virtual std::vector<SqwBase::t_var> GetVars() const override;
	virtual void SetVars(const std::vector<SqwBase::t_var>&) override;

	virtual SqwBase* shallow_copy() const override;
};

#endif
