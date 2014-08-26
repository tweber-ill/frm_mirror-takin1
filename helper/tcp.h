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

#include <boost/asio.hpp>
#include <boost/signals2.hpp>


namespace sys = boost::system;
namespace asio = boost::asio;
namespace ip = boost::asio::ip;
namespace sig = boost::signals2;


class TcpClient
{
	protected:
		asio::io_service m_service;
		ip::tcp::socket m_sock;
		std::thread* m_pthread = 0;

		std::string m_strCmdDelim = "\n";
		std::list<std::string> m_listWriteBuffer;
		std::string m_strReadBuffer;

		typedef sig::signal<void(const std::string&)> t_sigRecv;
		t_sigRecv m_sigRecv;

	public:
		TcpClient();
		virtual ~TcpClient();
		void set_delim(const char* pcDelim) { m_strCmdDelim = pcDelim; }

		void add_receiver(const t_sigRecv::slot_type& conn);

		bool connect(const char* pcHost, const char* pcService);
		void disconnect();

		void write(const std::string& str);

	protected:
		void flush_write();
		void read_loop();
};

#endif
