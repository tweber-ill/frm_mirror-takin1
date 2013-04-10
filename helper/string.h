/*
 * string helper
 * @author tweber
 * @date 06-mar-2013
 */

#ifndef __MIEZE_STRINGS__
#define __MIEZE_STRINGS__

#include <string>
#include <boost/tokenizer.hpp>
#include <iostream>
#include <sstream>

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

inline std::string get_file(const std::string& str)
{
	unsigned int uiPos = str.find_last_of("\\/");

	if(uiPos == std::string::npos)
		return std::string("");
	return str.substr(uiPos+1);
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

inline void trim(std::string& str)
{
        std::size_t posFirst = str.find_first_not_of(" \t");
        if(posFirst==std::string::npos)
        	posFirst = str.length();

        str.erase(str.begin(), str.begin()+posFirst);


        std::size_t posLast = str.find_last_not_of(" \t");
        if(posLast==std::string::npos)
                posLast = str.length();
        else
        	++posLast;

        str.erase(str.begin()+posLast, str.end());
}

inline void find_and_replace(std::string& str1, const std::string& str_old,
                                                const std::string& str_new)
{
        std::size_t pos = str1.find(str_old);
        if(pos==std::string::npos)
                return;

        str1.replace(pos, str_old.length(), str_new);
}

template<class T>
        void get_tokens(const std::string& str, const std::string& strDelim,
                                        std::vector<T>& vecRet)
{
        boost::char_separator<char> delim(strDelim.c_str());
        boost::tokenizer<boost::char_separator<char> > tok(str, delim);

        boost::tokenizer<boost::char_separator<char> >::iterator iter;
        for(iter=tok.begin(); iter!=tok.end(); ++iter)
        {
                std::istringstream istr(*iter);

                T t;
                istr >> t;              // WARNING: if T==string this terminates after first whitespace!

                vecRet.push_back(t);
        }
}

template<typename T>
std::string group_numbers(T tNum)
{
	struct Sep : std::numpunct<char>
	{
		Sep() : std::numpunct<char>(1) {}
		char do_thousands_sep() const { return ' ';}
		std::string do_grouping() const { return "\03"; }
	};
	Sep sep;

	std::ostringstream ostr;
	std::locale loc(ostr.getloc(), &sep);
	ostr.imbue(loc);

	ostr << tNum;
	return ostr.str();
}

#endif
