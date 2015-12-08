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

	std::vector<ScanPoint> vecPoints;

	std::vector<double> vecCts, vecMon;
	std::vector<double> vecCtsErr, vecMonErr;


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
	tl::log_info("Loading \"", pcFile, "\"");

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


	const std::size_t iNumPts = pInstr->GetScanCount();
	for(std::size_t iPt=0; iPt<iNumPts; ++iPt)
	{
		ScanPoint pt;

		const std::array<double, 5> sc = pInstr->GetScanHKLKiKf(iPt);

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

	return true;
}

// ----------------------------------------------------------------------------


class SqwFuncModel : public tl::MinuitFuncModel
{
protected:
	std::unique_ptr<SqwBase> m_pSqw;

public:
	SqwFuncModel(SqwBase* pSqw);
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
};

SqwFuncModel::SqwFuncModel(SqwBase* pSqw)
	: m_pSqw(pSqw)
{}

double SqwFuncModel::operator()(double x) const
{
	return 0.;
}

SqwFuncModel* SqwFuncModel::copy() const
{
	// cannot rebuild kd tree in phonon model with only a shallow copy
	return new SqwFuncModel(m_pSqw->shallow_copy());
}

bool SqwFuncModel::SetParams(const std::vector<double>& vecParams)
{
	return true;
}

std::vector<std::string> SqwFuncModel::GetParamNames() const
{
	std::vector<std::string> vecNames;
	return vecNames;
}

std::vector<double> SqwFuncModel::GetParamValues() const
{
	std::vector<double> vecVals;
	return vecVals;
}

std::vector<double> SqwFuncModel::GetParamErrors() const
{
	std::vector<double> vecErrs;
	return vecErrs;
}

// ----------------------------------------------------------------------------



int main()
{
	const char* pcFile = "/home/tweber/Messdaten/IN22-2015/data/scn-mod/MgV2O4_0188.scn";
	Scan sc;
	if(!load_file(pcFile, sc))
		return -1;

	const char* pcRes = "/home/tweber/Projekte/tastools/test/mira.taz";
	TASReso reso;
	if(!reso.LoadRes(pcRes))
		return -1;
	reso.SetLattice(sc.sample.a, sc.sample.b, sc.sample.c,
		sc.sample.alpha, sc.sample.beta, sc.sample.gamma,
		tl::make_vec({sc.plane.vec1[0], sc.plane.vec1[1], sc.plane.vec1[2]}), 
		tl::make_vec({sc.plane.vec2[0], sc.plane.vec2[1], sc.plane.vec2[2]}));

	return 0;
}
