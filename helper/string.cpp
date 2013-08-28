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

std::string get_fileext(const std::string& str)
{
	std::size_t iPos = str.find_last_of('.');

	if(iPos == std::string::npos)
		return std::string("");
	return str.substr(iPos+1);
}

// e.g. returns "tof" for "123.tof.bz2"
std::string get_fileext2(const std::string& str)
{
	std::size_t iPos = str.find_last_of('.');
	if(iPos == std::string::npos || iPos == 0)
		return std::string("");

	std::string strFile = str.substr(0, iPos);
	return get_fileext(strFile);
}

std::string get_dir(const std::string& str)
{
	std::size_t iPos = str.find_last_of("\\/");

	if(iPos == std::string::npos)
		return std::string("");
	return str.substr(0, iPos);
}

std::string get_file(const std::string& str)
{
	std::size_t iPos = str.find_last_of("\\/");

	if(iPos == std::string::npos)
		return std::string("");
	return str.substr(iPos+1);
}

bool is_equal(const std::string& str0, const std::string& str1, bool bCase)
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

void trim(std::string& str)
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

bool find_and_replace(std::string& str1, const std::string& str_old,
                                                const std::string& str_new)
{
	std::size_t pos = str1.find(str_old);
	if(pos==std::string::npos)
			return false;

	str1.replace(pos, str_old.length(), str_new);
	return true;
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




std::pair<std::string, std::string>
	split_first(const std::string& str, const std::string& strSep, bool bTrim=0)
{
	std::string str1, str2;

	std::size_t pos = str.find(strSep);
	if(pos != std::string::npos)
	{
		str1 = str.substr(0, pos);
		if(pos+1 < str.length())
			str2 = str.substr(pos+1, std::string::npos);
	}

	if(bTrim)
	{
		::trim(str1);
		::trim(str2);
	}

	return std::pair<std::string, std::string>(str1, str2);
}




StringMap::StringMap(const char* pcKeyValSep, const char* pcComment)
{
	if(pcComment)
		m_strComment = pcComment;
	if(pcKeyValSep)
		m_strKeyValSeparator = pcKeyValSep;
}

StringMap::~StringMap()
{
}

void StringMap::ParseString(const std::string& strConf)
{
	std::istringstream istr(strConf);
	while(!istr.eof())
	{
		std::string strLine;
		std::getline(istr, strLine);

		trim(strLine);
		if(strLine.length()==0)
			continue;

		if(m_strComment.length()>0 && strLine.substr(0,m_strComment.length())==m_strComment)
			continue;

		std::pair<std::string, std::string> pairStr = split_first(strLine, m_strKeyValSeparator, 1);
		if(pairStr.first.length()==0 && pairStr.second.length()==0)
			continue;

		bool bInserted = m_map.insert(pairStr).second;
		if(!bInserted)
		{
			std::cerr << "Warning: Key \""
					<<  pairStr.first
					<< "\" already exists in map." << std::endl;
		}

		//std::cout << "key: \"" << pairStr.first <<"\" value:\"" << pairStr.second << "\"" << std::endl;
	}
}
