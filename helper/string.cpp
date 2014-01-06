/*
 * string helper
 * @author tweber
 * @date 25-apr-2013
 */
#include "string.h"
#include "misc.h"

#ifndef NO_COMP
#include "comp.h"
#endif

#include <cstring>

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

void find_all_and_replace(std::string& str1, const std::string& str_old,
						const std::string& str_new)
{
        while(1)
        {
                std::size_t pos = str1.find(str_old);
                if(pos==std::string::npos)
                                break;
                str1.replace(pos, str_old.length(), str_new);
        }
}


bool begins_with(const std::string& str, const std::string& strBeg)
{
	if(str.length() < strBeg.length())
		return false;

	for(unsigned int i=0; i<strBeg.length(); ++i)
		if(str[i] != strBeg[i])
			return false;

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

const std::vector<std::string> StringMap::GetKeys() const
{
	std::vector<std::string> vecKeys;
	vecKeys.reserve(m_map.size());

	for(const t_map::value_type& val : m_map)
		vecKeys.push_back(val.first);

	return vecKeys;
}

bool StringMap::HasKey(const std::string& str) const
{
	return (m_map.find(str) != m_map.end());
}

std::string& StringMap::operator[](const std::string& str)
{
	t_map::iterator iter = m_map.find(str);
	if(iter == m_map.end())
		return m_map.insert(t_map::value_type(str, "")).first->second;

	return iter->second;
}

const std::string& StringMap::operator[](const std::string& str) const
{
	static std::string strDummy;

	t_map::const_iterator iter = m_map.find(str);
	if(iter != m_map.end())
		return iter->second;

	//std::cerr << "Warning: Returning dummy object." << std::endl;
	return strDummy;
}

void StringMap::Trim()
{
	for(t_map::value_type& pair : m_map)
	{
		trim(pair.second);

		if(pair.first == "" || pair.second == "")
			m_map.erase(pair.first);
	}
}

void StringMap::MergeFrom(const std::vector<const StringMap*>& vecMaps)
{
	m_map.clear();

	for(const StringMap* pMap : vecMaps)
		merge_map<std::string, std::string>(m_map, pMap->GetMap());
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

#ifndef NO_COMP
bool StringMap::Serialize(std::ostream& ostrSer) const
{
	unsigned int iLen = 0;
	for(const auto& pair : m_map)
	{
		const std::string& strKey = pair.first;
		const std::string& strVal = pair.second;

		iLen += strKey.length()+1;
		iLen += strVal.length()+1;
	}

	char *pcMem = new char[iLen];
	memset(pcMem, 0, iLen);
	unsigned int iCurIdx=0;

	for(const auto& pair : m_map)
	{
		const std::string& strKey = pair.first;
		const std::string& strVal = pair.second;

		memcpy(pcMem+iCurIdx, strKey.c_str(), strKey.length());
		iCurIdx += strKey.length()+1;
		memcpy(pcMem+iCurIdx, strVal.c_str(), strVal.length());
		iCurIdx += strVal.length()+1;
	}

	bool bOk = comp_mem_to_stream(pcMem, iLen, ostrSer, COMP_BZ2);
	delete[] pcMem;

	return bOk;
}


static inline std::vector<std::string> split0(const char* pcMem, unsigned int iLen)
{
	std::vector<std::string> vecStr;
	if(iLen==0) return vecStr;

	vecStr.push_back(std::string(pcMem));
	//std::cout << pcMem << std::endl;

	for(unsigned int iIdx=0; iIdx<iLen-1; ++iIdx)
	{
		if(pcMem[iIdx] == 0)
		{
			std::string str = pcMem+iIdx+1;
			trim(str);
			vecStr.push_back(str);

			//std::cout << str << std::endl;
		}
	}

	return vecStr;
}

bool StringMap::Deserialize(const void* pvMem, unsigned int iLen)
{
	char *pcUncomp = 0;
	unsigned int iLenUncomp = 0;
	if(!::decomp_mem_to_mem(pvMem, iLen, (void*&)pcUncomp, iLenUncomp))
		return false;

	std::vector<std::string> vecStrings = split0(pcUncomp, iLenUncomp);
	if(pcUncomp) delete[] pcUncomp;

	if(vecStrings.size()%2 != 0)
	{
		std::cerr << "Error: Uneven number of strings in key/value map."
				  << std::endl;
		return false;
	}

	for(unsigned int iIdx=0; iIdx<vecStrings.size(); iIdx+=2)
	{
		const std::string& strKey = vecStrings[iIdx];
		const std::string& strVal = vecStrings[iIdx+1];

		m_map.insert(std::pair<std::string,std::string>(strKey, strVal));
	}

	Trim();
	return true;
}
#else
bool StringMap::Serialize(std::ostream& ostrSer) const
{
	std::cerr << "Error: Serialize not linked." << std::endl;
	return false;
}

bool StringMap::Deserialize(const void* pvMem, unsigned int iLen)
{
	std::cerr << "Error: Deserialize not linked." << std::endl;
	return false;
}
#endif

std::ostream& operator<<(std::ostream& ostr, const StringMap& mapStr)
{
	const StringMap::t_map& map = mapStr.GetMap();
	for(const StringMap::t_map::value_type& pair : map)
		ostr <<  pair.first << " = " << pair.second << std::endl;
	return ostr;
}
