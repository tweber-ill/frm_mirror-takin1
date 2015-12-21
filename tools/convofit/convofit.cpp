/**
 * Convolution fitting
 * @author tweber
 * @date dec-2015
 * @license GPLv2
 */

#include "tlibs/file/loadinstr.h"
#include "tlibs/file/prop.h"
#include "tlibs/log/log.h"
#include "tlibs/math/math.h"
#include "tlibs/math/neutrons.hpp"
#include "tlibs/fit/minuit.h"

#include "../monteconvo/TASReso.h"
#include "../monteconvo/sqw.h"

#include <memory>
#include <iostream>
#include <fstream>

#include <Minuit2/FunctionMinimum.h>
#include <Minuit2/MnMigrad.h>
#include <Minuit2/MnSimplex.h>
#include <Minuit2/MnPrint.h>

namespace minuit = ROOT::Minuit2;


// ----------------------------------------------------------------------------

struct ScanPoint
{
	double h, k, l;
	tl::wavenumber ki, kf;
	tl::energy Ei, Ef, E;
};

struct Sample
{
	double a, b, c;
	double alpha, beta, gamma;
};

struct Plane
{
	double vec1[3];
	double vec2[3];
};

struct Scan
{
	Sample sample;
	Plane plane;
	bool bKiFixed=0;
	double dKFix = 2.662;
	double dTemp = 100., dTempErr=0.;

	std::vector<ScanPoint> vecPoints;

	std::vector<double> vecX;
	std::vector<double> vecCts, vecMon;
	std::vector<double> vecCtsErr, vecMonErr;

	double vecScanOrigin[4];
	double vecScanDir[4];


	ScanPoint InterpPoint(std::size_t i, std::size_t N) const
	{
		const ScanPoint& ptBegin = *vecPoints.cbegin();
		const ScanPoint& ptEnd = *vecPoints.crbegin();

		ScanPoint pt;

		pt.h = tl::lerp(ptBegin.h, ptEnd.h, double(i)/double(N-1));
		pt.k = tl::lerp(ptBegin.k, ptEnd.k, double(i)/double(N-1));
		pt.l = tl::lerp(ptBegin.l, ptEnd.l, double(i)/double(N-1));
		pt.E = tl::lerp(ptBegin.E, ptEnd.E, double(i)/double(N-1));
		pt.Ei = tl::lerp(ptBegin.Ei, ptEnd.Ei, double(i)/double(N-1));
		pt.Ef = tl::lerp(ptBegin.Ef, ptEnd.Ef, double(i)/double(N-1));
		bool bImag=0;
		pt.ki = tl::E2k(pt.Ei, bImag);
		pt.kf = tl::E2k(pt.Ef, bImag);

		return pt;
	}
};


bool save_file(const char* pcFile, const Scan& sc)
{
	std::ofstream ofstr(pcFile);
	if(!ofstr)
		return false;

	ofstr.precision(16);

	ofstr << "# scan_origin = " 
		<< sc.vecScanOrigin[0] << " "
		<< sc.vecScanOrigin[1] << " "
		<< sc.vecScanOrigin[2] << " "
		<< sc.vecScanOrigin[3] << "\n";
	ofstr << "# scan_dir = " 
		<< sc.vecScanDir[0] << " "
		<< sc.vecScanDir[1] << " "
		<< sc.vecScanDir[2] << " "
		<< sc.vecScanDir[3] << "\n";
	ofstr << "# T = " << sc.dTemp << " +- " << sc.dTempErr << "\n";

	ofstr << "#\n";

	ofstr << std::left << std::setw(21) << "# x" 
		<< std::left << std::setw(21) << "counts" 
		<< std::left << std::setw(21) << "count errors"
		<< std::left << std::setw(21) << "monitor"
		<< std::left << std::setw(21) << "monitor errors" << "\n";

	const std::size_t iNum = sc.vecX.size();
	for(std::size_t i=0; i<iNum; ++i)
	{
		ofstr << std::left << std::setw(20) << sc.vecX[i] << " "
			<< std::left << std::setw(20) << sc.vecCts[i] << " " 
			<< std::left << std::setw(20) << sc.vecCtsErr[i] << " "
			<< std::left << std::setw(20) << sc.vecMon[i] << " "
			<< std::left << std::setw(20) << sc.vecMonErr[i] << "\n";
	}

	return true;
}

