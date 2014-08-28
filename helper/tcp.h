/*
 * TcpClient
 * @author tweber
 * @date aug-2014
 * based on chat client by M. Kohlhoff:
 * 	www.boost.org/doc/libs/1_56_0/doc/html/boost_asio/example/cpp11/chat/chat_client.cpp
 */


#ifndef __TCP_CLIENT__
#define __TCP_CLIENT__

#include <string>
#include <list>
#include <thread>
//#include <mutex>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>


namespace sys = boost::system;
namespace asio = boost::asio;
namespace ip = boost::asio::ip;
namespace sig = boost::signals2;


class TcpClient
{
	protected:
		std::string m_strHost, m_strService;

		asio::io_service *m_pservice = 0;
		ip::tcp::socket *m_psock = 0;
		std::thread* m_pthread = 0;
		//std::mutex m_mutexWrite;

		std::string m_strCmdDelim = "\n";
		std::list<std::string> m_listWriteBuffer;
		std::string m_strReadBuffer;

		typedef sig::signal<void(const std::string&)> t_sigRecv;
		typedef sig::signal<void(const std::string&, const std::string&)> t_sigDisconn;
		typedef sig::signal<void(const std::string&, const std::string&)> t_sigConn;

		t_sigRecv m_sigRecv;
		t_sigDisconn m_sigDisconn;
		t_sigConn m_sigConn;

	public:
		TcpClient();
		virtual ~TcpClient();
		void set_delim(const std::string& strDelim) { m_strCmdDelim = strDelim; }

		void add_receiver(const t_sigRecv::slot_type& conn);
		void add_disconnect(const t_sigDisconn::slot_type& disconn);
		void add_connect(const t_sigConn::slot_type& conn);

		bool connect(const std::string& strHost, const std::string& strService);
		void disconnect();

		void write(const std::string& str);

		bool is_connected();

	protected:
		void flush_write();
		void read_loop();
};

#endif
