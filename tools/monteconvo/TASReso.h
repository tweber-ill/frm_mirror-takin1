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

enum class ResoAlgo
{
	CN,
	POP,
	ECK
};


class TASReso
{
protected:
	ResoAlgo m_algo = ResoAlgo::CN;
	McNeutronOpts m_opts;
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
};

#endif