bool load_file(std::vector<std::string> vecFiles, Scan& scan, bool bNormToMon=1)
{
	if(!vecFiles.size()) return 0;
	tl::log_info("Loading \"", vecFiles[0], "\".");

	std::unique_ptr<tl::FileInstr> pInstr(tl::FileInstr::LoadInstr(vecFiles[0].c_str()));
	if(!pInstr)
	{
		tl::log_err("Cannot load \"", vecFiles[0], "\".");
		return false;
	}

	for(std::size_t iFile=1; iFile<vecFiles.size(); ++iFile)
	{
		tl::log_info("Loading \"", vecFiles[iFile], "\" for merging.");
		std::unique_ptr<tl::FileInstr> pInstrM(tl::FileInstr::LoadInstr(vecFiles[iFile].c_str()));
		if(!pInstrM)
		{
			tl::log_err("Cannot load \"", vecFiles[iFile], "\".");
			continue;
		}

		pInstr->MergeWith(pInstrM.get());
	}


	const std::string strCountVar = pInstr->GetCountVar();
	const std::string strMonVar = pInstr->GetMonVar();
	scan.vecCts = pInstr->GetCol(strCountVar);
	scan.vecMon = pInstr->GetCol(strMonVar);
	std::function<double(double)> funcErr = [](double d) -> double 
	{
		//if(tl::float_equal(d, 0.))	// error 0 causes problems with minuit
		//	return d/100.;
		return std::sqrt(d);
	};
	scan.vecCtsErr = tl::apply_fkt(scan.vecCts, funcErr);
	scan.vecMonErr = tl::apply_fkt(scan.vecMon, funcErr);

	if(bNormToMon)
	{
		for(std::size_t iPos=0; iPos<scan.vecCts.size(); ++iPos)
		{
			double y = scan.vecCts[iPos];
			double dy = scan.vecCtsErr[iPos];
			double m = scan.vecMon[iPos];
			double dm  = scan.vecMonErr[iPos];

			scan.vecCts[iPos] /= m;
			scan.vecCtsErr[iPos] = std::sqrt(dy/m * dy/m  +  y*dm/(m*m) * y*dm/(m*m));
		}
	}

	const std::array<double, 3> latt = pInstr->GetSampleLattice();
	const std::array<double, 3> ang = pInstr->GetSampleAngles();

	scan.sample.a = latt[0]; scan.sample.b = latt[1]; scan.sample.c = latt[2];
	scan.sample.alpha = ang[0]; scan.sample.beta = ang[1]; scan.sample.gamma = ang[2];

	tl::log_info("Sample lattice: ", scan.sample.a, " ", scan.sample.b, " ", scan.sample.c);
	tl::log_info("Sample angles: ", tl::r2d(scan.sample.alpha), " ", tl::r2d(scan.sample.beta), " ", tl::r2d(scan.sample.gamma));


	const std::array<double, 3> vec1 = pInstr->GetScatterPlane0();
	const std::array<double, 3> vec2 = pInstr->GetScatterPlane1();
	scan.plane.vec1[0] = vec1[0]; scan.plane.vec1[1] = vec1[1]; scan.plane.vec1[2] = vec1[2];
	scan.plane.vec2[0] = vec2[0]; scan.plane.vec2[1] = vec2[1]; scan.plane.vec2[2] = vec2[2];

	tl::log_info("Scattering plane: [", vec1[0], vec1[1], vec1[2], "], "
		"[", vec2[0], vec2[1], vec2[2], "]");


	scan.bKiFixed = pInstr->IsKiFixed();
	scan.dKFix = pInstr->GetKFix();
	if(scan.bKiFixed)
		tl::log_info("ki = ", scan.dKFix);
	else
		tl::log_info("kf = ", scan.dKFix);


	const tl::FileInstr::t_vecVals& vecTemp = pInstr->GetCol("TT");
	if(vecTemp.size() == 0)
	{
		tl::log_err("Sample temperature column not found.");
		return false;
	}
	scan.dTemp = tl::mean_value(vecTemp);
	scan.dTempErr = tl::std_dev(vecTemp);
	tl::log_info("Sample temperature: ", scan.dTemp, " +- ", scan.dTempErr);


	const std::size_t iNumPts = pInstr->GetScanCount();
	for(std::size_t iPt=0; iPt<iNumPts; ++iPt)
	{
		const std::array<double, 5> sc = pInstr->GetScanHKLKiKf(iPt);

		ScanPoint pt;
		pt.h = sc[0]; pt.k = sc[1]; pt.l = sc[2];
		pt.ki = sc[3]/tl::angstrom; pt.kf = sc[4]/tl::angstrom;
		pt.Ei = tl::k2E(pt.ki); pt.Ef = tl::k2E(pt.kf);
		pt.E = pt.Ei-pt.Ef;

		tl::log_info("Point ", iPt+1, ": ", "h=", pt.h, ", k=", pt.k, ", l=", pt.l,
			", ki=", double(pt.ki*tl::angstrom), ", kf=", double(pt.kf*tl::angstrom),
			", E=", pt.E/tl::meV/*, ", Q=", pt.Q*tl::angstrom*/,
			", Cts=", scan.vecCts[iPt]/*, "+-", scan.vecCtsErr[iPt]*/,
			", Mon=", scan.vecMon[iPt]/*, "+-", scan.vecMonErr[iPt]*/);

		scan.vecPoints.emplace_back(std::move(pt));
	}



	const ScanPoint& ptBegin = *scan.vecPoints.cbegin();
	const ScanPoint& ptEnd = *scan.vecPoints.crbegin();

	scan.vecScanOrigin[0] = ptBegin.h;
	scan.vecScanOrigin[1] = ptBegin.k;
	scan.vecScanOrigin[2] = ptBegin.l;
	scan.vecScanOrigin[3] = ptBegin.E / tl::meV;

	scan.vecScanDir[0] = ptEnd.h - ptBegin.h;
	scan.vecScanDir[1] = ptEnd.k - ptBegin.k;
	scan.vecScanDir[2] = ptEnd.l - ptBegin.l;
	scan.vecScanDir[3] = (ptEnd.E - ptBegin.E) / tl::meV;

	const double dEps = 0.01;

	for(unsigned int i=0; i<4; ++i)
	{
		if(!tl::float_equal(scan.vecScanDir[i], 0., dEps))
		{
			scan.vecScanDir[i] /= scan.vecScanDir[i];
			scan.vecScanOrigin[i] = 0.;
		}
		else
		{
			scan.vecScanDir[i] = 0.;
		}
	}

	tl::log_info("Scan origin: (", scan.vecScanOrigin[0], " ", scan.vecScanOrigin[1], " ", scan.vecScanOrigin[2], " ", scan.vecScanOrigin[3], ")");
	tl::log_info("Scan dir: [", scan.vecScanDir[0], " ", scan.vecScanDir[1], " ", scan.vecScanDir[2], " ", scan.vecScanDir[3], "]");



	unsigned int iScIdx = 0;
	for(iScIdx=0; iScIdx<4; ++iScIdx)
		if(!tl::float_equal(scan.vecScanDir[iScIdx], 0., dEps))
			break;

	if(iScIdx >= 4)
	{
		tl::log_err("No scan variable found!");
		return false;
	}

	for(std::size_t iPt=0; iPt<iNumPts; ++iPt)
	{
		const ScanPoint& pt = scan.vecPoints[iPt];

		double dPos[] = { pt.h, pt.k, pt.l, pt.E/tl::meV };
		scan.vecX.push_back(dPos[iScIdx]);
		//tl::log_info("Added pos: ", *scan.vecX.rbegin());
	}


	return true;
}

