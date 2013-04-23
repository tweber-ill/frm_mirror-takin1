/*
 * xml helper
 * @author tweber
 * @date 23-apr-2013
 */

#ifndef __MIEZE_XML__
#define __MIEZE_XML__

#include <string>

class Xml
{
protected:

public:
	Xml();
	virtual ~Xml();

	bool Load(const char* pcFile);

	std::string QueryString(const char* pcAdr, const char* pcDef, bool *pbOk) const;
	double QueryDouble(const char* pcAdr, double dDef, bool *pbOk) const;
};

#endif
