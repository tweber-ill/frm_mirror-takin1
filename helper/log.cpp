/**
 * Simple logger
 * @autor tweber
 * @date 12-sep-2014
 */

#include "log.h"
#include <chrono>
#include <thread>

std::recursive_mutex Log::s_mtx;

std::string Log::get_timestamp()
{
	using std::chrono::system_clock;

	system_clock::time_point now = system_clock::now();
	std::time_t tm = system_clock::to_time_t(now);
	std::tm tmNow = *std::localtime(&tm);

	char cTime[64];
	std::strftime(cTime, sizeof cTime, "%Y-%b-%d %H:%M:%S", &tmNow);
	return std::string(cTime);
}

std::string Log::get_thread_id()
{
	std::ostringstream ostr;
	ostr << std::hex << std::this_thread::get_id();
	return ostr.str();
}

std::string Log::get_color(LogColor col, bool bBold)
{
	switch(col)
	{
		case LogColor::RED: return bBold ? "\033[1;31m" : "\033[0;31m";
		case LogColor::YELLOW: return bBold ? "\033[1;33m" : "\033[0;33m";
		case LogColor::BLUE: return bBold ? "\033[1;34m" : "\033[0;34m";
		case LogColor::GREEN: return bBold ? "\033[1;32m" : "\033[0;32m";
		case LogColor::PURPLE: return bBold ? "\033[1;35m" : "\033[0;35m";
		case LogColor::CYAN: return bBold ? "\033[1;36m" : "\033[0;36m";
		case LogColor::WHITE: return bBold ? "\033[1;37m" : "\033[0;37m";
		case LogColor::BLACK: return bBold ? "\033[1;30m" : "\033[0;30m";
		case LogColor::NONE: 
		default: return "\033[0m";
	}
}

void Log::begin_log()
{
	s_mtx.lock();
	for(std::ostream* pOstr : m_vecOstrs)
	{
		(*pOstr) << get_color(m_col, 1);
		if(m_bShowDate)
			(*pOstr) << get_timestamp() << ", ";
		if(m_bShowThread)
			(*pOstr) << get_thread_id() << ", ";
		(*pOstr) << m_strInfo;
		(*pOstr) << ": " << get_color(m_col, 0);
	}
}

void Log::end_log()
{
	for(std::ostream* pOstr : m_vecOstrs)
	{
		(*pOstr) << get_color(LogColor::NONE);
		(*pOstr) << std::endl;
	}
	s_mtx.unlock();
}

void Log::inc_depth()
{
	s_mtx.lock();
	if(m_iDepth++ == 0)
	{
		begin_log();
	}
	s_mtx.unlock();
}

void Log::dec_depth()
{
	s_mtx.lock();
	if(--m_iDepth <= 0)
	{
		m_iDepth = 0;
		end_log();
	}
	s_mtx.unlock();
}

Log::Log() : m_vecOstrs{&std::cerr}
{}

Log::Log(const std::string& strInfo, LogColor col)
	: m_vecOstrs{&std::cerr}, 
	  m_strInfo(strInfo), m_col(col)
{}

Log::~Log()
{}

void Log::AddOstr(std::ostream* pOstr)
{
	m_vecOstrs.push_back(pOstr);
}




Log log_info("INFO", LogColor::WHITE),
	log_warn("WARNING", LogColor::YELLOW), 
	log_err("ERROR", LogColor::RED),
	log_crit("CRITICAL", LogColor::PURPLE),
	log_debug("DEBUG", LogColor::CYAN);




/*
#include <thread>

int main()
{
	log_info.template operator()<std::string>("Start");
	log_info("Test");

	std::thread th1([&]{ for(int i=0; i<10; ++i) {log_err("In thread 1."); std::this_thread::sleep_for(std::chrono::milliseconds(10));} });
	std::thread th2([&]{ for(int i=0; i<10; ++i) {log_warn("In thread 2."); std::this_thread::sleep_for(std::chrono::milliseconds(100));} });
	std::thread th3([&]{ for(int i=0; i<10; ++i) {log_crit("In thread 3."); std::this_thread::sleep_for(std::chrono::milliseconds(250));} });

	log_info(1,2,3);
	log_info("a", "b", "x");

	for(int i=0; i<10; ++i)
		log_info("In main.");

	th1.join();
	th2.join();
	th3.join();
	return 0;
}
*/
