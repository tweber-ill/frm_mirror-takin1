/*
 * Compiler- and system-specific stuff
 * @author tweber
 * @date jan-2014
 */

#include "flags.h"

#include <mutex>
#include <cstdio>

void *my_popen(const char* pcCmd, const char* pcType)
{
	static std::mutex s_mutex;

	s_mutex.lock();
	FILE* p = ::popen(pcCmd, pcType);
	s_mutex.unlock();

	return p;
}
