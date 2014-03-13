/*
 * string helper
 * @author tweber
 * @date 25-apr-2013
 */
#include "string.h"
#include "misc.h"

#include <locale>
//#include <codecvt>


template<> const std::string& get_dir_seps()
{
	static const std::string strSeps("\\/");
	return strSeps;
}
template<> const std::wstring& get_dir_seps()
{
	static const std::wstring strSeps(L"\\/");
	return strSeps;
}


template<> const std::string& get_trim_chars()
{
	static const std::string strC(" \t");
	return strC;
}
template<> const std::wstring& get_trim_chars()
{
	static const std::wstring strC(L" \t");
	return strC;
}



template<>
void get_tokens<std::string>(const std::string& str,
							const std::string& strDelim,
							std::vector<std::string>& vecRet)
{
	boost::char_separator<char> delim(strDelim.c_str());
	boost::tokenizer<boost::char_separator<char> > tok(str, delim);

	for(const std::string& strTok : tok)
		vecRet.push_back(strTok);
}

template<>
void get_tokens<std::wstring>(const std::wstring& str,
							const std::wstring& strDelim,
							std::vector<std::wstring>& vecRet)
{
	boost::char_separator<wchar_t> delim(strDelim.c_str());
	boost::tokenizer<boost::char_separator<wchar_t>,
					std::wstring::const_iterator,
					std::wstring>
							tok(str, delim);

	for(const std::wstring& strTok : tok)
		vecRet.push_back(strTok);
}


// see: http://www.cplusplus.com/reference/locale/wstring_convert/
std::wstring str_to_wstr(const std::string& str)
{
	//std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
	//return conv.fromBytes(str);

	return std::wstring(str.begin(), str.end());
}

std::string wstr_to_str(const std::wstring& str)
{
	//std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
	//return conv.toBytes(str);

	return std::string(str.begin(), str.end());
}
