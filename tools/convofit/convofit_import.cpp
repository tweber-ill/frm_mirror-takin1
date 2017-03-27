/**
 * Convert monteconvo format to convofit
 * @author Tobias Weber
 * @date mar-2017
 * @license GPLv2
 */

#include "convofit_import.h"
#include "tlibs/file/tmp.h"
#include "../res/defs.h"
using t_real = t_real_reso;

#include <map>


/**
 * converts from monteconvo input file format
 */
std::string convert_monteconvo(
	const tl::Prop<std::string>& propMC, std::string strFile)
{
	// inputs
	std::map<std::string, std::string> mapJob;
	mapJob["input/scan_file"] = propMC.Query<std::string>("taz/monteconvo/scanfile");
	mapJob["input/instrument_file"] = propMC.Query<std::string>("taz/monteconvo/instr");
	mapJob["input/sqw_model"] = propMC.Query<std::string>("taz/monteconvo/sqw");
	mapJob["input/sqw_file"] = propMC.Query<std::string>("taz/monteconvo/sqw_conf");


	// outputs
	mapJob["output/scan_file"] = "scan.out";
	mapJob["output/model_file"] = "model.out";
	mapJob["output/log_file"] = "log.out";
	mapJob["output/plot"] = "1";
	mapJob["output/plot_intermediate"] = "1";


	// neutrons
	mapJob["montecarlo/neutrons"] = propMC.Query<std::string>("taz/monteconvo/neutron_count");


	// fitting
	mapJob["fitter/minimiser"] = "simplex";
	mapJob["fitter/strategy"] = "1";
	mapJob["fitter/max_funccalls"] = "250.";
	mapJob["fitter/tolerance"] = "25.";
	mapJob["fitter/sigma"] = "1.";


	// resolution
	std::string strAlgo = "eck";
	switch(propMC.Query<int>("taz/monteconvo/algo"))
	{
		case 0: strAlgo = "cn"; break;
		case 1: strAlgo = "pop"; break;
		case 2: strAlgo = "eck"; break;
		case 3: strAlgo = "viol"; break;
	}
	mapJob["resolution/algorithm"] = strAlgo;

	std::string strFocMonoH, strFocMonoV;
	std::string strFocAnaH, strFocAnaV;
	switch(propMC.Query<int>("taz/monteconvo/mono_foc"))
	{
		case 0: strFocMonoH = "0"; strFocMonoV = "0"; break;
		case 1: strFocMonoH = "1"; strFocMonoV = "0"; break;
		case 2: strFocMonoH = "0"; strFocMonoV = "1"; break;
		case 3: strFocMonoH = "1"; strFocMonoV = "1"; break;
	}
	switch(propMC.Query<int>("taz/monteconvo/ana_foc"))
	{
		case 0: strFocAnaH = "0"; strFocAnaV = "0"; break;
		case 1: strFocAnaH = "1"; strFocAnaV = "0"; break;
		case 2: strFocAnaH = "0"; strFocAnaV = "1"; break;
		case 3: strFocAnaH = "1"; strFocAnaV = "1"; break;
	}

	mapJob["resolution/focus_mono_v"] = strFocMonoV;
	mapJob["resolution/focus_mono_h"] = strFocMonoH;
	mapJob["resolution/focus_ana_v"] = strFocAnaV;
	mapJob["resolution/focus_ana_h"] = strFocAnaH;

	mapJob["resolution/use_r0"] = "1";


	// parameters
	t_real dScale = propMC.Query<t_real>("taz/monteconvo/S_scale", t_real(1));
	t_real dOffs = propMC.Query<t_real>("taz/monteconvo/S_offs", t_real(0));

	mapJob["fit_parameters/params"] = "scale offs ";
	mapJob["fit_parameters/fixed"] = "1 1 ";
	mapJob["fit_parameters/values"] = tl::var_to_str(dScale) + " " + tl::var_to_str(dOffs) + " ";
	mapJob["fit_parameters/errors"] = tl::var_to_str(dScale*0.1) + " " + tl::var_to_str(dOffs*0.1) + " ";

	std::vector<std::string> vecParams = propMC.GetChildNodes("taz/monteconvo/sqw_params/");
	for(const std::string& strParam : vecParams)
	{
		t_real dVal = propMC.Query<t_real>(
			"taz/monteconvo/sqw_params/" + strParam, t_real(0));
		t_real dErr = propMC.Query<t_real>(
			"taz/monteconvo/sqw_errors/" + strParam, t_real(0));
		bool bFit = propMC.Query<bool>(
			"taz/monteconvo/sqw_fitvar/" + strParam, 0);

		if(bFit)	// open fit parameter
		{
			mapJob["fit_parameters/params"] += strParam + " ";
			mapJob["fit_parameters/values"] += tl::var_to_str(dVal) + " ";
			mapJob["fit_parameters/errors"] += tl::var_to_str(dErr) + " ";
			mapJob["fit_parameters/fixed"] += (bFit ? "0 " : "1 ");
		}
		else	// fixed sqw parameter
		{
			// get the full value string which might also contain code
			std::string strVal = propMC.Query<std::string>(
				"taz/monteconvo/sqw_params/" + strParam, "0");
			mapJob["input/sqw_set_params"] += strParam + " = " + strVal + "; ";
		}
	}


	// write temporary job file if none given
	if(strFile == "")
		strFile = tl::create_tmp_file<char>("convofit");

	tl::Prop<std::string> job;
	job.Add(mapJob);
	if(!job.Save(strFile, tl::PropType::INFO))
	{
		tl::log_err("Could not convert to convofit file format.");
		return "";
	}

	return strFile;
}