bool load_file(const char* pcFile, Scan& scan, bool bNormToMon=1)
{
	std::vector<std::string> vec{pcFile};
	return load_file(vec, scan, bNormToMon);
}

// ----------------------------------------------------------------------------


class SqwFuncModel : public tl::MinuitFuncModel
{
protected:
	std::unique_ptr<SqwBase> m_pSqw;
	TASReso m_reso;
	unsigned int m_iNumNeutrons = 1000;

	ublas::vector<double> m_vecScanOrigin;	// hklE
	ublas::vector<double> m_vecScanDir;		// hklE

	double m_dScale = 1., m_dOffs = 0.;
	double m_dScaleErr = 0.1, m_dOffsErr = 0.;

	std::vector<std::string> m_vecModelParamNames;
	std::vector<double> m_vecModelParams;
	std::vector<double> m_vecModelErrs;

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

	void SetOtherParams(double dTemperature);

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
	}

	minuit::MnUserParameters GetMinuitParams() const;
	void SetMinuitParams(const minuit::MnUserParameterState& state);

	bool Save(const char *pcFile, double dXMin, double dXMax, std::size_t) const;
};


SqwFuncModel::SqwFuncModel(SqwBase* pSqw, const TASReso& reso)
	: m_pSqw(pSqw), m_reso(reso)
{}

