/**
 * create plots using the external heli model
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date dec-2016
 * @license GPLv2
 */

// clang -O2 -march=native -I../../ -I/usr/include/python2.7 -o heli heli.cpp ../monteconvo/sqwbase.cpp ../../tlibs/log/log.cpp ../monteconvo/sqw_py.cpp -std=c++11 -lstdc++ -lm -lboost_python -lpython2.7 -lboost_system -lboost_filesystem

#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>
#include "tools/monteconvo/sqw_py.h"

using t_real = double;
using t_vec = ublas::vector<t_real>;
using t_mat = ublas::matrix<t_real>;


constexpr t_real a = 4.56;
constexpr t_real kh_A = 0.036;
constexpr t_real kh_rlu = kh_A / (2.*M_PI / a);


// vars from S(q,E) model
constexpr const char* pcBdir = "g_Bdir";
constexpr const char* pcBmag = "g_B";
constexpr const char* pcTemp = "g_T";


void plotdisp(SqwPy& sqw)
{
	t_real qh = 4. / std::sqrt(2.) * kh_rlu;
	t_real hkl_0[] = {1.-qh, 1.+qh, 0.};
	t_real hkl_1[] = {1.+qh, 1.-qh, 0.};
	std::size_t iSteps = 256;

	sqw.SetVarIfAvail(pcBmag, "0.2");
	sqw.SetVarIfAvail(pcBdir, "array([1, -1, 0])");	// plus
	std::ofstream ofstrP("disp_plus_heli.dat");

	for(std::size_t iStep=0; iStep<iSteps; ++iStep)
	{
		std::cout << "Step " << (iStep+1) << " of " << iSteps << std::endl;

		t_real dFrac = t_real(iStep)/t_real(iSteps-1);
		t_real hkl[] =
		{
			tl::lerp(hkl_0[0], hkl_1[0], dFrac),
			tl::lerp(hkl_0[1], hkl_1[1], dFrac),
			tl::lerp(hkl_0[2], hkl_1[2], dFrac)
		};

		std::vector<t_real> vecE, vecW;
		std::tie(vecE, vecW) = sqw.disp(hkl[0], hkl[1], hkl[2]);

		ofstrP << std::left << std::setw(10) << hkl[0] << " "
			<< std::left << std::setw(10) << hkl[1] << " "
			<< std::left << std::setw(10) << hkl[2] << " ";

		for(std::size_t iE=0; iE<vecE.size(); ++iE)
			ofstrP << std::left << std::setw(10) << vecE[iE] << " "
				<< std::left << std::setw(10) << vecW[iE] << " ";
		ofstrP << "\n";
	}

	// ------------------------------------------------------------------------

	sqw.SetVarIfAvail(pcBmag, "-0.2");
	sqw.SetVarIfAvail(pcBdir, "array([1, -1, 0])");	// minus
	std::ofstream ofstrM("disp_minus_heli.dat");

	for(std::size_t iStep=0; iStep<iSteps; ++iStep)
	{
		std::cout << "Step " << (iStep+1) << " of " << iSteps << std::endl;

		t_real dFrac = t_real(iStep)/t_real(iSteps-1);
		t_real hkl[] =
		{
			tl::lerp(hkl_0[0], hkl_1[0], dFrac),
			tl::lerp(hkl_0[1], hkl_1[1], dFrac),
			tl::lerp(hkl_0[2], hkl_1[2], dFrac)
		};

		std::vector<t_real> vecE, vecW;
		std::tie(vecE, vecW) = sqw.disp(hkl[0], hkl[1], hkl[2]);

		ofstrM << std::left << std::setw(10) << hkl[0] << " "
			<< std::left << std::setw(10) << hkl[1] << " "
			<< std::left << std::setw(10) << hkl[2] << " ";

		for(std::size_t iE=0; iE<vecE.size(); ++iE)
			ofstrM << std::left << std::setw(10) << vecE[iE] << " "
				<< std::left << std::setw(10) << vecW[iE] << " ";
		ofstrM << "\n";
	}
}


