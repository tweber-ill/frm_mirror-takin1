#!/bin/bash


FINDQWT=http://cmake.org/Wiki/images/2/27/FindQwt.cmake


function dl_findqwt
{
	echo -e "Downloading FindQwt...\n"

	rm -f FindQwt.cmake
	if ! wget ${FINDQWT}
	then
		echo -e "Error: Cannot download FindQwt.";
		exit -1;
	fi
}


echo -e "--------------------------------------------------------------------------------"
dl_findqwt
echo -e "--------------------------------------------------------------------------------"
