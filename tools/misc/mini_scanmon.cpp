/*
 * clang -o mini_scanmon mini_scanmon.cpp ../../tlibs/net/tcp.cpp ../../tlibs/helper/log.cpp -lstdc++ -std=c++11 -lboost_system -lpthread
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include "../../tlibs/net/tcp.h"
#include "../../tlibs/helper/log.h"
#include "../../tlibs/string/string.h"

using namespace tl;


std::string strCtr = "nicos/ctr1/value";
std::string strTim = "nicos/timer/value";
std::string strSel = "nicos/timer/preselection";

typedef float t_real;

t_real dCtr = 0.;
t_real dTim = 0.;
t_real dSel = 0.;

void refresh()
{
	t_real dProgress = 0.;
	t_real dExpCtr = 0.;

	if(!float_equal(dSel, t_real(0.)))
	{
		dProgress = dTim / dSel;
		dExpCtr = dCtr / dProgress;
	}

	std::cout.precision(2);
	std::cout << "\r"
		<< "Counts: " /*<< std::setw(8)*/ << std::fixed << dCtr
		<< ", Expected: " << std::fixed << dExpCtr
		<< ", Scan progress: " << std::fixed << dProgress*100. << " % ("
		<< std::fixed << dTim << " s of " << dSel << " s)."
		<< "        ";
	std::cout.flush();
}


void disconnected(const std::string& strHost, const std::string& strSrv)
{
	log_info("Disconnected from ", strHost, " on port ", strSrv, ".");;
}

void connected(const std::string& strHost, const std::string& strSrv)
{
	log_info("Connected to ", strHost, " on port ", strSrv, ".");
}

void received(const std::string& strMsg)
{
	//log_info("Received: ", strMsg);

	std::pair<std::string, std::string> pair = split_first<std::string>(strMsg, "=", true);

	if(pair.first == strTim)
		dTim = str_to_var<t_real>(pair.second);
	else if(pair.first == strCtr)
		dCtr = str_to_var<t_real>(pair.second);
	else if(pair.first == strSel)
		dSel = str_to_var<t_real>(pair.second);

	refresh();
}


int main(int argc, char** argv)
{
	if(argc < 3)
	{
		std::cerr << "Usage: " << argv[0] << " <server> <port>" << std::endl;
		std::cerr << "\te.g.: " << argv[0] << " mira1.mira.frm2 14869" << std::endl;
		return -1;
	}


	TcpClient client;
	client.add_receiver(received);
	client.add_disconnect(disconnected);
	client.add_connect(connected);


	if(!client.connect(argv[1], argv[2]))
	{
		log_err("Error: Cannot connect.");
		return -1;
	}


	//std::this_thread::sleep_for(std::chrono::milliseconds(100));

	for(const std::string& strKey : {strSel, strTim, strCtr})
	{
		client.write(strKey + "?\n");
		client.write(strKey + ":\n");
	}

	client.wait();
	return 0;
}
