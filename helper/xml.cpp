/*
 * xml helper
 * @author tweber
 * @date 23-apr-2013
 */

#include "xml.h"
#include "../helper/string.h"

#include <QtXmlPatterns/QXmlQuery>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <QtCore/QTextStream>


Xml::Xml()
{}

Xml::~Xml()
{ }

bool Xml::Load(const char* pcFile)
{
	QFile file(pcFile);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;

	QTextStream istr(&file);
	QString strXml = istr.readAll();

	m_bufXml.close();
	m_bufXml.setData(strXml.toUtf8());
	m_bufXml.open(QIODevice::ReadOnly);

	file.close();
	return true;
}

std::string Xml::QueryString(const char* pcAdr, const char* pcDef, bool *pbOk)
{
	std::ostringstream ostrQuery;
	ostrQuery << "doc($xml)" << pcAdr << "/string()";

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
