/**
 * Convolution fitting model
 * @author tweber
 * @date dec-2015
 * @license GPLv2
 */

#include <fstream>

#include "model.h"
#include "tlibs/log/log.h"
#include "tlibs/string/string.h"

using t_real = tl::t_real_min;


SqwFuncModel::SqwFuncModel(SqwBase* pSqw, const TASReso& reso)
	: m_pSqw(pSqw), m_reso(reso)
{}

t_real SqwFuncModel::operator()(t_real x) const
{
	TASReso reso = m_reso;
	const ublas::vector<t_real> vecScanPos = m_vecScanOrigin + x*m_vecScanDir;

	if(!reso.SetHKLE(vecScanPos[0],vecScanPos[1],vecScanPos[2],vecScanPos[3]))
	{
		std::ostringstream ostrErr;
		ostrErr << "Invalid crystal position: ("
			<< vecScanPos[0] << " " << vecScanPos[1] << " " << vecScanPos[2] 
			<< ") rlu, " << vecScanPos[3] << " meV.";
		//throw tl::Err(ostrErr.str().c_str());
		tl::log_err(ostrErr.str());
		return 0.;
	}


	std::vector<ublas::vector<t_real>> vecNeutrons;
	Ellipsoid4d elli = reso.GenerateMC(m_iNumNeutrons, vecNeutrons);

	t_real dS = 0.;
	t_real dhklE_mean[4] = {0., 0., 0., 0.};

	for(const ublas::vector<t_real>& vecHKLE : vecNeutrons)
	{
		dS += (*m_pSqw)(vecHKLE[0], vecHKLE[1], vecHKLE[2], vecHKLE[3]);

		for(int i=0; i<4; ++i)
			dhklE_mean[i] += vecHKLE[i];
	}

	dS /= t_real(m_iNumNeutrons);
	for(int i=0; i<4; ++i)
		dhklE_mean[i] /= t_real(m_iNumNeutrons);

	if(m_bUseR0)
		dS *= reso.GetResoResults().dResVol;
	if(m_bUseR0 && reso.GetResoParams().bCalcR0)
		dS *= reso.GetResoResults().dR0;

	tl::log_debug("Q = (",
		vecScanPos[0], ", ", vecScanPos[1], ", ", vecScanPos[2],
		") rlu, E = ", vecScanPos[3], " meV -> S = ", dS*m_dScale + m_dOffs);
	return dS*m_dScale + m_dOffs;
}

SqwFuncModel* SqwFuncModel::copy() const
{
	// cannot rebuild kd tree in phonon model with only a shallow copy
	SqwFuncModel* pMod = new SqwFuncModel(m_pSqw->shallow_copy(), m_reso);
	pMod->m_vecScanOrigin = this->m_vecScanOrigin;
	pMod->m_vecScanDir = this->m_vecScanDir;
	pMod->m_iNumNeutrons = this->m_iNumNeutrons;
	pMod->m_dScale = this->m_dScale;
	pMod->m_dOffs = this->m_dOffs;
	pMod->m_dScaleErr = this->m_dScaleErr;
	pMod->m_dOffsErr = this->m_dOffsErr;
	pMod->m_vecModelParamNames = this->m_vecModelParamNames;
	pMod->m_vecModelParams = this->m_vecModelParams;
	pMod->m_vecModelErrs = this->m_vecModelErrs;
	pMod->m_strTempParamName = this->m_strTempParamName;
	pMod->m_strFieldParamName = this->m_strFieldParamName;
	pMod->m_bUseR0 = this->m_bUseR0;
	return pMod;
}

void SqwFuncModel::SetOtherParamNames(std::string strTemp, std::string strField)
{
	m_strTempParamName = strTemp;
	m_strFieldParamName = strField;
}

void SqwFuncModel::SetOtherParams(t_real dTemperature, t_real dField)
{
	std::vector<SqwBase::t_var> vecVars;
	if(m_strTempParamName != "")
		vecVars.push_back(std::make_tuple(m_strTempParamName, "double", tl::var_to_str(dTemperature)));
	if(m_strFieldParamName != "")
		vecVars.push_back(std::make_tuple(m_strFieldParamName, "double", tl::var_to_str(dField)));
	m_pSqw->SetVars(vecVars);
}

void SqwFuncModel::SetModelParams()
{
	const std::size_t iNumParams = m_vecModelParams.size();
	std::vector<SqwBase::t_var> vecVars;
	vecVars.reserve(iNumParams);

	for(std::size_t iParam=0; iParam<iNumParams; ++iParam)
	{
		std::string strVal = tl::var_to_str(m_vecModelParams[iParam]);
		SqwBase::t_var var = std::make_tuple(m_vecModelParamNames[iParam], "double", strVal);
		vecVars.push_back(var);
	}

	m_pSqw->SetVars(vecVars);
}

