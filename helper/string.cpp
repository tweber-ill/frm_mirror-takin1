/*
 * string helper
 * @author tweber
 * @date 25-apr-2013
 */
#include "string.h"

template<>
void get_tokens<std::string>(const std::string& str, const std::string& strDelim,
                                        std::vector<std::string>& vecRet)
{
	boost::char_separator<char> delim(strDelim.c_str());
	boost::tokenizer<boost::char_separator<char> > tok(str, delim);

	for(const std::string& strTok : tok)
		vecRet.push_back(strTok);
}
