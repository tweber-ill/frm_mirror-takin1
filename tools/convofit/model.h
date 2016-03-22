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

	ublas::vector<tl::t_real_min> m_vecScanOrigin;	// hklE
	ublas::vector<tl::t_real_min> m_vecScanDir;	// hklE

	tl::t_real_min m_dScale = 1., m_dOffs = 0.;
	tl::t_real_min m_dScaleErr = 0.1, m_dOffsErr = 0.;

	std::vector<std::string> m_vecModelParamNames;
	std::vector<tl::t_real_min> m_vecModelParams;
	std::vector<tl::t_real_min> m_vecModelErrs;

	std::string m_strTempParamName = "T";
	std::string m_strFieldParamName = "";

	bool m_bUseR0 = false;

protected:
	void SetModelParams();

public:
	SqwFuncModel(SqwBase* pSqw, const TASReso& reso);
	SqwFuncModel() = delete;
	virtual ~SqwFuncModel() = default;

	virtual bool SetParams(const std::vector<tl::t_real_min>& vecParams) override;
	virtual bool SetErrs(const std::vector<tl::t_real_min>& vecErrs);
	virtual tl::t_real_min operator()(tl::t_real_min x) const override;

	virtual SqwFuncModel* copy() const override;
	virtual std::string print(bool bFillInSyms=true) const override { return ""; }

	virtual const char* GetModelName() const override { return "SqwFuncModel"; }
	virtual std::vector<std::string> GetParamNames() const override;
	virtual std::vector<tl::t_real_min> GetParamValues() const override;
	virtual std::vector<tl::t_real_min> GetParamErrors() const override;

	void SetOtherParamNames(std::string strTemp, std::string strField);
	void SetOtherParams(tl::t_real_min dTemperature, tl::t_real_min dField);

	void SetReso(const TASReso& reso) { m_reso = reso; }
	void SetNumNeutrons(unsigned int iNum) { m_iNumNeutrons = iNum; }

	void SetScanOrigin(tl::t_real_min h, tl::t_real_min k, tl::t_real_min l, tl::t_real_min E)
	{ m_vecScanOrigin = tl::make_vec({h,k,l,E}); }
	void SetScanDir(tl::t_real_min h, tl::t_real_min k, tl::t_real_min l, tl::t_real_min E)
	{ m_vecScanDir = tl::make_vec({h,k,l,E}); }

	void AddModelFitParams(const std::string& strName, tl::t_real_min dInitValue=0., tl::t_real_min dErr=0.)
	{
		m_vecModelParamNames.push_back(strName);
		m_vecModelParams.push_back(dInitValue);
		m_vecModelErrs.push_back(dErr);
	}

	minuit::MnUserParameters GetMinuitParams() const;
	void SetMinuitParams(const minuit::MnUserParameters& params);
	void SetMinuitParams(const minuit::MnUserParameterState& state)
	{ SetMinuitParams(state.Parameters()); }

	bool Save(const char *pcFile, tl::t_real_min dXMin, tl::t_real_min dXMax, std::size_t) const;

	void SetUseR0(bool bR0) { m_bUseR0 = bR0; }
};


#endif
