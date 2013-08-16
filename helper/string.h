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

extern std::string
insert_before(const std::string& str,
				const std::string& strChar,
				const std::string& strInsert);
extern std::string get_fileext(const std::string& str);
extern std::string get_fileext2(const std::string& str);
extern std::string get_dir(const std::string& str);
extern std::string get_file(const std::string& str);
extern bool is_equal(const std::string& str0, const std::string& str1, bool bCase=false);
extern void trim(std::string& str);
extern bool find_and_replace(std::string& str1, const std::string& str_old,
                                                const std::string& str_new);


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

template<>
void get_tokens<std::string>(const std::string& str, const std::string& strDelim,
                                        std::vector<std::string>& vecRet);

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
