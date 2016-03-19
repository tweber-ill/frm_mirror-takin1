/**
 * Convolution fitting model
 * @author tweber
 * @date dec-2015
 * @license GPLv2
 */

#ifndef __CONVOFIT_MOD_H__
#define __CONVOFIT_MOD_H__

#include <memory>
#include <vector>
#include <string>

#include "tlibs/fit/minuit.h"
#include <Minuit2/FunctionMinimum.h>
#include <Minuit2/MnMigrad.h>
#include <Minuit2/MnSimplex.h>
#include <Minuit2/MnPrint.h>

#include "../monteconvo/sqw.h"
#include "../monteconvo/TASReso.h"

namespace minuit = ROOT::Minuit2;


class SqwFuncModel : public tl::MinuitFuncModel
{
protected:
	std::unique_ptr<SqwBase> m_pSqw;
	TASReso m_reso;
	unsigned int m_iNumNeutrons = 1000;

	ublas::vector<double> m_vecScanOrigin;		// hklE
	ublas::vector<double> m_vecScanDir;		// hklE

	double m_dScale = 1., m_dOffs = 0.;
	double m_dScaleErr = 0.1, m_dOffsErr = 0.;

	std::vector<std::string> m_vecModelParamNames;
	std::vector<double> m_vecModelParams;
	std::vector<double> m_vecModelErrs;

	std::string m_strTempParamName = "T";
	std::string m_strFieldParamName = "";

	bool m_bUseR0 = false;

protected:
	void SetModelParams();

public:
	SqwFuncModel(SqwBase* pSqw, const TASReso& reso);
	SqwFuncModel() = delete;
	virtual ~SqwFuncModel() = default;

	virtual bool SetParams(const std::vector<double>& vecParams) override;
	virtual bool SetErrs(const std::vector<double>& vecErrs);
	virtual double operator()(double x) const override;

	virtual SqwFuncModel* copy() const override;
	virtual std::string print(bool bFillInSyms=true) const override { return ""; }

	virtual const char* GetModelName() const override { return "SqwFuncModel"; }
	virtual std::vector<std::string> GetParamNames() const override;
	virtual std::vector<double> GetParamValues() const override;
	virtual std::vector<double> GetParamErrors() const override;

	void SetOtherParamNames(std::string strTemp, std::string strField);
	void SetOtherParams(double dTemperature, double dField);

	void SetReso(const TASReso& reso) { m_reso = reso; }
	void SetNumNeutrons(unsigned int iNum) { m_iNumNeutrons = iNum; }

	void SetScanOrigin(double h, double k, double l, double E)
	{ m_vecScanOrigin = tl::make_vec({h,k,l,E}); }
	void SetScanDir(double h, double k, double l, double E)
	{ m_vecScanDir = tl::make_vec({h,k,l,E}); }

	void AddModelFitParams(const std::string& strName, double dInitValue=0., double dErr=0.)
	{
		m_vecModelParamNames.push_back(strName);
		m_vecModelParams.push_back(dInitValue);
		m_vecModelErrs.push_back(dErr);

		SetModelParams();
	}

	minuit::MnUserParameters GetMinuitParams() const;
	void SetMinuitParams(const minuit::MnUserParameterState& state);

	bool Save(const char *pcFile, double dXMin, double dXMax, std::size_t) const;

	void SetUseR0(bool bR0) { m_bUseR0 = bR0; }
};


#endif
