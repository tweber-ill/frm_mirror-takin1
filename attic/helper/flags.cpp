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

	std::lock_guard<std::mutex> _lck(s_mutex);
	FILE* p = ::popen(pcCmd, pcType);

	return p;
}


int my_pclose(void *pPipe)
{
	return ::pclose((FILE*)pPipe);
}
