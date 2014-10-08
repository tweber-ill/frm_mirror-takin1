/**
 * Simple logger
 * @autor tweber
 * @date 12-sep-2014
 */

#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <vector>
#include <iostream>
#include <sstream>
#include <mutex>

enum class LogColor
{
	NONE,

	RED,
	BLUE,
	GREEN,
	YELLOW,
	PURPLE,
	CYAN,
	WHITE,
	BLACK
};

class Log
{
private:
	int m_iDepth = 0;

protected:
	static std::recursive_mutex s_mtx;
	std::vector<std::ostream*> m_vecOstrs;

	std::string m_strInfo;
	LogColor m_col = LogColor::NONE;

	bool m_bEnabled = 1;
	bool m_bShowDate = 1;
	bool m_bShowThread = 0;

protected:
	static std::string get_timestamp();
	static std::string get_thread_id();
	static std::string get_color(LogColor col, bool bBold=0);

	void begin_log();
	void end_log();

	void inc_depth();
	void dec_depth();

public:
	Log();
	Log(const std::string& strInfo, LogColor col);
	virtual ~Log();

	void AddOstr(std::ostream* pOstr);

	template<typename Arg>
	void operator()(const Arg& arg)
	{
		if(!m_bEnabled) return;

		inc_depth();

		for(std::ostream* pOstr : m_vecOstrs)
			(*pOstr) << arg;

		dec_depth();
	}

	template<typename Arg, typename... Args>
	void operator()(const Arg& arg, Args... args)
	{
		if(!m_bEnabled) return;

		inc_depth();

		(*this)(arg);
		(*this)(args...);

		dec_depth();
	}

	void SetEnabled(bool bEnab) { m_bEnabled = bEnab; }
	void SetShowDate(bool bDate) { m_bShowDate = bDate; }
	void SetShowThread(bool bThread) { m_bShowThread = bThread; }
};


extern Log log_info, log_warn, log_err, log_crit, log_debug;
extern void log_backtrace();

#endif
