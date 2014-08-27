/*
 * clang -o tst_client tst_client.cpp ../helper/tcp.cpp -lstdc++ -std=c++11 -lboost_system -lpthread
 */

#include <iostream>
#include <fstream>
#include "../helper/tcp.h"


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

void disconnected(const std::string& strHost, const std::string& strSrv)
{
	std::cout << "Disonnected from " << strHost << " on port " << strSrv << "." << std::endl;
}

void connected(const std::string& strHost, const std::string& strSrv)
{
	std::cout << "Connected to " << strHost << " on port " << strSrv << "." << std::endl;
}


int main()
{
	TcpClient client;
	TstOut tstout;
	client.add_receiver(boost::bind(&TstOut::print, &tstout, _1));
	client.add_disconnect(disconnected);
	client.add_connect(connected);


	if(!client.connect("mira1.mira.frm2", "14869"))
		return -1;

	while(client.is_connected())
	{
		std::string strMsg;
		std::cout << "in: ";
		std::getline(std::cin, strMsg);
		strMsg+="\n";
		client.write(strMsg);
	};

	return 0;
}
