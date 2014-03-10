/*
 * string helper
 * @author tweber
 * @date 06-mar-2013
 */

#ifndef __MIEZE_STRINGS__
#define __MIEZE_STRINGS__

#include <string>
#include <cstring>
#include <boost/tokenizer.hpp>
#include <iostream>
#include <sstream>
#include <map>

template<class t_str=std::string>
t_str insert_before(const t_str& str,
					const t_str& strChar,
					const t_str& strInsert)
{
	std::size_t pos = str.find(strChar);
	if(pos==t_str::npos)
			return str;

	t_str strRet = str;
	strRet.insert(pos, strInsert);

	return strRet;
}

template<class t_str=std::string>
t_str get_fileext(const t_str& str)
{
	std::size_t iPos = str.find_last_of('.');

	if(iPos == t_str::npos)
		return t_str();
	return str.substr(iPos+1);
}

// e.g. returns "tof" for "123.tof.bz2"
template<class t_str=std::string>
t_str get_fileext2(const t_str& str)
{
	std::size_t iPos = str.find_last_of('.');
	if(iPos == t_str::npos || iPos == 0)
		return t_str();

	t_str strFile = str.substr(0, iPos);
	return get_fileext(strFile);
}

template<class t_str=std::string> const t_str& get_dir_seps();
template<> const std::string& get_dir_seps();
template<> const std::wstring& get_dir_seps();


template<class t_str=std::string>
t_str get_dir(const t_str& str)
{
	std::size_t iPos = str.find_last_of(get_dir_seps<t_str>());

	if(iPos == t_str::npos)
		return t_str();
	return str.substr(0, iPos);
}

template<class t_str=std::string>
t_str get_file(const t_str& str)
{
	std::size_t iPos = str.find_last_of(get_dir_seps<t_str>());

	if(iPos == t_str::npos)
		return t_str();
	return str.substr(iPos+1);
}


template<class t_str=std::string>
bool is_equal(const t_str& str0, const t_str& str1, bool bCase=0)
{
	if(str0.size() != str1.size())
		return false;

	if(bCase) return str0==str1;

	for(unsigned int i=0; i<str0.size(); ++i)
	{
		if(std::tolower(str0[i]) != std::tolower(str1[i]))
			return false;
	}
	return true;
}


template<class t_str=std::string> const t_str& get_trim_chars();
template<> const std::string& get_trim_chars();
template<> const std::wstring& get_trim_chars();


template<class t_str=std::string>
void trim(t_str& str)
{
	std::size_t posFirst = str.find_first_not_of(get_trim_chars<t_str>());
	if(posFirst==t_str::npos)
		posFirst = str.length();

	str.erase(str.begin(), str.begin()+posFirst);


	std::size_t posLast = str.find_last_not_of(get_trim_chars<t_str>());
	if(posLast==std::string::npos)
			posLast = str.length();
	else
		++posLast;

	str.erase(str.begin()+posLast, str.end());
}


template<class t_str=std::string>
bool find_and_replace(t_str& str1, const t_str& str_old,
						const t_str& str_new)
{
	std::size_t pos = str1.find(str_old);
	if(pos==t_str::npos)
			return false;

	str1.replace(pos, str_old.length(), str_new);
	return true;
}

template<class t_str=std::string>
void find_all_and_replace(t_str& str1, const t_str& str_old,
						const t_str& str_new)
{
	while(1)
	{
		std::size_t pos = str1.find(str_old);
		if(pos==t_str::npos)
			break;
		str1.replace(pos, str_old.length(), str_new);
	}
}


template<class t_str=std::string>
bool begins_with(const t_str& str, const t_str& strBeg)
{
	if(str.length() < strBeg.length())
		return false;

	for(unsigned int i=0; i<strBeg.length(); ++i)
		if(str[i] != strBeg[i])
			return false;

	return true;
}


template<class t_str=std::string>
std::pair<t_str, t_str>
split_first(const t_str& str, const t_str& strSep, bool bTrim=0)
{
	t_str str1, str2;

	std::size_t pos = str.find(strSep);
	if(pos != t_str::npos)
	{
		str1 = str.substr(0, pos);
		if(pos+1 < str.length())
			str2 = str.substr(pos+1, t_str::npos);
	}

	if(bTrim)
	{
		::trim(str1);
		::trim(str2);
	}

	return std::pair<t_str, t_str>(str1, str2);
}

