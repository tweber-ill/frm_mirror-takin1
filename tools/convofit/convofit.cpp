/**
 * Convolution fitting
 * @author tweber
 * @date dec-2015
 * @license GPLv2
 */

#include "tlibs/file/loadinstr.h"
#include "tlibs/helper/log.h"
#include "tlibs/math/math.h"
#include "tlibs/math/neutrons.hpp"
#include "tlibs/fit/minuit.h"

#include "../monteconvo/TASReso.h"
#include "../monteconvo/sqw.h"

#include <memory>
#include <iostream>

#include <Minuit2/FunctionMinimum.h>
#include <Minuit2/MnMigrad.h>
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

bool load_file(const char* pcFile, Scan& scan)
{
	tl::log_info("Loading \"", pcFile, "\".");

	std::shared_ptr<tl::FileInstr> pInstr(tl::FileInstr::LoadInstr(pcFile));
	if(!pInstr)
	{
		tl::log_err("Cannot load \"", pcFile, "\".");
		return false;
	}

	const std::string strCountVar = pInstr->GetCountVar();
	const std::string strMonVar = pInstr->GetMonVar();
	scan.vecCts = pInstr->GetCol(strCountVar);
	scan.vecMon = pInstr->GetCol(strMonVar);
	scan.vecCtsErr = tl::apply_fkt(scan.vecCts, std::sqrt);
	scan.vecMonErr = tl::apply_fkt(scan.vecMon, std::sqrt);

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
	
	for(unsigned int i=0; i<4; ++i)
	{
		if(!tl::float_equal(scan.vecScanDir[i], 0., 0.01))
		{
			scan.vecScanDir[i] /= scan.vecScanDir[i];
			scan.vecScanOrigin[i] = 0.;
		}
	}
	
	tl::log_info("Scan origin: (", scan.vecScanOrigin[0], " ", scan.vecScanOrigin[1], " ", scan.vecScanOrigin[2], " ", scan.vecScanOrigin[3], ")");
	tl::log_info("Scan dir: [", scan.vecScanDir[0], " ", scan.vecScanDir[1], " ", scan.vecScanDir[2], " ", scan.vecScanDir[3], "]");



	unsigned int iScIdx = 0;
	for(iScIdx=0; iScIdx<4; ++iScIdx)
		if(!tl::float_equal(scan.vecScanDir[iScIdx], 0.))
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

// ----------------------------------------------------------------------------


class SqwFuncModel : public tl::MinuitFuncModel
{
protected:
	std::unique_ptr<SqwBase> m_pSqw;
	TASReso m_reso;
	unsigned int m_iNumNeutrons = 1000;

	ublas::vector<double> m_vecScanOrigin;	// hklE
	ublas::vector<double> m_vecScanDir;		// hklE

	double m_dScale = 1.;

public:
	SqwFuncModel(SqwBase* pSqw, const TASReso& reso);
	SqwFuncModel() = delete;
	virtual ~SqwFuncModel() = default;

	virtual bool SetParams(const std::vector<double>& vecParams) override;
	virtual double operator()(double x) const override;

	virtual SqwFuncModel* copy() const override;
	virtual std::string print(bool bFillInSyms=true) const override { return ""; }

	virtual const char* GetModelName() const override { return "SqwFuncModel"; }
	virtual std::vector<std::string> GetParamNames() const override;
	virtual std::vector<double> GetParamValues() const override;
	virtual std::vector<double> GetParamErrors() const override;
	
	void SetReso(const TASReso& reso) { m_reso = reso; }
	void SetNumNeutrons(unsigned int iNum) { m_iNumNeutrons = iNum; }

	void SetScanOrigin(double h, double k, double l, double E)
	{ m_vecScanOrigin = tl::make_vec({h,k,l,E}); }
	void SetScanDir(double h, double k, double l, double E)
	{ m_vecScanDir = tl::make_vec({h,k,l,E}); }
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

	tl::log_debug("Scan position: ", vecScanPos, ", S = ", dS*m_dScale);
	return dS*m_dScale;
}

SqwFuncModel* SqwFuncModel::copy() const
{
	// cannot rebuild kd tree in phonon model with only a shallow copy
	SqwFuncModel* pMod = new SqwFuncModel(m_pSqw->shallow_copy(), m_reso);
	pMod->m_vecScanOrigin = this->m_vecScanOrigin;
	pMod->m_vecScanDir = this->m_vecScanDir;
	pMod->m_dScale = this->m_dScale;
	return pMod;
}

bool SqwFuncModel::SetParams(const std::vector<double>& vecParams)
{
	m_dScale = vecParams[0];

	return true;
}

std::vector<std::string> SqwFuncModel::GetParamNames() const
{
	std::vector<std::string> vecNames = {"scale"};


	return vecNames;
}

std::vector<double> SqwFuncModel::GetParamValues() const
{
	std::vector<double> vecVals = {m_dScale};


	return vecVals;
}

std::vector<double> SqwFuncModel::GetParamErrors() const
{
	std::vector<double> vecErrs = {m_dScale/100.};


	return vecErrs;
}

// ----------------------------------------------------------------------------



int main()
{
	const char* pcFile = "/home/tweber/Messdaten/IN22-2015/data/scn-mod/MgV2O4_0188.scn";
	const char* pcRes = "/home/tweber/Projekte/tastools/test/mira.taz";
	const char* pcSqw = "/home/tweber/Projekte/tastools/test/MgV2O4_phonons.dat";
	
	Scan sc;
	if(!load_file(pcFile, sc))
		return -1;


	TASReso reso;
	tl::log_info("Loading instrument file \"", pcRes, "\".");
	if(!reso.LoadRes(pcRes))
		return -1;
	reso.SetLattice(sc.sample.a, sc.sample.b, sc.sample.c,
		sc.sample.alpha, sc.sample.beta, sc.sample.gamma,
		tl::make_vec({sc.plane.vec1[0], sc.plane.vec1[1], sc.plane.vec1[2]}), 
		tl::make_vec({sc.plane.vec2[0], sc.plane.vec2[1], sc.plane.vec2[2]}));
	reso.SetKiFix(sc.bKiFixed);
	reso.SetKFix(sc.dKFix);
	reso.SetAlgo(ResoAlgo::POP);


	tl::log_info("Loading S(q,w) file \"", pcSqw, "\".");
	SqwPhonon *pSqw = new SqwPhonon(pcSqw);
	if(!pSqw->IsOk())
		return -1;
	SqwFuncModel mod(pSqw, reso);
	mod.SetScanOrigin(sc.vecScanOrigin[0], sc.vecScanOrigin[1], sc.vecScanOrigin[2], sc.vecScanOrigin[3]);
	mod.SetScanDir(sc.vecScanDir[0], sc.vecScanDir[1], sc.vecScanDir[2], sc.vecScanDir[3]);


	double dSigma = 1.;
	tl::Chi2Function chi2fkt(&mod, sc.vecX.size(), sc.vecX.data(), sc.vecCts.data(), sc.vecCtsErr.data());
	chi2fkt.SetSigma(dSigma);


	minuit::MnUserParameters params;
	params.Add("scale", 10000., 1000.);
	
	minuit::MnStrategy strat(0);
	minuit::MnMigrad migrad(chi2fkt, params, strat);
	minuit::FunctionMinimum mini = migrad();
	bool bValidFit = mini.IsValid() && mini.HasValidParameters();
	tl::log_info("Fit valid: ", bValidFit);
	
	std::cout << mini.UserState().Value("scale") << " +- " << mini.UserState().Error("scale") << std::endl;

	return 0;
}
