/*
 * Minimalistic takin command line client
 * @author tweber
 * @date apr-2016
 * @license GPLv2
 */

#include "tlibs/log/log.h"
#include "tlibs/string/string.h"
#include "libs/version.h"
#include "tools/monteconvo/TASReso.h"

//#include <unordered_map>
#include <map>

namespace ublas = boost::numeric::ublas;

std::istream& istr = std::cin;
std::ostream& ostr = std::cout;

using t_real = t_real_reso;
template<class KEY, class VAL> using t_map = std::/*unordered_*/map<KEY, VAL>;


// ----------------------------------------------------------------------------
// client function declarations
void show_help(const std::vector<std::string>& vecArgs);
void load_sample(const std::vector<std::string>& vecArgs);
void load_instr(const std::vector<std::string>& vecArgs);
void fix(const std::vector<std::string>& vecArgs);
void calc(const std::vector<std::string>& vecArgs);
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// globals
TASReso g_tas;

using t_func = void(*)(const std::vector<std::string>&);
using t_funcmap = t_map<std::string, t_func>;

t_funcmap g_funcmap =
{
	{"help", &show_help},
	{"load_sample", &load_sample},
	{"load_instr", &load_instr},
	{"fix", &fix},
	{"calc", &calc},
};
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// client functions
void show_help(const std::vector<std::string>& vecArgs)
{
	std::string strHelp = "Available client functions: ";

	for(const t_funcmap::value_type& pair : g_funcmap)
		strHelp += pair.first + ", ";

	ostr << strHelp << std::endl;
}

void load_sample(const std::vector<std::string>& vecArgs)
{
	if(vecArgs.size() < 2)
	{
		ostr << "Error: No filename given." << std::endl;
		return;
	}

	if(g_tas.LoadLattice(vecArgs[1].c_str()))
		ostr << "OK" << std::endl;
	else
		ostr << "Error: Unable to load "
			<< vecArgs[1] << "." << std::endl;
}

void load_instr(const std::vector<std::string>& vecArgs)
{
	if(vecArgs.size() < 2)
	{
		ostr << "Error: No filename given." << std::endl;
		return;
	}

	if(g_tas.LoadRes(vecArgs[1].c_str()))
		ostr << "OK" << std::endl;
	else
		ostr << "Error: Unable to load "
			<< vecArgs[1] << "." << std::endl;
}

void fix(const std::vector<std::string>& vecArgs)
{
	if(vecArgs.size() < 3)
	{
		ostr << "Error: No variable or value given." << std::endl;
		return;
	}

	if(vecArgs[1] == "ki")
		g_tas.SetKiFix(1);
	else if(vecArgs[1] == "kf")
		g_tas.SetKiFix(0);
	else
	{
		ostr << "Error: Unknown variable "
			<< vecArgs[1] << "." << std::endl;
		return;
	}

	const t_real dVal = tl::str_to_var<t_real>(vecArgs[2]);
	g_tas.SetKFix(dVal);
	ostr << "OK" << std::endl;
}

std::ostream& operator<<(std::ostream& ostr, const ublas::matrix<t_real>& m)
{
	for(std::size_t i=0; i<m.size1(); ++i)
	{
		for(std::size_t j=0; j<m.size2(); ++j)
		{
			ostr << m(i,j) << " ";
		}
		ostr << " ";
	}

	return ostr;
}

void calc(const std::vector<std::string>& vecArgs)
{
	if(vecArgs.size() < 5)
	{
		ostr << "Error: No hkl and E position given." << std::endl;
		return;
	}

	const t_real dH = tl::str_to_var<t_real>(vecArgs[1]);
	const t_real dK = tl::str_to_var<t_real>(vecArgs[2]);
	const t_real dL = tl::str_to_var<t_real>(vecArgs[3]);
	const t_real dE = tl::str_to_var<t_real>(vecArgs[4]);

	const ResoResults& res = g_tas.GetResoResults();

	if(!g_tas.SetHKLE(dH, dK, dL, dE))
	{
		ostr << "Error: At postion Q=("
			<< dH << "," << dK << "," << dL
			<< "), E=" << dE << ": " << res.strErr
			<< std::endl;
		return;
	}

	ostr << "OK" << std::endl;

	ostr << "Reso: " << res.reso << "\n";
	ostr << "R0: " << res.dR0 << "\n";
	ostr << "Vol: " << res.dResVol << "\n";
	ostr << "Q_avg: " << res.Q_avg << "\n";
	ostr << "Bragg_FWHMs: " << res.dBraggFWHMs[0] << " "
		<< res.dBraggFWHMs[1] << " " 
		<< res.dBraggFWHMs[2] << " "
		<< res.dBraggFWHMs[3] << "\n";

	ostr.flush();
}
// ----------------------------------------------------------------------------





// ----------------------------------------------------------------------------
int main()
{
	tl::log_info("This is Takin-CLI, version " TAKIN_VER
		" (built on " __DATE__ ").");
	tl::log_info("Please report bugs to tobias.weber@tum.de.");

	std::string strLine;
	while(std::getline(istr, strLine))
	{
		std::vector<std::string> vecToks;
		tl::get_tokens<std::string, std::string, decltype(vecToks)>
			(strLine, " \t", vecToks);

		for(std::string& strTok : vecToks)
			tl::trim(strTok);

		if(!vecToks.size()) continue;

		if(vecToks[0] == "exit")
			break;

		t_funcmap::const_iterator iter = g_funcmap.find(vecToks[0]);
		if(iter == g_funcmap.end())
		{
			ostr << "Error: No such function: "
				<< vecToks[0] << std::endl;
			continue;
		}

		(*iter->second)(vecToks);
	}

	return 0;
}
// ----------------------------------------------------------------------------
