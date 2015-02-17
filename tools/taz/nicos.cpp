/*
 * Connection to Nicos
 * @author tweber
 * @date 27-aug-2014
 * @copyright GPLv2
 */

#include "nicos.h"
#include "tlibs/string/string.h"
#include "tlibs/helper/log.h"
#include "tlibs/helper/py.h"


NicosCache::NicosCache(QSettings* pSettings) : m_pSettings(pSettings)
{
	std::vector<std::pair<std::string, std::string*>> vecStrings =
	{
		{"sample_name", &m_strSampleName},
		{"lattice", &m_strSampleLattice},
		{"angles", &m_strSampleAngles},
		{"orient1", &m_strSampleOrient1},
		{"orient2", &m_strSampleOrient2},
		{"spacegroup", &m_strSampleSpacegroup},
		{"psi0", &m_strSamplePsi0},
		{"stheta", &m_strSampleTheta},
		{"s2theta", &m_strSample2Theta},
		{"mtheta", &m_strMonoTheta},
		{"m2theta", &m_strMono2Theta},
		{"mono_d", &m_strMonoD},
		{"atheta", &m_strAnaTheta},
		{"a2theta", &m_strAna2Theta},
		{"ana_d", &m_strAnaD},
		{"stheta_aux", &m_strSampleTheta_aux},
		{"stheta_aux_alias", &m_strSampleTheta_aux_alias}
	};

	for(const std::pair<std::string, std::string*>& pair : vecStrings)
	{
		std::string strKey = std::string("net/") + pair.first;
		if(!m_pSettings->contains(strKey.c_str()))
			continue;

		*pair.second = m_pSettings->value(strKey.c_str(), pair.second->c_str()
										).toString().toStdString();
	}

	m_vecKeys = std::vector<std::string>
	{
		m_strSampleName,
		m_strSampleLattice, m_strSampleAngles,
		m_strSampleOrient1, m_strSampleOrient2,
		m_strSampleSpacegroup,
		m_strSamplePsi0, m_strSampleTheta, m_strSample2Theta,

		m_strMonoD, m_strMonoTheta, m_strMono2Theta,
		m_strAnaD, m_strAnaTheta, m_strAna2Theta,

		m_strSampleTheta_aux, m_strSampleTheta_aux_alias,

		"logbook/remark",
	};

	m_tcp.add_connect(boost::bind(&NicosCache::slot_connected, this, _1, _2));
	m_tcp.add_disconnect(boost::bind(&NicosCache::slot_disconnected, this, _1, _2));
	m_tcp.add_receiver(boost::bind(&NicosCache::slot_receive, this, _1));
}

NicosCache::~NicosCache()
{
	disconnect();
}

void NicosCache::connect(const std::string& strHost, const std::string& strPort)
{
	m_mapCache.clear();
	emit cleared_cache();

	if(!m_tcp.connect(strHost, strPort))
		return;
}

void NicosCache::disconnect()
{
	m_tcp.disconnect();
}

void NicosCache::AddKey(const std::string& strKey)
{
	// connection is asynchronous -> cannot yet request data from server
	//if(!m_tcp.is_connected())
	//	return false;

	m_vecKeys.push_back(strKey);
}

void NicosCache::ClearKeys()
{
	m_vecKeys.clear();
}

void NicosCache::RefreshKeys()
{
	std::string strMsg;
	for(const std::string& strKey : m_vecKeys)
		strMsg += "@"+strKey+"?\n";
	m_tcp.write(strMsg);
}

void NicosCache::RegisterKeys()
{
	std::string strMsg;
	for(const std::string& strKey : m_vecKeys)
		strMsg += "@"+strKey+":\n";
	m_tcp.write(strMsg);
}


void NicosCache::slot_connected(const std::string& strHost, const std::string& strSrv)
{
	tl::log_info("Connected to ", strHost, " on port ", strSrv, ".");

	QString qstrHost = strHost.c_str();
	QString qstrSrv = strSrv.c_str();
	emit connected(qstrHost, qstrSrv);

	RegisterKeys();
	RefreshKeys();
}

void NicosCache::slot_disconnected(const std::string& strHost, const std::string& strSrv)
{
	tl::log_info("Disconnected from ", strHost, " on port ", strSrv, ".");

	emit disconnected();
}

