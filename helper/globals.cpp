/*
 * globals
 * @author tweber
 * @date 20-mar-2015
 * @license GPLv2
 */

#include "globals.h"
#include "tlibs/log/log.h"

#include <boost/filesystem.hpp>

// -----------------------------------------------------------------------------

unsigned int g_iPrec = 6;
unsigned int g_iPrecGfx = 4;

double g_dEps = 1e-6;
double g_dEpsGfx = 1e-4;

bool g_bHasFormfacts = 0;
bool g_bHasScatlens = 0;

#ifndef NO_QT
	QFont g_fontGfx, g_fontGL;
#endif

// -----------------------------------------------------------------------------


static const std::vector<std::string> s_vecInstallPaths =
{
	".",
#ifdef INSTALL_PREFIX
	INSTALL_PREFIX "/share/takin",
#endif
};

#ifndef NO_QT
QIcon load_icon(const std::string& strIcon)
{
	std::string strFile = find_resource(strIcon);
	if(strFile != "")
		return QIcon(strFile.c_str());

	return QIcon();
}
#endif

std::string find_resource(const std::string& strFile)
{
	for(const std::string& strPrefix : s_vecInstallPaths)
	{
		std::string _strFile = strPrefix + "/" + strFile;
		//tl::log_debug("Looking for file: ", _strFile);
		if(boost::filesystem::exists(_strFile))
			return _strFile;
	}

	tl::log_err("Could not load resource file \"", strFile, "\".");
	return "";
}

// -----------------------------------------------------------------------------
