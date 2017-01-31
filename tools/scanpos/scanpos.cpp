/**
 * Plot scattering plane and positions from given scan files
 * @author Tobias Weber
 * @date 2016, 30-jan-2017
 * @license GPLv2
 *
 * gcc -I../.. -I. -DNO_IOSTR -o scanpos ../../tlibs/file/loadinstr.cpp ../../tlibs/log/log.cpp ../../libs/globals.cpp scanpos.cpp -std=c++11 -lstdc++ -lm -lboost_system -lboost_filesystem -lboost_program_options
 */

#define TLIBS_USE_GLOBAL_OPS
#include "tlibs/math/linalg.h"
#include "tlibs/math/linalg_ops.h"
#include "tlibs/file/loadinstr.h"
#include "tlibs/log/log.h"
#include "tlibs/version.h"
#include "libs/globals.h"

#include <vector>
#include <boost/program_options.hpp>

namespace opts = boost::program_options;

using t_real = t_real_glob;
using t_vec = tl::ublas::vector<t_real>;
using t_mat = tl::ublas::matrix<t_real>;


// transform rlu into local <vec0, vec1> coordinates
std::pair<t_vec, t_vec> get_coord(const t_vec& vec0, const t_vec& vec1, const tl::FileInstrBase<t_real>& scan)
{
	t_vec vecHKL, vecPos;

	std::size_t iNumPos = scan.GetScanCount();
	if(iNumPos)
	{
		// only use first position
		auto pos = scan.GetScanHKLKiKf(0);
		vecHKL = tl::make_vec<t_vec>({ pos[0], pos[1], pos[2] });

		vecPos.resize(2);
		vecPos[0] = vecHKL*vec0/std::pow(tl::ublas::norm_2(vec0), t_real(2));
		vecPos[1] = vecHKL*vec1/std::pow(tl::ublas::norm_2(vec1), t_real(2));

		tl::log_info(vecHKL, " rlu  ->  ", vecPos, ".");
	}

	return std::make_pair(vecHKL, vecPos);
}


