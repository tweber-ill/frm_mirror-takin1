/*
 * Compiler- and system-specific stuff
 * @author tweber
 * @date 2013
 */

#ifndef __COMPILER_FLAGS_H__
#define __COMPILER_FLAGS_H__

#ifdef __CYGWIN__
        #undef __STRICT_ANSI__
#endif

// normal popen is not thread-safe on all systems
void *my_popen(const char* pcCmd, const char* pcType="w");

#endif
