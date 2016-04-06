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
#include "../res/defs.h"
#include "scan.h"

namespace minuit = ROOT::Minuit2;

//using t_real_mod = tl::t_real_min;
using t_real_mod = t_real_reso;


class SqwFuncModel : public tl::MinuitMultiFuncModel<t_real_mod>
{
protected:
	std::unique_ptr<SqwBase> m_pSqw;
	std::vector<TASReso> m_vecResos;
	//TASReso m_reso;
	unsigned int m_iNumNeutrons = 1000;

	ublas::vector<t_real_mod> m_vecScanOrigin;	// hklE
	ublas::vector<t_real_mod> m_vecScanDir;		// hklE

	t_real_mod m_dScale = 1., m_dOffs = 0.;
	t_real_mod m_dScaleErr = 0.1, m_dOffsErr = 0.;

	std::vector<std::string> m_vecModelParamNames;
	std::vector<t_real_mod> m_vecModelParams;
	std::vector<t_real_mod> m_vecModelErrs;

	std::string m_strTempParamName = "T";
	std::string m_strFieldParamName = "";

	bool m_bUseR0 = false;

	// -------------------------------------------------------------------------
	// optional, for multi-fits
	std::size_t m_iCurParamSet = 0;
	const std::vector<Scan>* m_pScans = nullptr;
	// -------------------------------------------------------------------------

protected:
	void SetModelParams();

public:
	SqwFuncModel(SqwBase* pSqw, const TASReso& reso);
	SqwFuncModel(SqwBase* pSqw, const std::vector<TASReso>& vecResos);
	SqwFuncModel() = delete;
	virtual ~SqwFuncModel() = default;

	virtual bool SetParams(const std::vector<tl::t_real_min>& vecParams) override;
	virtual bool SetErrs(const std::vector<tl::t_real_min>& vecErrs);
	virtual tl::t_real_min operator()(tl::t_real_min x) const override;

	virtual SqwFuncModel* copy() const override;

	virtual const char* GetModelName() const override { return "SqwFuncModel"; }
	virtual std::vector<std::string> GetParamNames() const override;
	virtual std::vector<tl::t_real_min> GetParamValues() const override;
	virtual std::vector<tl::t_real_min> GetParamErrors() const override;

	// -------------------------------------------------------------------------
	// optional, for multi-fits
	virtual void SetParamSet(std::size_t iSet) override;
	virtual std::size_t GetParamSetCount() const override;
	virtual std::size_t GetExpLen() const override;
	virtual const t_real_mod* GetExpX() const override;
	virtual const t_real_mod* GetExpY() const override;
	virtual const t_real_mod* GetExpDY() const override;

	void SetScans(const std::vector<Scan>* pScans) { m_pScans = pScans; }
	// -------------------------------------------------------------------------


	void SetOtherParamNames(std::string strTemp, std::string strField);
	void SetOtherParams(t_real_mod dTemperature, t_real_mod dField);

	void SetReso(const TASReso& reso) { /*m_reso = reso;*/ m_vecResos = {reso}; }
	void SetResos(const std::vector<TASReso>& vecResos) { m_vecResos = vecResos; }
	void SetNumNeutrons(unsigned int iNum) { m_iNumNeutrons = iNum; }

	void SetScanOrigin(t_real_mod h, t_real_mod k, t_real_mod l, t_real_mod E)
	{ m_vecScanOrigin = tl::make_vec({h,k,l,E}); }
	void SetScanDir(t_real_mod h, t_real_mod k, t_real_mod l, t_real_mod E)
	{ m_vecScanDir = tl::make_vec({h,k,l,E}); }

	void AddModelFitParams(const std::string& strName, t_real_mod dInitValue=0., t_real_mod dErr=0.)
	{
		m_vecModelParamNames.push_back(strName);
		m_vecModelParams.push_back(dInitValue);
		m_vecModelErrs.push_back(dErr);
	}

	minuit::MnUserParameters GetMinuitParams() const;
	void SetMinuitParams(const minuit::MnUserParameters& params);
	void SetMinuitParams(const minuit::MnUserParameterState& state)
	{ SetMinuitParams(state.Parameters()); }

	bool Save(const char *pcFile, t_real_mod dXMin, t_real_mod dXMax, std::size_t) const;

	void SetUseR0(bool bR0) { m_bUseR0 = bR0; }

	SqwBase* GetSqwBase() { return m_pSqw.get(); }
};


#endif