void make_plot(const std::string& strFile,
	const t_vec& vec0, const t_vec& vec1,
	const std::vector<t_vec>& vecAllHKL, const std::vector<t_vec>& vecAllPos,
	bool bFlip = 1)
{
	std::ostream *pOstr = &std::cout;
	std::ofstream ofstr;

	if(strFile != "")
	{
		ofstr.open(strFile);
		if(!ofstr)
		{
			tl::log_err("Cannot write to file \"", strFile, "\".");
			return;
		}

		pOstr = &ofstr;
	}

	if(!vecAllPos.size())
	{
		tl::log_err("No scan positions found.");
		return;
	}

	(*pOstr).precision(g_iPrec);
	(*pOstr) << "#!/usr/bin/gnuplot -p\n";
	(*pOstr) << "# Created with tlibs version " << TLIBS_VERSION << ".\n\n";

	(*pOstr) << "#set term pdf enhanced color font \"NimbusSanL-Regu,16\"\n";
	(*pOstr) << "#set output \"" << strFile << ".pdf\"\n\n";

	(*pOstr) << "col_bragg = \"#ff0000\"\n";
	(*pOstr) << "col_pos = \"#0000ff\"\n";
	(*pOstr) << "size_bragg = 2\n";
	(*pOstr) << "size_pos = 1\n\n";

	(*pOstr) << "unset key\n";
	(*pOstr) << "set size 1,1\n";
	(*pOstr) << "set xlabel \"[" << vec0[0] << ", " << vec0[1] << ", " << vec0[2] << "] (rlu)\"\n";
	(*pOstr) << "set ylabel \"[" << vec1[0] << ", " << vec1[1] << ", " << vec1[2] << "] (rlu)\"\n\n";

	(*pOstr) << "set xtics rotate 0.05\n";
	(*pOstr) << "set mxtics 2\n";
	(*pOstr) << "set ytics 0.05\n";
	(*pOstr) << "set mytics 2\n\n";


	// guess Bragg peak
	t_vec vecBraggHKL = tl::make_vec<t_vec>(
		{std::round(vecAllHKL[0][0]), std::round(vecAllHKL[0][1]), std::round(vecAllHKL[0][2])});
	t_vec vecBragg(2);
	vecBragg[0] = vecBraggHKL*vec0 / std::pow(tl::ublas::norm_2(vec0), t_real(2));
	vecBragg[1] = vecBraggHKL*vec1 / std::pow(tl::ublas::norm_2(vec1), t_real(2));


	// labels
	t_real dLabelPadX = std::abs(vecBragg[0]*0.0025);
	t_real dLabelPadY = std::abs(vecBragg[1]*0.0025);

	(*pOstr) << "set label 1"
		<< " at " << vecBragg[0]+dLabelPadX << "," << vecBragg[1]
		<< " \"(" << vecBraggHKL[0] << ", " << vecBraggHKL[1] << ", " << vecBraggHKL[2] << ")\""
		<< " \n";

	t_real xmin = vecBragg[0], xmax = vecBragg[0];
	t_real ymin = vecBragg[1], ymax = vecBragg[1];

	for(std::size_t iPos=0; iPos<vecAllPos.size(); ++iPos)
	{
		const t_vec& vecHKL = vecAllHKL[iPos];
		const t_vec& vecPos = vecAllPos[iPos];

		xmin = std::min(vecPos[0], xmin);
		ymin = std::min(vecPos[1], ymin);
		xmax = std::max(vecPos[0], xmax);
		ymax = std::max(vecPos[1], ymax);

		(*pOstr) << "set label " << (iPos+2)
			<< " at " << vecPos[0]+dLabelPadX << "," << vecPos[1]
			<< " \"(" << vecHKL[0] << ", " << vecHKL[1] << ", " << vecHKL[2] << ")\""
			<< " \n";
	}
	(*pOstr) << "\n";


	// ranges
	t_real xpad = (xmax-xmin)*0.1 + dLabelPadX*10.;
	t_real ypad = (ymax-ymin)*0.1 + dLabelPadY*10.;
	(*pOstr) << "xpad = " << xpad << "\n";
	(*pOstr) << "ypad = " << ypad << "\n";
	(*pOstr) << "set xrange [" << xmin << "-xpad" << " : " << xmax << "+xpad" << "]\n";
	if(bFlip)
		(*pOstr) << "set yrange [" << ymax << "+ypad" << " : " << ymin << "-ypad" << "]\n\n";
	else
		(*pOstr) << "set yrange [" << ymin << "-ypad" << " : " << ymax << "+ypad" << "]\n\n";


	(*pOstr) << "plot \\\n";
	(*pOstr) << "\t\"-\" u 1:2 w p pt 7 ps size_bragg lc rgb col_bragg, \\\n";
	(*pOstr) << "\t\"-\" u 1:2 w p pt 7 ps size_pos lc rgb col_pos\n";

	(*pOstr) << std::left << std::setw(g_iPrec*2) << vecBragg[0] << " " 
		<< std::left << std::setw(g_iPrec*2)<< vecBragg[1] 
		<< "\t# Bragg peak\ne\n";

	for(std::size_t iPos=0; iPos<vecAllPos.size(); ++iPos)
	{
		const t_vec& vecHKL = vecAllHKL[iPos];
		const t_vec& vecPos = vecAllPos[iPos];

		(*pOstr) << std::left << std::setw(g_iPrec*2) << vecPos[0] << " " 
			<< std::left << std::setw(g_iPrec*2) << vecPos[1]
			<< "\t# Q = (" << vecHKL[0] << ", " << vecHKL[1] << ", " << vecHKL[2] << ")\n";
	}
	(*pOstr) << "e\n";
}


