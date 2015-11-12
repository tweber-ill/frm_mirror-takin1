/*
 * globals
 * @author tweber
 * @date 20-mar-2015
 * @license GPLv2
 */

#ifndef __TAZ_GLOBALS_H__
#define __TAZ_GLOBALS_H__

#include <string>


extern unsigned int g_iPrec;
extern unsigned int g_iPrecGfx;
extern bool g_bHasFormfacts;
extern bool g_bHasScatlens;


extern std::string find_resource(const std::string& strFile);


#ifndef NO_QT
	#include <QIcon>
	extern QIcon load_icon(const std::string& strIcon);
#endif


#endif