double SqwFuncModel::operator()(double x) const
{
	TASReso reso = m_reso;
	const ublas::vector<double> vecScanPos = m_vecScanOrigin + x*m_vecScanDir;

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


	std::vector<ublas::vector<double>> vecNeutrons;
	Ellipsoid4d elli = reso.GenerateMC(m_iNumNeutrons, vecNeutrons);

	double dS = 0.;
	double dhklE_mean[4] = {0., 0., 0., 0.};

	for(const ublas::vector<double>& vecHKLE : vecNeutrons)
	{
		dS += (*m_pSqw)(vecHKLE[0], vecHKLE[1], vecHKLE[2], vecHKLE[3]);

		for(int i=0; i<4; ++i)
			dhklE_mean[i] += vecHKLE[i];
	}

	dS /= double(m_iNumNeutrons);
	for(int i=0; i<4; ++i)
		dhklE_mean[i] /= double(m_iNumNeutrons);

	tl::log_debug("Scan position: ", vecScanPos, ", S = ", dS*m_dScale + m_dOffs);
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
	return pMod;
}

void SqwFuncModel::SetOtherParams(double dTemperature)
{
	std::vector<SqwBase::t_var> vecVars;

	vecVars.push_back(std::make_tuple("T", "double", tl::var_to_str(dTemperature)));

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

bool SqwFuncModel::SetParams(const std::vector<double>& vecParams)
{
	m_dScale = vecParams[0];
	m_dOffs = vecParams[1];

	for(std::size_t iParam=2; iParam<vecParams.size(); ++iParam)
		m_vecModelParams[iParam-2] = vecParams[iParam];

	//tl::log_debug("Params:");
	//for(double d : vecParams)
	//	tl::log_debug(d);

	SetModelParams();
	return true;
}

bool SqwFuncModel::SetErrs(const std::vector<double>& vecErrs)
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

std::vector<double> SqwFuncModel::GetParamValues() const
{
	std::vector<double> vecVals = {m_dScale, m_dOffs};

	for(double d : m_vecModelParams)
		vecVals.push_back(d);

	return vecVals;
}

std::vector<double> SqwFuncModel::GetParamErrors() const
{
	std::vector<double> vecErrs = {m_dScaleErr, m_dOffsErr};

	for(double d : m_vecModelErrs)
		vecErrs.push_back(d);

	return vecErrs;
}

void SqwFuncModel::SetMinuitParams(const minuit::MnUserParameterState& state)
{
	std::vector<double> vecNewVals;
	std::vector<double> vecNewErrs;

	const std::vector<std::string> vecNames = GetParamNames();
	for(std::size_t iParam=0; iParam<vecNames.size(); ++iParam)
	{
		const std::string& strName = vecNames[iParam];

		const double dVal = state.Value(strName);
		const double dErr = state.Error(strName);

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
		double dHint = m_vecModelParams[iParam];
		double dErr = m_vecModelErrs[iParam];

		params.Add(strParam, dHint, dErr);
	}

	return params;
}

bool SqwFuncModel::Save(const char *pcFile, double dXMin, double dXMax, std::size_t iNum=512) const
{
	std::ofstream ofstr(pcFile);
	if(!ofstr)
	{
		tl::log_err("Cannot open \"", pcFile, "\".");
		return false;
	}

	ofstr.precision(16);

	const std::vector<std::string> vecNames = GetParamNames();
	const std::vector<double> vecVals = GetParamValues();
	const std::vector<double> vecErrs = GetParamErrors();

	for(std::size_t iParam=0; iParam<vecNames.size(); ++iParam)
		ofstr << "# " << vecNames[iParam] << " = " 
			<< vecVals[iParam] << " +- " 
			<< vecErrs[iParam] << "\n";

	for(std::size_t i=0; i<iNum; ++i)
	{
		double dX = tl::lerp(dXMin, dXMax, double(i)/double(iNum-1));
		double dY = (*this)(dX);

		ofstr << std::left << std::setw(20) << dX 
			<< std::left << std::setw(20) << dY << "\n";
	}

	return true;
}

// ----------------------------------------------------------------------------



int main(int argc, char** argv)
{
	if(argc <= 1)
	{
		tl::log_info("Usage:");
		tl::log_info("\t", argv[0], " <job file 1> <job file 2> ...");
		return -1;
	}

	for(int iArg=1; iArg<argc; ++iArg)
	{
		std::string strJob = argv[iArg];
		tl::log_info("Executing job file ", iArg, ": \"", strJob, "\".");

		tl::Prop<std::string> prop;
		if(!prop.Load(strJob.c_str(), tl::PropType::INFO))
		{
			tl::log_err("Cannot load job file \"", strJob, "\".");
			return -1;
		}

		std::string strScFile = prop.Query<std::string>("input/scan_file");
		std::string strResFile = prop.Query<std::string>("input/instrument_file");
		std::string strSqwMod = prop.Query<std::string>("input/sqw_model");
		std::string strSqwFile = prop.Query<std::string>("input/sqw_file");
		bool bNormToMon = prop.Query<bool>("input/norm_to_monitor", 1);

		std::vector<std::string> vecScFiles;
		tl::get_tokens<std::string, std::string>(strScFile, ";", vecScFiles);
		for(std::string& strFile : vecScFiles) tl::trim(strFile);

		unsigned iNumNeutrons = prop.Query<unsigned>("montecarlo/neutrons", 1000);

		std::string strResAlgo = prop.Query<std::string>("resolution/algorithm", "pop");
		bool bResFocMonoV = prop.Query<bool>("resolution/focus_mono_v", 0);
		bool bResFocMonoH = prop.Query<bool>("resolution/focus_mono_h", 0);
		bool bResFocAnaV = prop.Query<bool>("resolution/focus_ana_v", 0);
		bool bResFocAnaH = prop.Query<bool>("resolution/focus_ana_h", 0);

		std::string strMinimiser = prop.Query<std::string>("fitter/minimiser");
		int iStrat = prop.Query<int>("fitter/strategy", 0);
		double dSigma = prop.Query<double>("fitter/sigma", 1.);

		unsigned int iMaxFuncCalls = prop.Query<unsigned>("fitter/max_funccalls", 0);
		double dTolerance = prop.Query<double>("fitter/tolerance", 0.5);

		std::string strScOutFile = prop.Query<std::string>("output/scan_file");
		std::string strModOutFile = prop.Query<std::string>("output/model_file");
		bool bPlot = prop.Query<bool>("output/plot", 0);

		if(strScOutFile=="" || strModOutFile=="")
		{
			tl::log_err("Not output files selected.");
			return -1;
		}


		std::string strFitParams = prop.Query<std::string>("fit_parameters/params");
		std::string strFitValues = prop.Query<std::string>("fit_parameters/values");
		std::string strFitErrors = prop.Query<std::string>("fit_parameters/errors");
		std::string strFitFixed = prop.Query<std::string>("fit_parameters/fixed");

		std::vector<std::string> vecFitParams;
		tl::get_tokens<std::string, std::string>(strFitParams, " \t\n,;", vecFitParams);
		std::vector<double> vecFitValues;
		tl::get_tokens<double, std::string>(strFitValues, " \t\n,;", vecFitValues);
		std::vector<double> vecFitErrors;
		tl::get_tokens<double, std::string>(strFitErrors, " \t\n,;", vecFitErrors);
		std::vector<bool> vecFitFixed;
		tl::get_tokens<bool, std::string>(strFitFixed, " \t\n,;", vecFitFixed);

		if(vecFitParams.size() != vecFitValues.size() || 
			vecFitParams.size() != vecFitErrors.size() || 
			vecFitParams.size() != vecFitFixed.size())
		{
			tl::log_err("Fit parameter size mismatch.");
			return -1;
		}



		Scan sc;
		if(!load_file(vecScFiles, sc, bNormToMon))
			return -1;


		TASReso reso;
		tl::log_info("Loading instrument file \"", strResFile, "\".");
		if(!reso.LoadRes(strResFile.c_str()))
			return -1;
		reso.SetLattice(sc.sample.a, sc.sample.b, sc.sample.c,
			sc.sample.alpha, sc.sample.beta, sc.sample.gamma,
			tl::make_vec({sc.plane.vec1[0], sc.plane.vec1[1], sc.plane.vec1[2]}), 
			tl::make_vec({sc.plane.vec2[0], sc.plane.vec2[1], sc.plane.vec2[2]}));
		reso.SetKiFix(sc.bKiFixed);
		reso.SetKFix(sc.dKFix);

		if(strResAlgo == "pop")
			reso.SetAlgo(ResoAlgo::POP);
		else if(strResAlgo == "cn")
			reso.SetAlgo(ResoAlgo::CN);
		else if(strResAlgo == "eck")
			reso.SetAlgo(ResoAlgo::ECK);
		else
		{
			tl::log_err("Invalid resolution algorithm selected: \"", strResAlgo, "\".");
			return -1;
		}

		if(bResFocMonoV || bResFocMonoH || bResFocAnaV || bResFocAnaH)
		{
			unsigned iFoc = 0;
			if(bResFocMonoV) iFoc |= unsigned(ResoFocus::FOC_MONO_V);
			if(bResFocMonoH) iFoc |= unsigned(ResoFocus::FOC_MONO_H);
			if(bResFocAnaV) iFoc |= unsigned(ResoFocus::FOC_ANA_V);
			if(bResFocAnaH) iFoc |= unsigned(ResoFocus::FOC_ANA_H);

			reso.SetOptimalFocus(ResoFocus(iFoc));
		}


		tl::log_info("Loading S(q,w) file \"", strSqwFile, "\".");
		SqwBase *pSqw = nullptr;

		if(strSqwMod == "phonons")
			pSqw = new SqwPhonon(strSqwFile.c_str());
		else
		{
			tl::log_err("Invalid S(q,w) model selected: \"", strSqwMod, "\".");
			return -1;
		}

		if(!pSqw->IsOk())
			return -1;
		SqwFuncModel mod(pSqw, reso);
		mod.SetScanOrigin(sc.vecScanOrigin[0], sc.vecScanOrigin[1], sc.vecScanOrigin[2], sc.vecScanOrigin[3]);
		mod.SetScanDir(sc.vecScanDir[0], sc.vecScanDir[1], sc.vecScanDir[2], sc.vecScanDir[3]);
		mod.SetNumNeutrons(iNumNeutrons);
		mod.SetOtherParams(sc.dTemp);

		for(std::size_t iParam=0; iParam<vecFitParams.size(); ++iParam)
		{
			const std::string& strParam = vecFitParams[iParam];
			double dVal = vecFitValues[iParam];
			double dErr = vecFitErrors[iParam];

			// not a S(q,w) model parameter
			if(strParam=="scale" || strParam=="offs")
				continue;

			mod.AddModelFitParams(strParam, dVal, dErr);
		}

		tl::Chi2Function chi2fkt(&mod, sc.vecX.size(), sc.vecX.data(), sc.vecCts.data(), sc.vecCtsErr.data());
		chi2fkt.SetSigma(dSigma);


		minuit::MnUserParameters params = mod.GetMinuitParams();
		for(std::size_t iParam=0; iParam<vecFitParams.size(); ++iParam)
		{
			const std::string& strParam = vecFitParams[iParam];
			double dVal = vecFitValues[iParam];
			double dErr = vecFitErrors[iParam];
			bool bFix = vecFitFixed[iParam];

			params.SetValue(strParam, dVal);
			params.SetError(strParam, dErr);
			if(bFix) params.Fix(strParam);
		}


		minuit::MnStrategy strat(iStrat);
		/*strat.SetGradientStepTolerance(1.);
		strat.SetGradientTolerance(0.2);
		strat.SetHessianStepTolerance(1.);
		strat.SetHessianG2Tolerance(0.2);*/

		std::unique_ptr<minuit::MnApplication> pmini;
		if(strMinimiser == "simplex")
			pmini.reset(new minuit::MnSimplex(chi2fkt, params, strat));
		else if(strMinimiser == "migrad")
			pmini.reset(new minuit::MnMigrad(chi2fkt, params, strat));
		else
		{
			tl::log_err("Invalid minimiser selected: \"", strMinimiser, "\".");
			return -1;
		}

		minuit::FunctionMinimum mini = (*pmini)(iMaxFuncCalls, dTolerance);
		const minuit::MnUserParameterState& state = mini.UserState();
		bool bValidFit = mini.IsValid() && mini.HasValidParameters() && state.IsValid();
		mod.SetMinuitParams(state);


		std::pair<decltype(sc.vecX)::iterator, decltype(sc.vecX)::iterator> xminmax
			= std::minmax_element(sc.vecX.begin(), sc.vecX.end());
		mod.Save(strModOutFile.c_str(), *xminmax.first, *xminmax.second, 256);
		save_file(strScOutFile.c_str(), sc);


		std::ostringstream ostrMini;
		ostrMini << mini << "\n";
		tl::log_info(ostrMini.str(), "Fit valid: ", bValidFit);

		if(bPlot)
		{
			std::ostringstream ostr;
			ostr << "gnuplot -p -e \"plot \\\"" 
				<< strModOutFile.c_str() << "\\\" using 1:2 w lines lw 1.5 lt 1, \\\""
				<< strScOutFile.c_str() << "\\\" using 1:2:3 w yerrorbars ps 1 pt 7\"\n";

			std::system(ostr.str().c_str());
		}

		if(argc > 2)
			tl::log_info("================================================================================");
	}

	return 0;
}
