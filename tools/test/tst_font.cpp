/**
 * @author Tobias Weber <tobias.weber@tum.de>
 * @license GPLv2
 */

// gcc -I/usr/include/freetype2 -o tst_font tst_font.cpp -lstdc++ -lfreetype -std=c++11

#include <iostream>
#include <iomanip>
#include <ft2build.h>
#include FT_FREETYPE_H

int main()
{
	std::cout << "Char: ";
	char ch;
	std::cin >> ch;

	FT_Library ftLib;
	if(FT_Init_FreeType(&ftLib) != 0)
	{
		std::cerr << "Error opening freetype." << std::endl;
		return -1;
	}

	FT_Face ftFace;
	if(FT_New_Face(ftLib, "/usr/share/fonts/dejavu/DejaVuSansMono.ttf", 0, &ftFace) != 0)
	{
		std::cerr << "Error loading font." << std::endl;
		return -1;
	}

	FT_Set_Pixel_Sizes(ftFace, 0, 32);
	if(FT_Load_Char(ftFace, ch, FT_LOAD_RENDER/*|FT_LOAD_MONOCHROME*/) != 0)
	{
		std::cerr << "Error rendering font." << std::endl;
		return -1;
	}

	int iAsc = /*int(ftFace->ascender>>6)*/ int(ftFace->bbox.yMax>>6);
	int iDesc = /*int(ftFace->descender>>6)*/ int(ftFace->bbox.yMin>>6);
	int iGlobH = iAsc-iDesc /*int(ftFace->height>>6)*/;
	int iGlobW = (ftFace->bbox.xMax>>6)-(ftFace->bbox.xMin>>6) /*int(ftFace->height>>6)*/;
	int iBelow = -iDesc;

	std::cout << "asc: " << iAsc 
		<< ", desc: " << iDesc 
		<< ", glob h: " << iGlobH
		<< ", glob w: " << iGlobW 
		<< std::endl;

	unsigned char* pcBmp = ftFace->glyph->bitmap.buffer;

	unsigned int iW = ftFace->glyph->bitmap.width;
	unsigned int iH = ftFace->glyph->bitmap.rows;
	FT_Int iLeft = ftFace->glyph->bitmap_left;
	FT_Int iTop = ftFace->glyph->bitmap_top;
	FT_Pos iRight = ftFace->glyph->advance.x >> 6;
	FT_Pos iBottom = ftFace->glyph->advance.y >> 6;

	std::cout << "left: " << iLeft << ", top: " << iTop 
		<< ", right: " <<  iRight << ", bottom: " << iBottom
		<< std::endl;


	for(int iY=0; iY<iH; ++iY)
	{
		for(int iX=0; iX<iW; ++iX)
		{
			std::cout << std::hex << std::setw(2)
				<< short(*(pcBmp+iY*iW + iX))
				<< " ";
		}

		std::cout << "\n";
	}


	FT_Done_Face(ftFace);
	FT_Done_FreeType(ftLib);
	return 0;
}
