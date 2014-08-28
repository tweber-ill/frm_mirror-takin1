/*
 * Connection to Nicos
 * @author tweber
 * @date 27-aug-2014
 */

#include "nicos.h"
#include "helper/string.h"
#include <chrono>


// TODO: move to config file
static const std::string g_strSampleName = "nicos/sample/samplename";
static const std::string g_strSampleLattice = "nicos/sample/lattice";
static const std::string g_strSampleAngles = "nicos/sample/angles";
static const std::string g_strSampleOrient1 = "nicos/sample/orient1";
static const std::string g_strSampleOrient2 = "nicos/sample/orient2";
static const std::string g_strSampleSpacegroup = "nicos/sample/spacegroup";
//static const std::string g_strSamplePsi0 = "nicos/sample/psi0";

static const std::string g_strMonoTheta = "nicos/m2th/value";
static const std::string g_strMono2Theta = "nicos/m2tt/value";
static const std::string g_strMonoD = "nicos/mono/dvalue";

static const std::string g_strAnaTheta = "nicos/ath/value";
static const std::string g_strAna2Theta = "nicos/att/value";
static const std::string g_strAnaD = "nicos/ana/dvalue";

static const std::string g_strSampleTheta = "nicos/om/value";
static const std::string g_strSample2Theta = "nicos/phi/value";



NicosCache::NicosCache()
	: m_vecKeys({	g_strSampleName,
					g_strSampleLattice,
					g_strSampleAngles,
					g_strSampleOrient1,
					g_strSampleOrient2,
					g_strSampleSpacegroup,

					g_strMonoD,
					//g_strMonoTheta,
					g_strMono2Theta,

					g_strAnaD,
					//g_strAnaTheta,
					g_strAna2Theta,

					g_strSampleTheta,
					g_strSample2Theta,
			})
{
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
	for(const std::string& strKey : m_vecKeys)
		m_tcp.write("@"+strKey+"?\n");
}

void NicosCache::RegisterKeys()
{
	for(const std::string& strKey : m_vecKeys)
		m_tcp.write("@"+strKey+":\n");
}


void NicosCache::slot_connected(const std::string& strHost, const std::string& strSrv)
{
	std::cout << "Connected to " << strHost << " on port " << strSrv << "." << std::endl;

	QString qstrHost = strHost.c_str();
	QString qstrSrv = strSrv.c_str();
	emit connected(qstrHost, qstrSrv);

	RegisterKeys();
	RefreshKeys();
}

void NicosCache::slot_disconnected(const std::string& strHost, const std::string& strSrv)
{
	std::cout << "Disconnected from " << strHost << " on port " << strSrv << "." << std::endl;
	emit disconnected();
}


static std::vector<double> get_py_array(const std::string& str)
{
	std::vector<double> vecArr;

	std::size_t iStart = str.find('[');
	std::size_t iEnd = str.find(']');

	// invalid array
	if(iStart==std::string::npos || iEnd==std::string::npos || iEnd<iStart)
		return vecArr;

	std::string strArr = str.substr(iStart+1, iEnd-iStart-1);
	::get_tokens<double, std::string>(strArr, ",", vecArr);

	return vecArr;
}

static std::string get_py_string(const std::string& str)
{
	std::size_t iStart = str.find_first_of("\'\"");
	std::size_t iEnd = str.find_last_of("\'\"");

	// invalid string
	if(iStart==std::string::npos || iEnd==std::string::npos || iEnd<iStart)
		return "";

	return str.substr(iStart+1, iEnd-iStart-1);
}

void NicosCache::slot_receive(const std::string& str)
{
	//std::cout << "received:" << str << std::endl;
	std::pair<std::string, std::string> pairTimeVal = ::split_first<std::string>(str, "@", 1);
	std::pair<std::string, std::string> pairKeyVal = ::split_first<std::string>(pairTimeVal.second, "=", 1);

	//const std::string& strTime = pairTimeVal.first;
	const std::string& strKey = pairKeyVal.first;
	const std::string& strVal = pairKeyVal.second;


	/*
	double dTimestamp = str_to_var<double>(strTime);
	double dNow = std::chrono::system_clock::now().time_since_epoch().count();
	dNow *= double(std::chrono::system_clock::period::num) / double(std::chrono::system_clock::period::den);
	double dAge = dNow - dTimestamp;

	std::cout << "age: " << dAge << " s" << "\n";
	std::cout << "key: " << strKey << "\n";
	std::cout << "val: " << strVal << std::endl;
	*/



	if(strVal.length() == 0)
		return;

	CrystalOptions crys;
	TriangleOptions triag;

	if(strKey == g_strSampleLattice)
	{
		std::vector<double> vecLattice = get_py_array(strVal);
		if(vecLattice.size() != 3)
			return;

		crys.bChangedLattice = 1;
		for(int i=0; i<3; ++i)
			crys.dLattice[i] = vecLattice[i];
	}
	else if(strKey == g_strSampleAngles)
	{
		std::vector<double> vecAngles = get_py_array(strVal);
		if(vecAngles.size() != 3)
			return;

		crys.bChangedLatticeAngles = 1;
		for(int i=0; i<3; ++i)
			crys.dLatticeAngles[i] = vecAngles[i];
	}
	else if(strKey == g_strSampleOrient1)
	{
		std::vector<double> vecOrient = get_py_array(strVal);
		if(vecOrient.size() != 3)
			return;

		crys.bChangedPlane1 = 1;
		for(int i=0; i<3; ++i)
			crys.dPlane1[i] = vecOrient[i];
	}
	else if(strKey == g_strSampleOrient2)
	{
		std::vector<double> vecOrient = get_py_array(strVal);
		if(vecOrient.size() != 3)
			return;

		crys.bChangedPlane2 = 1;
		for(int i=0; i<3; ++i)
			crys.dPlane2[i] = vecOrient[i];
	}
	else if(strKey == g_strSampleSpacegroup)
	{
		crys.strSpacegroup = get_py_string(strVal);
		crys.bChangedSpacegroup = 1;
	}
	else if(strKey == g_strMono2Theta)
	{
		triag.dMonoTwoTheta = str_to_var<double>(strVal) /180.*M_PI;
		triag.bChangedMonoTwoTheta = 1;
	}
	else if(strKey == g_strAna2Theta)
	{
		triag.dAnaTwoTheta = str_to_var<double>(strVal) /180.*M_PI;
		triag.bChangedAnaTwoTheta = 1;
	}
	else if(strKey == g_strSample2Theta)
	{
		triag.dTwoTheta = str_to_var<double>(strVal) /180.*M_PI;
		triag.bChangedTwoTheta = 1;
	}
	else if(strKey == g_strSampleTheta)
	{
		triag.dTheta = str_to_var<double>(strVal) /180.*M_PI;
		//std::cout << triag.dTheta << std::endl;
		triag.bChangedTheta = 1;
	}
	else if(strKey == g_strMonoD)
	{
		triag.dMonoD = str_to_var<double>(strVal);
		triag.bChangedMonoD = 1;
	}
	else if(strKey == g_strAnaD)
	{
		triag.dAnaD = str_to_var<double>(strVal);
		triag.bChangedAnaD = 1;
	}

	emit vars_changed(crys, triag);
}

#include "nicos.moc"
