/*
 * clang -o tst_client tst_client.cpp ../../tlibs/net/tcp.cpp ../../tlibs/helper/log.cpp -lstdc++ -std=c++11 -lboost_system -lpthread
 */

#include <iostream>
#include <fstream>
#include "../../tlibs/net/tcp.h"

using namespace tl;

struct TstOut
{
	std::ofstream ofstr;

	TstOut() : ofstr("tst_out.txt")
	{}

	void print(const std::string& str)
	{
		//ofstr << str << std::endl;
		std::cout << "out: " << str << std::endl;
	}
};

static void disconnected(const std::string& strHost, const std::string& strSrv)
{
	std::cout << "Disconnected from " << strHost << " on port " << strSrv << "." << std::endl;
}

static void connected(const std::string& strHost, const std::string& strSrv)
{
	std::cout << "Connected to " << strHost << " on port " << strSrv << "." << std::endl;
}

static void received(const std::string& strMsg)
{
	std::cout << strMsg << std::endl;
}


int main(int argc, char** argv)
{
	if(argc < 3)
	{
		std::cerr << "Usage: " << argv[0] << " <server> <port>" << std::endl;
		return -1;
	}


	TcpClient client;
	TstOut tstout;
	client.add_receiver(boost::bind(&TstOut::print, &tstout, _1));
	//client.add_receiver(received);
	client.add_disconnect(disconnected);
	client.add_connect(connected);


	if(!client.connect(argv[1], argv[2]))
	{
		std::cerr << "Error: Cannot connect." << std::endl;
		return -1;
	}

	std::string strMsg;
	while(client.is_connected())
	{
		//std::cout << "in: ";

		std::getline(std::cin, strMsg);
		if(strMsg == "!exit!")
			break;

		strMsg+="\n";
		client.write(strMsg);
	}

	return 0;
}
