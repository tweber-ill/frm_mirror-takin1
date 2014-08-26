/*
 * TcpClient
 * @author tweber
 * @date aug-2014
 * based on chat client by M. Kohlhoff:
 * 	www.boost.org/doc/libs/1_56_0/doc/html/boost_asio/example/cpp11/chat/chat_client.cpp
 *
 * clang -o tst_client tst_client.cpp -lstdc++ -std=c++11 -lboost_system -lpthread
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


TcpClient::TcpClient() : m_sock(m_service)
{}

TcpClient::~TcpClient()
{
	disconnect();
}

void TcpClient::add_receiver(const t_sigRecv::slot_type& conn)
{
	m_sigRecv.connect(conn);
}

bool TcpClient::connect(const char* pcHost, const char* pcService)
{
	try
	{
		ip::tcp::resolver res(m_service);
		ip::tcp::resolver::iterator iter = res.resolve({pcHost, pcService});
		asio::async_connect(m_sock, iter,
		[&](const sys::error_code& err, ip::tcp::resolver::iterator) 
		{
			if(!err)
				read_loop();
		});

		m_pthread = new std::thread([&]() { m_service.run(); });
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
	m_sock.close();

	if(m_pthread)
	{
		m_pthread->join();
		delete m_pthread;
		m_pthread = 0;
	}
}


void TcpClient::write(const std::string& str)
{
	m_listWriteBuffer.push_back(str);
	m_service.post([&](){ flush_write(); });
}

void TcpClient::flush_write()
{
	if(m_listWriteBuffer.empty())
		return;

	const std::string& str = m_listWriteBuffer.front();
	//std::cout << "str = " << str << std::endl;
	asio::async_write(m_sock, asio::buffer(str.data(), str.length()),
	[&](const sys::error_code& err, std::size_t len)
	{
		if(err)
		{
			m_sock.close();
			return;
		}

		m_listWriteBuffer.pop_front();
		flush_write();
	});
}

void TcpClient::read_loop()
{
	static const std::size_t iBufLen = 256;
	static char pcBuf[iBufLen];

	asio::async_read(m_sock, asio::buffer(pcBuf, iBufLen), asio::transfer_at_least(1),
	[&](const sys::error_code& err, std::size_t len)
	{
		if(err)
		{
			std::cerr << "Read error." << std::endl;
			m_sock.close();
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
