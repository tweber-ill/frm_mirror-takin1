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
#include <map>

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
extern std::pair<std::string, std::string>
		split_first(const std::string& str, const std::string& strSep);
//extern std::vector<std::string>
//		split(const std::string& str, const std::string& strSep);

extern bool begins_with(const std::string& str, const std::string& strBeg);

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

// e.g. str = "123.4 +- 0.5"
template<typename T=double>
void get_val_and_err(const std::string& str, T& val, T& err)
{
	std::vector<T> vec;
	get_tokens(str, "+-", vec);

	if(vec.size() >= 1)
		val = vec[0];
	if(vec.size() >= 2)
		err = vec[1];
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
