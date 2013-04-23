/*
 * xml helper
 * @author tweber
 * @date 23-apr-2013
 */

#include "xml.h"

Xml::Xml()
{}


Xml::~Xml()
{ }

bool Xml::Load(const char* pcFile)
{
	return false;
}

std::string Xml::QueryString(const char* pcAdr, const char* pcDef, bool *pbOk) const
{
	return "";
}

double Xml::QueryDouble(const char* pcAdr, double dDef, bool *pbOk) const
{
	return 0.;
}
