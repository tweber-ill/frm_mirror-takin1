/*
 * string helper
 * @author tweber
 * @date 25-apr-2013
 */
#include "string.h"

std::string insert_before(const std::string& str,
						const std::string& strChar, const std::string& strInsert)
{
    std::size_t pos = str.find(strChar);
    if(pos==std::string::npos)
            return str;

    std::string strRet = str;
    strRet.insert(pos, strInsert);

	return strRet;
}


template<>
void get_tokens<std::string>(const std::string& str, const std::string& strDelim,
                                        std::vector<std::string>& vecRet)
{
	boost::char_separator<char> delim(strDelim.c_str());
	boost::tokenizer<boost::char_separator<char> > tok(str, delim);

	for(const std::string& strTok : tok)
		vecRet.push_back(strTok);
}
