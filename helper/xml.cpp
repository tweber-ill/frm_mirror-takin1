/*
 * xml helper
 * @author tweber
 * @date 23-apr-2013
 */

#include "xml.h"
#include "../helper/string.h"
#include "../helper/misc.h"

#include <QtXmlPatterns/QXmlQuery>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include <fstream>
#include <list>
#include <vector>


Xml::Xml() : m_bOK(0)
{}

Xml::~Xml()
{}

bool Xml::Load(const char* pcFile)
{
	QFile file(pcFile);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		m_bOK = 0;
		return false;
	}

	QTextStream istr(&file);
	QString strXml = istr.readAll();

	m_bufXml.close();
	m_bufXml.setData(strXml.toUtf8());
	m_bufXml.open(QIODevice::ReadOnly);

	file.close();

	m_bOK = 1;
	return true;
}

std::string Xml::QueryString(const char* pcAdr, const char* pcDef, bool *pbOk)
{
	if(!m_bOK) return pcDef;

	m_bufXml.seek(0);

	std::string strAdr(pcAdr);
	::trim(strAdr);
	if(strAdr.length()==0)
	{
		if(pbOk) *pbOk=0;
		return "";
	}
	if(strAdr[0] != '/')
		strAdr = std::string("/") + strAdr;

	std::ostringstream ostrQuery;
	ostrQuery << "doc($xml)" << strAdr << "/string()";

	QXmlQuery query;
	query.bindVariable("xml", &m_bufXml);
	query.setQuery(QString(ostrQuery.str().c_str()));

	QStringList strlstOut;
	bool bOk = query.evaluateTo(&strlstOut);
	if(!bOk || strlstOut.size()==0)
	{
		if(pbOk) *pbOk = false;

		if(pcDef)
			return std::string(pcDef);
		return "";
	}

	if(pbOk) *pbOk = true;
	std::string strOut = strlstOut.at(0).toStdString();
	::trim(strOut);

	return strOut;
}


// -------------------------------------------------------------------------------
// saving

struct XmlNode
{
	std::string strName, strVal;
	std::vector<XmlNode*> vecChildren;

	XmlNode() {}

	~XmlNode()
	{
		for(XmlNode* pNode : vecChildren)
			delete pNode;
	}

	XmlNode* GetChild(const std::string& strKey)
	{
		for(XmlNode* pNode : vecChildren)
		{
			if(pNode->strName == strKey)
				return pNode;
		}

		XmlNode *pNode = new XmlNode;
		vecChildren.push_back(pNode);
		pNode->strName = strKey;
		return pNode;
	}

	void SortIntoTree(const std::list<std::string>& lstKey, const std::string& strVal)
	{
		if(lstKey.size() == 0)
			return;

		XmlNode* pNode = GetChild(*lstKey.begin());
		if(lstKey.size()==1)
		{
			pNode->strVal = strVal;
		}
		else
		{
			std::list<std::string> lstKeyNext = lstKey;
			lstKeyNext.pop_front();

			pNode->SortIntoTree(lstKeyNext, strVal);
		}
	}

	void Print(std::ostream& ostr, unsigned int iLevel=1) const
	{
		const XmlNode& node = *this;

		ostr << "<" << node.strName << ">";
		if(node.vecChildren.size()==0)
			ostr << " " << node.strVal << " ";
		else
		{
			ostr << "\n";
			for(const XmlNode* pNode : node.vecChildren)
			{
				for(unsigned int iLev=0; iLev<iLevel; ++iLev)
					ostr << "\t";
				pNode->Print(ostr, iLevel+1);
				ostr << "\n";
			}
		}
		ostr << "</" << node.strName << ">";
	}
};

std::ostream& operator<<(std::ostream& ostr, const XmlNode& node)
{
	if(node.vecChildren.size() == 1)
		node.vecChildren[0]->Print(ostr);
 	return ostr;
}

bool Xml::SaveMap(const char* pcFile, const std::map<std::string, std::string>& mapXml)
{
	std::ofstream ofstr(pcFile);
	if(!ofstr.is_open())
		return false;

	XmlNode node;
	node.strName = "root";
	for(const std::pair<std::string, std::string>& pairXml : mapXml)
	{
		const std::string& strKey = pairXml.first;
		const std::string& strVal = pairXml.second;

		std::vector<std::string> vecKey;
		get_tokens<std::string>(strKey, "/", vecKey);

		std::list<std::string> lstKey = vector_to_list(vecKey);
		node.SortIntoTree(lstKey, strVal);
	}

	ofstr << node;
	return true;
}
