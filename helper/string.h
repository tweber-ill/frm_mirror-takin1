/*
 * string helper
 * @author tweber
 * @date 06-mar-2013
 */

#ifndef __MIEZE_STRINGS__
#define __MIEZE_STRINGS__

#include <string>

inline std::string get_fileext(const std::string& str)
{
	unsigned int uiPos = str.find_last_of('.');

	if(uiPos == std::string::npos)
		return std::string("");
	return str.substr(uiPos+1);
}

inline std::string get_dir(const std::string& str)
{
	unsigned int uiPos = str.find_last_of("\\/");

	if(uiPos == std::string::npos)
		return std::string("");
	return str.substr(0, uiPos);
}

inline bool is_equal(const std::string& str0, const std::string& str1, bool bCase=false)
{
	if(bCase) return str0==str1;

	if(str0.size() != str1.size())
		return false;

	for(unsigned int i=0; i<str0.size(); ++i)
	{
		if(tolower(str0[i]) != tolower(str1[i]))
			return false;
	}
	return true;
}

#endif