void plotmag(SqwPy& sqw)
{
	std::ofstream ofstrP("mag_plus_heli.dat");
	std::ofstream ofstrM("mag_minus_heli.dat");
	std::ofstream ofstrPerp("mag_perp_heli.dat");
	std::ofstream ofstrPerpHori("mag_perp_hori_heli.dat");

	t_real q_kh = 2.5;
	t_real qh = q_kh / std::sqrt(2.) * kh_rlu;
	t_real hkl[] = { 1.-qh, 1.+qh, 0. };

	t_real dmag0 = 0.001;
	t_real dmag1 = 0.55;
	std::size_t iSteps = 256;

	for(std::size_t iStep=0; iStep<iSteps; ++iStep)
	{
		std::cout << "Step " << (iStep+1) << " of " << iSteps << std::endl;

		t_real dFrac = t_real(iStep)/t_real(iSteps-1);
		t_real dMag = tl::lerp(dmag0, dmag1, dFrac);

		sqw.SetVarIfAvail(pcBdir, "array([1, -1, 0])");	// plus
		sqw.SetVarIfAvail(pcBmag, tl::var_to_str(dMag));

		std::vector<t_real> vecE, vecW;
		std::tie(vecE, vecW) = sqw.disp(hkl[0], hkl[1], hkl[2]);
		ofstrP << std::left << std::setw(10) << dMag << " ";

		for(std::size_t iE=0; iE<vecE.size(); ++iE)
			ofstrP << std::left << std::setw(10) << vecE[iE] << " "
				<< std::left << std::setw(10) << vecW[iE] << " ";
		ofstrP << "\n";

		// --------------------------------------------------------------------

		sqw.SetVarIfAvail(pcBdir, "array([-1, 1, 0])");	// minus

		std::tie(vecE, vecW) = sqw.disp(hkl[0], hkl[1], hkl[2]);
		ofstrM << std::left << std::setw(10) << -dMag << " ";

		for(std::size_t iE=0; iE<vecE.size(); ++iE)
			ofstrM << std::left << std::setw(10) << vecE[iE] << " "
				<< std::left << std::setw(10) << vecW[iE] << " ";
		ofstrM << "\n";


		// --------------------------------------------------------------------

		sqw.SetVarIfAvail(pcBdir, "array([0, 0, 1])");	// perp

		std::tie(vecE, vecW) = sqw.disp(hkl[0], hkl[1], hkl[2]);
		ofstrPerp << std::left << std::setw(10) << dMag << " ";

		for(std::size_t iE=0; iE<vecE.size(); ++iE)
			ofstrPerp << std::left << std::setw(10) << vecE[iE] << " "
				<< std::left << std::setw(10) << vecW[iE] << " ";
		ofstrPerp << "\n";


		// --------------------------------------------------------------------

		sqw.SetVarIfAvail(pcBdir, "array([1, 1, 0])");	// perp, in plane

		std::tie(vecE, vecW) = sqw.disp(hkl[0], hkl[1], hkl[2]);
		ofstrPerpHori << std::left << std::setw(10) << dMag << " ";

		for(std::size_t iE=0; iE<vecE.size(); ++iE)
			ofstrPerpHori << std::left << std::setw(10) << vecE[iE] << " "
				<< std::left << std::setw(10) << vecW[iE] << " ";
		ofstrPerpHori << "\n";
	}
}


int main()
{
	SqwPy sqw("./helis.py");
	if(!sqw.IsOk()) return -1;

	sqw.SetVarIfAvail(pcTemp, "20.");
	sqw.SetVarIfAvail(pcBmag, "0.2");
	sqw.SetVarIfAvail("g_nonrecip_reinit", "1");

	plotdisp(sqw);
	plotmag(sqw);

	return 0;
}
