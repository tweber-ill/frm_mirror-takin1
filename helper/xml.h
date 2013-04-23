/*
 * xml helper
 * @author tweber
 * @date 23-apr-2013
 */

#ifndef __MIEZE_XML__
#define __MIEZE_XML__

#include <string>
#include <sstream>

#include <QtCore/QBuffer>

class Xml
{
protected:
	QBuffer m_bufXml;

public:
	Xml();
	virtual ~Xml();

	bool Load(const char* pcFile);

	std::string QueryString(const char* pcAdr, const char* pcDef, bool *pbOk);

	template<typename T>
	T Query(const char* pcAdr, T tDef, bool *pbOk)
	{
		std::ostringstream ostrDef;
		ostrDef << tDef;

		bool bOk = 0;
		std::string str = QueryString(pcAdr, ostrDef.str().c_str(), &bOk);
		if(pbOk) *pbOk = bOk;

		if(bOk)
		{
			T tRes(0);

			std::istringstream istr(str);
			istr >> tRes;

			return tRes;
		}
		else
			return tDef;
	}
};

#endif
