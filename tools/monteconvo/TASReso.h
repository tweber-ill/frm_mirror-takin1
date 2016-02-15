/*
 * loads reso settings
 * @author tweber
 * @date jul-2015
 * @license GPLv2
 */

#ifndef __DO_RESO_H__
#define __DO_RESO_H__

#include "../res/eck.h"
#include "../res/ellipse.h"
#include "../res/mc.h"

enum class ResoAlgo
{
	CN,
	POP,
	ECK
};

enum class ResoFocus : unsigned
{
	FOC_NONE = 0,

	FOC_MONO_H = (1<<1),
	FOC_MONO_V = (1<<2),

	FOC_ANA_H = (1<<3),
	FOC_ANA_V = (1<<4)
};


class TASReso
{
protected:
	ResoAlgo m_algo = ResoAlgo::CN;
	ResoFocus m_foc = ResoFocus::FOC_NONE;

	McNeutronOpts<ublas::matrix<double>> m_opts;
	EckParams m_reso;
	CNResults m_res;

	bool m_bKiFix = 0;
	double m_dKFix = 1.4;

public:
	TASReso();
	TASReso(const TASReso& res);
	const TASReso& operator=(const TASReso& res);

	virtual ~TASReso() = default;

	bool LoadRes(const char* pcXmlFile);
	bool LoadLattice(const char* pcXmlFile);

	bool SetLattice(double a, double b, double c,
		double alpha, double beta, double gamma,
		const ublas::vector<double>& vec1, const ublas::vector<double>& vec2);
	bool SetHKLE(double h, double k, double l, double E);
	Ellipsoid4d GenerateMC(std::size_t iNum, std::vector<ublas::vector<double>>&) const;

	void SetKiFix(bool bKiFix) { m_bKiFix = bKiFix; }
	void SetKFix(double dKFix) { m_dKFix = dKFix; }

	void SetAlgo(ResoAlgo algo) { m_algo = algo; }
	void SetOptimalFocus(ResoFocus foc) { m_foc = foc; }
};

#endif