int main(int argc, char** argv)
{
	// --------------------------------------------------------------------
	// get job files and program options
	std::vector<std::string> vecScans;
	std::string strOutFile;
	bool bFlip = 0;
	std::string strVec0, strVec1;
	t_vec vec0, vec1;

	// normal args
	opts::options_description args("Extracts scan positions into a figure");
	args.add(boost::shared_ptr<opts::option_description>(
		new opts::option_description("scan-file",
		opts::value<decltype(vecScans)>(&vecScans), "scan file")));
	args.add(boost::shared_ptr<opts::option_description>(
		new opts::option_description("flip-y",
		opts::bool_switch(&bFlip), "flip y axis")));
	args.add(boost::shared_ptr<opts::option_description>(
		new opts::option_description("out-file",
		opts::value<decltype(strOutFile)>(&strOutFile), "output file")));
	args.add(boost::shared_ptr<opts::option_description>(
		new opts::option_description("vec0",
		opts::value<decltype(strVec0)>(&strVec0), "first scattering plane vector")));
	args.add(boost::shared_ptr<opts::option_description>(
		new opts::option_description("vec1",
		opts::value<decltype(strVec1)>(&strVec1), "second scattering plane vector")));

	// positional args
	opts::positional_options_description args_pos;
	args_pos.add("scan-file", -1);

	opts::basic_command_line_parser<char> clparser(argc, argv);
	clparser.options(args);
	clparser.positional(args_pos);
	opts::basic_parsed_options<char> parsedopts = clparser.run();

	opts::variables_map opts_map;
	opts::store(parsedopts, opts_map);
	opts::notify(opts_map);

	if(argc <= 1)
	{
		std::ostringstream ostrHelp;
		ostrHelp << "Usage: " << argv[0] << " [options] <scan-file 1> <scan-file 2> ... <scan-file n>\n";
		ostrHelp << args;
		tl::log_info(ostrHelp.str());
		return -1;
	}

	if(vecScans.size() == 0)
	{
		tl::log_err("No scan files given.");
		return -1;
	}


	if(strVec0.size())
		vec0 = tl::str_to_var<decltype(vec0)>("[3](" + strVec0 + ")");
	if(strVec1.size())
		vec1 = tl::str_to_var<decltype(vec1)>("[3](" + strVec1 + ")");
	// --------------------------------------------------------------------


	// first scan file serves as reference
	std::unique_ptr<tl::FileInstrBase<t_real>> ptrScan(
		tl::FileInstrBase<t_real>::LoadInstr(vecScans[0].c_str()));
	if(!ptrScan)
	{
		tl::log_err("Invalid scan file: \"", vecScans[0], "\".");
		return -1;
	}


	// get scattering plane if not already given as program arg
	if(vec0.size() != 3)
	{
		auto arrVec0 = ptrScan->GetScatterPlane0();
		vec0 = tl::make_vec<t_vec>({arrVec0[0],arrVec0[1],arrVec0[2]});
	}
	if(vec1.size() != 3)
	{
		auto arrVec1 = ptrScan->GetScatterPlane1();
		vec1 = tl::make_vec<t_vec>({arrVec1[0],arrVec1[1],arrVec1[2]});
	}

	if(bFlip) vec1 = -vec1;
	tl::log_info("Scattering plane: ", vec0, ", ", vec1, ".");


	std::vector<t_vec> vecAllHKL, vecAllPos;
	vecAllHKL.reserve(argc-2);
	vecAllPos.reserve(argc-2);

	// first scan position
	auto vecPos = get_coord(vec0, vec1, *ptrScan);
	if(vecPos.second.size() != 2)
	{
		tl::log_err("Invalid scan position for file \"", vecScans[0], "\".");
		return -1;
	}
	vecAllHKL.push_back(vecPos.first);
	vecAllPos.push_back(vecPos.second);


	// load other files
	for(int iFile=1; iFile<vecScans.size(); ++iFile)
	{
		std::unique_ptr<tl::FileInstrBase<t_real>> ptrScanOther(
			tl::FileInstrBase<t_real>::LoadInstr(vecScans[iFile].c_str()));
		if(!ptrScanOther)
		{
			tl::log_err("Invalid scan file: \"", vecScans[iFile], "\".");
			continue;
		}

		// remaining scan positions
		auto vecPosOther = get_coord(vec0, vec1, *ptrScanOther);
		if(vecPosOther.second.size() != 2)
		{
			tl::log_err("Invalid scan position for file \"", vecScans[iFile], "\".");
			continue;
		}

		vecAllHKL.push_back(vecPosOther.first);
		vecAllPos.push_back(vecPosOther.second);
	}


	make_plot(strOutFile, vec0, vec1, vecAllHKL, vecAllPos, bFlip);

	return 0;
}
