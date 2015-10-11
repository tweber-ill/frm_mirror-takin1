/*
 * globals
 * @author tweber
 * @date 20-mar-2015
 * @copyright GPLv2
 */
 
#include "globals.h"
#include "tlibs/helper/log.h"

#include <boost/filesystem.hpp>

// ----------------------------------------------------------------------------- 
 
unsigned int g_iPrec = 6;
unsigned int g_iPrecGfx = 4;

// ----------------------------------------------------------------------------- 
 
static const std::vector<std::string> s_vecInstallPaths =
{
	".",
#ifdef INSTALL_PREFIX
	INSTALL_PREFIX "/share/takin",
#endif
};

QIcon load_icon(const std::string& strIcon)
{
	for(const std::string& strPrefix : s_vecInstallPaths)
	{
		std::string strFile = strPrefix + "/" + strIcon;
		//tl::log_debug("Looking for file: ", strFile);
		if(boost::filesystem::exists(strFile))
			return QIcon(strFile.c_str());
	}

	tl::log_err("Could not load icon \"", strIcon, "\".");
	return QIcon();
}

// -----------------------------------------------------------------------------
