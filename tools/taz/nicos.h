/*
 * Connection to Nicos
 * @author tweber
 * @date 27-aug-2014
 */

#ifndef __NICOS_H__
#define __NICOS_H__

#include "helper/tcp.h"
#include "tasoptions.h"

#include "../../dialogs/NetCacheDlg.h"

#include <QtCore/QObject>
#include <QtCore/QString>

#include <map>
#include <vector>
#include <string>

class NicosCache : public QObject
{ Q_OBJECT
	protected:
		TcpClient m_tcp;
		std::vector<std::string> m_vecKeys;
		t_mapCacheVal m_mapCache;

		// endpoints of the TcpClient signals
		void slot_connected(const std::string& strHost, const std::string& strSrv);
		void slot_disconnected(const std::string& strHost, const std::string& strSrv);
		void slot_receive(const std::string& str);

	public:
		NicosCache();
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
};

#endif
