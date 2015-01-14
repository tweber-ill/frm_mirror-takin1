/*
 * Connection to Nicos
 * @author tweber
 * @date 27-aug-2014
 * @copyright GPLv2
 */

#ifndef __NICOS_H__
#define __NICOS_H__

#include "tlibs/net/tcp.h"
#include "tasoptions.h"

#include "../../dialogs/NetCacheDlg.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QSettings>

#include <map>
#include <vector>
#include <string>

class NicosCache : public QObject
{ Q_OBJECT
	protected:
		QSettings* m_pSettings = 0;

		tl::TcpClient m_tcp;
		std::vector<std::string> m_vecKeys;
		t_mapCacheVal m_mapCache;

		// endpoints of the TcpClient signals
		void slot_connected(const std::string& strHost, const std::string& strSrv);
		void slot_disconnected(const std::string& strHost, const std::string& strSrv);
		void slot_receive(const std::string& str);

	public:
		NicosCache(QSettings* pSettings=0);
		virtual ~NicosCache();

		void connect(const std::string& strHost, const std::string& strPort);
		void disconnect();

		void AddKey(const std::string& strKey);
		void ClearKeys();

		void RefreshKeys();
		void RegisterKeys();

	signals:
		void connected(const QString& strHost, const QString& strSrv);
		void disconnected();

		void vars_changed(const CrystalOptions& crys, const TriangleOptions& triag);

		void updated_cache_value(const std::string& strKey, const CacheVal& val);
		void cleared_cache();

	protected:
		// Nicos device names
		std::string m_strSampleName;
		std::string m_strSampleLattice, m_strSampleAngles;
		std::string m_strSampleOrient1, m_strSampleOrient2;
		std::string m_strSampleSpacegroup;
		std::string m_strSamplePsi0, m_strSampleTheta, m_strSample2Theta;
		std::string m_strMonoTheta, m_strMono2Theta, m_strMonoD;
		std::string m_strAnaTheta, m_strAna2Theta, m_strAnaD;

		// rotation sample stick: sth != om, otherwise: sth == om
		std::string m_strSampleTheta_aux, m_strSampleTheta_aux_alias;
};

#endif
