find_path(tlibs_INCLUDE_DIRS
	NAMES version.h
	PATH_SUFFIXES tlibs
	HINTS /usr/include/tlibs /usr/local/include/tlibs
	DOC "tlibs include directories"
)


find_library(tlibs_LIBRARIES
	NAMES libtlibs.so
	HINTS /usr/lib64/ /usr/lib/ /usr/local/lib64 /usr/local/lib
	DOC "tlibs library"
)


message("tLibs include directories: ${tlibs_INCLUDE_DIRS}")
message("tLibs library: ${tlibs_LIBRARIES}")
