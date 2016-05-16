/*
 * globals
 * @author tweber
 * @date 20-mar-2015
 * @license GPLv2
 */

#ifndef __TAZ_GLOBALS_H__
#define __TAZ_GLOBALS_H__

#include <string>


//using t_real_glob = float;
using t_real_glob = double;


extern unsigned int g_iPrec;
extern unsigned int g_iPrecGfx;

extern t_real_glob g_dEps;
extern t_real_glob g_dEpsGfx;

extern bool g_bHasFormfacts;
extern bool g_bHasScatlens;
extern bool g_bShowFsq;

extern void add_resource_path(const std::string& strPath);
extern std::string find_resource(const std::string& strFile);


#ifndef NO_QT
	#include <QIcon>
	#include <QFont>

	extern QIcon load_icon(const std::string& strIcon);
	extern QFont g_fontGen, g_fontGfx, g_fontGL;
#endif


#endif
