/**
 * Convolution fitting
 * @author tweber
 * @date dec-2015
 * @license GPLv2
 */

#ifndef __CONVOFIT_SCAN_H__
#define __CONVOFIT_SCAN_H__

#include <vector>
#include <string>

#include "tlibs/math/math.h"
#include "tlibs/math/neutrons.hpp"


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


struct Filter
{
	bool bLower = 0;
	bool bUpper = 0;

	double dLower = 0.;
	double dUpper = 0.;
};


struct Scan
{
	Sample sample;
	Plane plane;
	bool bKiFixed=0;
	double dKFix = 2.662;

	std::string strTempCol = "TT";
	double dTemp = 100., dTempErr=0.;

	std::string strFieldCol = "";
	double dField = 0., dFieldErr=0.;

	std::string strCntCol = "";
	std::string strMonCol = "";
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


extern bool load_file(std::vector<std::string> vecFiles, Scan& scan, 
	bool bNormToMon=1, const Filter& filter = Filter());
extern bool load_file(const char* pcFile, Scan& scan,
	bool bNormToMon=1, const Filter& filter=Filter());
extern bool save_file(const char* pcFile, const Scan& sc);


#endif