bool SqwFuncModel::SetParams(const std::vector<t_real>& vecParams)
{
	// --------------------------------------------------------------------
	// prints changed model parameters
	std::vector<t_real> vecOldParams = {m_dScale, m_dOffs};
	vecOldParams.insert(vecOldParams.end(), m_vecModelParams.begin(), m_vecModelParams.end());
	std::vector<std::string> vecParamNames = GetParamNames();
	if(vecOldParams.size()==vecParams.size() && vecParamNames.size()==vecParams.size())
	{
		std::ostringstream ostrDebug;
		std::transform(vecParams.begin(), vecParams.end(), vecParamNames.begin(),
			std::ostream_iterator<std::string>(ostrDebug, ", "),
			[&vecOldParams, &vecParamNames](t_real dVal, const std::string& strParam) -> std::string
			{
				std::vector<std::string>::const_iterator iterParam =
					std::find(vecParamNames.begin(), vecParamNames.end(), strParam);
				if(iterParam == vecParamNames.end())
					return "";
				t_real dOldParam = vecOldParams[iterParam-vecParamNames.begin()];

				bool bChanged = !tl::float_equal(dVal, dOldParam);
				std::string strRet = strParam +
					std::string(" = ") +
					tl::var_to_str(dVal);
				if(bChanged)
					strRet += " (old: " + tl::var_to_str(dOldParam) + ")";
				return strRet;
			});
		tl::log_debug("Changed model parameters: ", ostrDebug.str());
	}
	// --------------------------------------------------------------------

	m_dScale = vecParams[0];
	m_dOffs = vecParams[1];

	for(std::size_t iParam=2; iParam<vecParams.size(); ++iParam)
		m_vecModelParams[iParam-2] = vecParams[iParam];

	//tl::log_debug("Params:");
	//for(t_real d : vecParams)
	//	tl::log_debug(d);

	SetModelParams();
	return true;
}

bool SqwFuncModel::SetErrs(const std::vector<t_real>& vecErrs)
{
	m_dScaleErr = vecErrs[0];
	m_dOffsErr = vecErrs[1];

	for(std::size_t iParam=2; iParam<vecErrs.size(); ++iParam)
		m_vecModelErrs[iParam-2] = vecErrs[iParam];

	//SetModelParams();
	return true;
}

std::vector<std::string> SqwFuncModel::GetParamNames() const
{
	std::vector<std::string> vecNames = {"scale", "offs"};

	for(const std::string& str : m_vecModelParamNames)
		vecNames.push_back(str);

	return vecNames;
}

std::vector<t_real> SqwFuncModel::GetParamValues() const
{
	std::vector<t_real> vecVals = {m_dScale, m_dOffs};

	for(t_real d : m_vecModelParams)
		vecVals.push_back(d);

	return vecVals;
}

std::vector<t_real> SqwFuncModel::GetParamErrors() const
{
	std::vector<t_real> vecErrs = {m_dScaleErr, m_dOffsErr};

	for(t_real d : m_vecModelErrs)
		vecErrs.push_back(d);

	return vecErrs;
}

void SqwFuncModel::SetMinuitParams(const minuit::MnUserParameters& state)
{
	std::vector<t_real> vecNewVals;
	std::vector<t_real> vecNewErrs;

	const std::vector<std::string> vecNames = GetParamNames();
	for(std::size_t iParam=0; iParam<vecNames.size(); ++iParam)
	{
		const std::string& strName = vecNames[iParam];

		const t_real dVal = state.Value(strName);
		const t_real dErr = state.Error(strName);

		vecNewVals.push_back(dVal);
		vecNewErrs.push_back(dErr);
	}

	SetParams(vecNewVals);
	SetErrs(vecNewErrs);
}

minuit::MnUserParameters SqwFuncModel::GetMinuitParams() const
{
	minuit::MnUserParameters params;

	params.Add("scale", m_dScale, m_dScaleErr);
	params.Add("offs", m_dOffs, m_dOffsErr);

	for(std::size_t iParam=0; iParam<m_vecModelParamNames.size(); ++iParam)
	{
		const std::string& strParam = m_vecModelParamNames[iParam];
		t_real dHint = m_vecModelParams[iParam];
		t_real dErr = m_vecModelErrs[iParam];

		params.Add(strParam, dHint, dErr);
	}

	return params;
}

bool SqwFuncModel::Save(const char *pcFile, t_real dXMin, t_real dXMax, std::size_t iNum=512) const
{
	std::ofstream ofstr(pcFile);
	if(!ofstr)
	{
		tl::log_err("Cannot open \"", pcFile, "\".");
		return false;
	}

	ofstr.precision(16);

	const std::vector<std::string> vecNames = GetParamNames();
	const std::vector<t_real> vecVals = GetParamValues();
	const std::vector<t_real> vecErrs = GetParamErrors();

	for(std::size_t iParam=0; iParam<vecNames.size(); ++iParam)
		ofstr << "# " << vecNames[iParam] << " = " 
			<< vecVals[iParam] << " +- " 
			<< vecErrs[iParam] << "\n";

	for(std::size_t i=0; i<iNum; ++i)
	{
		t_real dX = tl::lerp(dXMin, dXMax, t_real(i)/t_real(iNum-1));
		t_real dY = (*this)(dX);

		ofstr << std::left << std::setw(32) << dX << " "
			<< std::left << std::setw(32) << dY << "\n";
	}

	return true;
}