//extern std::vector<std::string>
//		split(const std::string& str, const std::string& strSep);


template<class T, class t_str=std::string>
void get_tokens(const t_str& str, const t_str& strDelim,
				std::vector<T>& vecRet)
{
	typedef typename t_str::value_type t_char;

	boost::char_separator<t_char> delim(strDelim.c_str());
	boost::tokenizer<boost::char_separator<t_char> > tok(str, delim);

	typename boost::tokenizer<boost::char_separator<t_char> >::iterator iter;
	for(iter=tok.begin(); iter!=tok.end(); ++iter)
	{
		std::basic_istringstream<t_char> istr(*iter);

		T t;
		istr >> t;              // WARNING: if T==string this terminates after first whitespace!

		vecRet.push_back(t);
	}
}

template<>
void get_tokens<std::string>(const std::string& str, const std::string& strDelim,
                                        std::vector<std::string>& vecRet);
template<>
void get_tokens<std::wstring>(const std::wstring& str, const std::wstring& strDelim,
                                        std::vector<std::wstring>& vecRet);


template<typename T, class t_str=std::string>
T str_to_var(const t_str& str)
{
	typedef typename t_str::value_type t_char;

	std::basic_istringstream<t_char> istr(str);
	T t;

	istr >> t;
	return t;
}

template<typename T, class t_str=std::string>
t_str var_to_str(const T& t)
{
	typedef typename t_str::value_type t_char;

	std::basic_ostringstream<t_char> ostr;
	ostr << t;

	return ostr.str();
}


// e.g. str = "123.4 +- 0.5"
template<typename T=double, class t_str=std::string>
void get_val_and_err(const t_str& str, T& val, T& err)
{
	// "+-", inelegant...
	t_str strPlusMinus;
	strPlusMinus.resize(2);
	strPlusMinus[0] = '+';
	strPlusMinus[1] = '-';

	std::vector<T> vec;
	get_tokens(str, strPlusMinus, vec);

	if(vec.size() >= 1)
		val = vec[0];
	if(vec.size() >= 2)
		err = vec[1];
}


template<typename T, typename t_char = char>
std::string group_numbers(T tNum)
{
	struct Sep : std::numpunct<t_char>
	{
		Sep() : std::numpunct<t_char>(1) {}
		t_char do_thousands_sep() const { return ' ';}
		std::string do_grouping() const { return "\03"; }
	};
	Sep sep;

	std::basic_ostringstream<t_char> ostr;
	std::locale loc(ostr.getloc(), &sep);
	ostr.imbue(loc);

	ostr << tNum;
	return ostr.str();
}

extern std::wstring str_to_wstr(const std::string& str);
extern std::string wstr_to_str(const std::wstring& str);




class StringMap;
class StringMap
{
	public:
		typedef std::map<std::string, std::string> t_map;

	protected:
		t_map m_map;
		std::string m_strKeyValSeparator;
		std::string m_strComment;

	public:
		StringMap(const char* pcKeyValSep=":", const char* pcComment="#");
		virtual ~StringMap();

		void ParseString(const std::string& strConf);

		void SetMap(const t_map& map) { m_map = map; }
		const t_map& GetMap() const { return m_map; }
		t_map& GetMap() { return m_map; }

		bool Serialize(std::ostream& ostrSer) const;
		bool Deserialize(const void* pvMem, unsigned int iLen);

		bool HasKey(const std::string& str) const;
		std::string& operator[](const std::string& str);
		const std::string& operator[](const std::string& str) const;

		const std::vector<std::string> GetKeys() const;

		void Trim();
		void MergeFrom(const std::vector<const StringMap*>& vecMaps);
};

extern std::ostream& operator<<(std::ostream& ostr, const StringMap& mapStr);


template<typename T>
std::map<T, T> vecmap_to_map(const std::map<T, std::vector<T> >& themap)
{
	std::map<T,T> singleMap;

	for(const auto& pair : themap)
	{
		const T& key = pair.first;
		const std::vector<T>& vect = pair.second;

		T tval;
		if(vect.size() > 0)
			tval = vect[0];

		singleMap.insert(std::pair<T, T>(pair.first, tval));
	}

	return singleMap;
}

#endif
