/**
 * @author Tobias Weber <tobias.weber@tum.de>
 * @license GPLv2
 */

#include <fontconfig/fontconfig.h>
#include <iostream>
#include "../helper/string.h"

int main()
{
	if(!FcInit())
	{
		std::cerr << "Cannot init fontconfig." << std::endl;
		return -1;
	}

	FcPattern *pPattern = FcPatternCreate();
	//FcObjectSet* pSet = FcObjectSetCreate();
	FcObjectSet* pSet = FcObjectSetBuild(FC_FILE, 0);
	FcFontSet* pFSet = FcFontList(FcConfigGetCurrent(), pPattern, pSet);
	std::cout << "Number of fonts: " << pFSet->nfont << std::endl;

	for(unsigned int i=0; i<pFSet->nfont; ++i)
	{
		FcChar8 *pcFile;
		FcPatternGetString(pFSet->fonts[i], FC_FILE, 0, &pcFile);

		std::string strFile((char*)pcFile);
		if(!contains<std::string>(strFile, "dejavusansmono.ttf", 0))
			continue;

		std::cout << "File: " << strFile << std::endl;

		FcStrFree(pcFile);
	}

	FcObjectSetDestroy(pSet);
	FcPatternDestroy(pPattern);
	FcFini();
	return 0;
}
