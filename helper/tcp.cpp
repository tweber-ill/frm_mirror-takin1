/*
 * TcpClient
 * @author tweber
 * @date aug-2014
 * based on chat client by M. Kohlhoff:
 * 	www.boost.org/doc/libs/1_56_0/doc/html/boost_asio/example/cpp11/chat/chat_client.cpp
 */

#include "tcp.h"
#include <boost/tokenizer.hpp>
#include <iostream>


static inline bool get_cmd_tokens(const std::string& str, const std::string& strDelim, 
				std::vector<std::string>& vecStr, std::string& strRemainder)
{
        boost::char_separator<char> delim(strDelim.c_str(), "", boost::keep_empty_tokens);
        boost::tokenizer<boost::char_separator<char> > tok(str, delim);

        for(const std::string& strTok : tok)
	{
		//std::cout << "tok: " << strTok << std::endl;
		vecStr.push_back(strTok);
	}

	if(vecStr.size()<=1)
		return false;

	// keep_empty_tokens leads to an empty last element if no remaining string is left
	if(*vecStr.rbegin() == "")
	{
		strRemainder = "";
		vecStr.pop_back();
	}
	else
	{
		strRemainder = *vecStr.rbegin();
		vecStr.pop_back();
	}

	//std::cout << "toks: " << vecStr.size() << std::endl;
	//std::cout << "remainder: " << strRemainder << std::endl;
	return true;
}


TcpClient::TcpClient()
{}

TcpClient::~TcpClient()
{
	disconnect();

	m_sigRecv.disconnect_all_slots();
	m_sigDisconn.disconnect_all_slots();
	m_sigConn.disconnect_all_slots();
}

bool TcpClient::connect(const std::string& strHost, const std::string& strService)
{
	m_strHost = strHost;
	m_strService = strService;

	try
	{
		disconnect();

		m_pservice = new asio::io_service;
		m_psock = new ip::tcp::socket(*m_pservice);

		ip::tcp::resolver res(*m_pservice);
		ip::tcp::resolver::iterator iter = res.resolve({strHost, strService});
		asio::async_connect(*m_psock, iter,
		[&](const sys::error_code& err, ip::tcp::resolver::iterator) 
		{
			if(!err)
				read_loop();

			m_sigConn(m_strHost, m_strService);
		});

		m_pthread = new std::thread([&]() { m_pservice->run(); });
	}
	catch(const std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 0;
	}

	return 1;
}

void TcpClient::disconnect()
{
	bool bConnected = is_connected();
	if(bConnected)
	{
		m_pservice->stop();
		m_psock->close();
	}

	if(m_psock)
	{
		delete m_psock;
		m_psock = 0;
	}

	if(m_pthread)
	{
		m_pthread->join();
		delete m_pthread;
		m_pthread = 0;
	}

	if(m_pservice)
	{
		delete m_pservice;
		m_pservice = 0;
	}


	if(bConnected)
	{
		m_sigDisconn(m_strHost, m_strService);

		m_strHost = "";
		m_strService = "";
	}
}

bool TcpClient::is_connected()
{
	if(!m_psock) return 0;
	return m_psock->is_open();
}

void TcpClient::write(const std::string& str)
{
	//m_mutexWrite.lock();
	//std::cout << "push_back: " << str << std::endl;
	m_listWriteBuffer.push_back(str);
	//m_mutexWrite.unlock();
	m_pservice->post([&](){ flush_write(); });
}

void TcpClient::flush_write()
{
	//m_mutexWrite.lock();
	if(m_listWriteBuffer.empty())
	{
		//m_mutexWrite.unlock();
		return;
	}

	const std::string& str = m_listWriteBuffer.front();
	//m_mutexWrite.unlock();

	//std::cout << "str = " << str << std::endl;
	asio::async_write(*m_psock, asio::buffer(str.data(), str.length()),
	[&](const sys::error_code& err, std::size_t len)
	{
		if(err)
		{
			disconnect();
			return;
		}

		int iBufSize = m_listWriteBuffer.size();
		if(iBufSize != 0)
		{
			//m_mutexWrite.lock();
			m_listWriteBuffer.pop_front();
			//m_mutexWrite.unlock();

			if(iBufSize != 0)
				flush_write();
		}
	});
}

void TcpClient::read_loop()
{
	static const std::size_t iBufLen = 256;
	static char pcBuf[iBufLen];

	asio::async_read(*m_psock, asio::buffer(pcBuf, iBufLen), asio::transfer_at_least(1),
	[&](const sys::error_code& err, std::size_t len)
	{
		if(err)
		{
			//std::cerr << "Read error." << std::endl;
			disconnect();
			return;
		}

		std::string strCurMsg(pcBuf, len);
		m_strReadBuffer.append(strCurMsg);

		//std::cout << "read buffer: " << m_strReadBuffer << std::endl;
		std::vector<std::string> vecCmds;
		if(get_cmd_tokens(m_strReadBuffer, m_strCmdDelim, vecCmds, m_strReadBuffer))
		{
			//std::cout << "remainder: " << m_strReadBuffer << std::endl;

			for(const std::string& strCmd : vecCmds)
			{
				//std::cout << "msg: " << strCmd << std::endl;
				m_sigRecv(strCmd);
			}
		}

		read_loop();
	});
  }


// --------------------------------------------------------------------------------
// Signals
void TcpClient::add_receiver(const t_sigRecv::slot_type& conn)
{
	m_sigRecv.connect(conn);
}

void TcpClient::add_disconnect(const t_sigDisconn::slot_type& disconn)
{
	m_sigDisconn.connect(disconn);
}

void TcpClient::add_connect(const t_sigConn::slot_type& conn)
{
	m_sigConn.connect(conn);
}
// --------------------------------------------------------------------------------
