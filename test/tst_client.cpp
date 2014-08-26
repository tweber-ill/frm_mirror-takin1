/*
 * clang -o tst_client tst_client.cpp ../helper/tcp.cpp -lstdc++ -std=c++11 -lboost_system -lpthread
 */

#include <iostream>
#include "../helper/tcp.h"


struct TstOut
{
	void print(const std::string& str) const
	{
		std::cout << "out: " << str << std::endl;
	}
};

/*void tst_out(const std::string& str)
{
	std::cout << str << std::endl;
}*/

int main()
{
	TcpClient client;
	//client.add_receiver(tst_out);
	TstOut tstout;
	client.add_receiver(boost::bind(&TstOut::print, &tstout, _1));

	if(!client.connect("localhost", "1701"))
		return -1;

	while(1)
	{
		std::string strMsg;
		std::cout << "in: ";
		std::getline(std::cin, strMsg);
		strMsg+="\n";
		client.write(strMsg);
	};

	return 0;
}
