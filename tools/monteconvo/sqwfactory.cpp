/**
 * factory and plugin interface for S(q,w) models
 * @author Tobias Weber
 * @date 2016
 * @license GPLv2
 */

#include "sqwfactory.h"
#include "sqw.h"
#ifndef NO_PY
	#include "sqw_py.h"
#endif

#include "tlibs/log/log.h"
#include "tlibs/file/file.h"
#include "libs/globals.h"

#include <algorithm>
#include <unordered_map>


// key: identifier, value: [func, long name]
using t_mapSqw = std::unordered_map<std::string,
	std::tuple<std::shared_ptr<SqwBase>(*)(const std::string&), std::string>>;

static t_mapSqw g_mapSqw =
{
	{ "kd", t_mapSqw::mapped_type {
		[](const std::string& strCfgFile) -> std::shared_ptr<SqwBase>
		{ return std::make_shared<SqwKdTree>(strCfgFile.c_str()); },
		"Table" } },
	{ "phonon", t_mapSqw::mapped_type {
		[](const std::string& strCfgFile) -> std::shared_ptr<SqwBase>
		{ return std::make_shared<SqwPhonon>(strCfgFile.c_str()); }, 
		"Phonon Model" } },
#ifndef NO_PY
	{ "py", t_mapSqw::mapped_type {
		[](const std::string& strCfgFile) -> std::shared_ptr<SqwBase>
		{ return std::make_shared<SqwPy>(strCfgFile.c_str()); }, 
		"Python Model" } },
#endif
	{ "elastic",
		t_mapSqw::mapped_type {
		[](const std::string& strCfgFile) -> std::shared_ptr<SqwBase>
		{ return std::make_shared<SqwElast>(strCfgFile.c_str()); }, 
		"Elastic Model" } },
};


std::vector<std::tuple<std::string, std::string>> get_sqw_names()
{
	using t_tup = std::tuple<std::string, std::string>;
	std::vector<t_tup> vec;
	vec.reserve(g_mapSqw.size());

	for(const t_mapSqw::value_type& val : g_mapSqw)
	{
		t_tup tup;
		std::get<0>(tup) = val.first;
		std::get<1>(tup) = std::get<1>(val.second);

		vec.push_back(std::move(tup));
	}

	std::sort(vec.begin(), vec.end(), [](const t_tup& tup0, const t_tup& tup1) -> bool
	{
		const std::string& str0 = std::get<1>(tup0);
		const std::string& str1 = std::get<1>(tup1);

		return std::lexicographical_compare(str0.begin(), str0.end(), str1.begin(), str1.end());
	});
	return vec;
}

std::shared_ptr<SqwBase> construct_sqw(const std::string& strName, 
	const std::string& strConfigFile)
{
	typename t_mapSqw::const_iterator iter = g_mapSqw.find(strName);
	if(iter == g_mapSqw.end())
	{
		tl::log_err("No S(q,w) model of name \"", strName, "\" found.");
		return nullptr;
	}

	return (*std::get<0>(iter->second))(strConfigFile);
}


#ifdef USE_PLUGINS

#include <boost/dll/import.hpp>

void load_sqw_plugins()
{
	static bool bPluginsLoaded = 0;
	if(bPluginsLoaded) return;

	std::string strPlugins = find_resource_dir("plugins");
	if(strPlugins != "")
	{
		tl::log_info("Plugin directory: ", strPlugins);

		std::vector<std::string> vecPlugins = tl::get_all_files(strPlugins.c_str());
		for(const std::string& strPlugin : vecPlugins)
		{
			// TODO
		}
	}

	bPluginsLoaded = 1;
}

#else

void load_sqw_plugins()
{
	tl::log_err("No S(q,w) plugin interface available.");
}

#endif