void NicosCache::slot_receive(const std::string& str)
{
	//log_debug("Received: ", str);

	std::pair<std::string, std::string> pairTimeVal = tl::split_first<std::string>(str, "@", 1);
	std::pair<std::string, std::string> pairKeyVal = tl::split_first<std::string>(pairTimeVal.second, "=", 1);
	if(pairKeyVal.second == "")
	{
		pairKeyVal = tl::split_first<std::string>(pairTimeVal.second, "!", 1);
		if(pairKeyVal.second != "")
			tl::log_warn("Value \"", pairKeyVal.second, "\" for \"", pairKeyVal.first, "\" is marked as invalid.");
		else
			tl::log_err("Invalid net reply: \"", str, "\"");
	}


	const std::string& strKey = pairKeyVal.first;
	const std::string& strVal = pairKeyVal.second;


	const std::string& strTime = pairTimeVal.first;
	double dTimestamp = tl::str_to_var<double>(strTime);

	if(strVal.length() == 0)
		return;

	CacheVal cacheval;
	cacheval.strVal = strVal;
	cacheval.dTimestamp = dTimestamp;
	m_mapCache[strKey] = cacheval;

	//std::cout << strKey << " = " << strVal << std::endl;
	emit updated_cache_value(strKey, cacheval);



	CrystalOptions crys;
	TriangleOptions triag;

	if(strKey == m_strSampleLattice)
	{
		std::vector<double> vecLattice = tl::get_py_array(strVal);
		if(vecLattice.size() != 3)
			return;

		crys.bChangedLattice = 1;
		for(int i=0; i<3; ++i)
			crys.dLattice[i] = vecLattice[i];
	}
	else if(strKey == m_strSampleAngles)
	{
		std::vector<double> vecAngles = tl::get_py_array(strVal);
		if(vecAngles.size() != 3)
			return;

		crys.bChangedLatticeAngles = 1;
		for(int i=0; i<3; ++i)
			crys.dLatticeAngles[i] = vecAngles[i];
	}
	else if(strKey == m_strSampleOrient1)
	{
		std::vector<double> vecOrient = tl::get_py_array(strVal);
		if(vecOrient.size() != 3)
			return;

		crys.bChangedPlane1 = 1;
		for(int i=0; i<3; ++i)
			crys.dPlane1[i] = vecOrient[i];
	}
	else if(strKey == m_strSampleOrient2)
	{
		std::vector<double> vecOrient = tl::get_py_array(strVal);
		if(vecOrient.size() != 3)
			return;

		crys.bChangedPlane2 = 1;
		for(int i=0; i<3; ++i)
			crys.dPlane2[i] = -vecOrient[i];	// hack to convert left-handed (nicos) to right-handed coordinates
	}
	else if(strKey == m_strSampleSpacegroup)
	{
		crys.strSpacegroup = tl::get_py_string(strVal);
		crys.bChangedSpacegroup = 1;
	}
	else if(strKey == m_strMono2Theta)
	{
		triag.dMonoTwoTheta = tl::str_to_var<double>(strVal) /180.*M_PI;
		triag.bChangedMonoTwoTheta = 1;
	}
	else if(strKey == m_strAna2Theta)
	{
		triag.dAnaTwoTheta = tl::str_to_var<double>(strVal) /180.*M_PI;
		triag.bChangedAnaTwoTheta = 1;
	}
	else if(strKey == m_strSample2Theta)
	{
		triag.dTwoTheta = tl::str_to_var<double>(strVal) /180.*M_PI;
		triag.bChangedTwoTheta = 1;
	}
	else if(strKey == m_strMonoD)
	{
		triag.dMonoD = tl::str_to_var<double>(strVal);
		triag.bChangedMonoD = 1;
	}
	else if(strKey == m_strAnaD)
	{
		triag.dAnaD = tl::str_to_var<double>(strVal);
		triag.bChangedAnaD = 1;
	}
	else if(strKey == m_strSampleName)
	{
		crys.strSampleName = tl::get_py_string(strVal);
	}


	if(m_mapCache.find(m_strSampleTheta) != m_mapCache.end()
			&& m_mapCache.find(m_strSamplePsi0) != m_mapCache.end()
			&& m_mapCache.find(m_strSampleTheta_aux) != m_mapCache.end()
			&& m_mapCache.find(m_strSampleTheta_aux_alias) != m_mapCache.end())
	{
		// rotation of crystal -> rotation of plane (or triangle) -> sample theta

		double dOm = tl::str_to_var<double>(m_mapCache[m_strSampleTheta].strVal) /180.*M_PI;
		double dTh_aux = tl::str_to_var<double>(m_mapCache[m_strSampleTheta_aux].strVal) /180.*M_PI;
		double dPsi = tl::str_to_var<double>(m_mapCache[m_strSamplePsi0].strVal) /180.*M_PI;

		const std::string& strSthAlias = m_mapCache[m_strSampleTheta_aux_alias].strVal;

		// angle from ki to bragg peak at orient1
		triag.dAngleKiVec0 = -dOm-dPsi;

		// if the rotation sample stick is used, sth is an additional angle,
		// otherwise sth copies the om value.
		if(tl::get_py_string(strSthAlias) != "om")
			triag.dAngleKiVec0 -= dTh_aux;

		triag.bChangedAngleKiVec0 = 1;
		//std::cout << "rotation: " << triag.dAngleKiVec0 << std::endl;
	}

	emit vars_changed(crys, triag);
}

#include "nicos.moc"
