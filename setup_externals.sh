#!/bin/bash


FINDQWT=http://cmake.org/Wiki/images/2/27/FindQwt.cmake
TANGOICONS=http://tango.freedesktop.org/releases/tango-icon-theme-0.8.90.tar.gz


function dl_findqwt
{
#	rm -f FindQwt.cmake

	if [ ! -f FindQwt.cmake ]; then
		echo -e "Downloading FindQwt...\n"

		if ! wget ${FINDQWT}; then
			echo -e "Error: Cannot download FindQwt.";
			exit -1;
		fi
	fi
}


function dl_tangoicons
{
#	rm -f tango-icon-theme.tar.gz

	if [ ! -f tango-icon-theme.tar.gz  ]; then
		echo -e "Downloading Tango icons...\n"

		if ! wget ${TANGOICONS} -O tango-icon-theme.tar.gz; then
			echo -e "Error: Cannot download Tango icons.";
			exit -1;
		fi
	fi


	echo -e "Extracting Tango icons...\n"

	tar --wildcards -xzvf tango-icon-theme.tar.gz */scalable/actions/document-save.svg --strip-components=3
	tar --wildcards -xzvf tango-icon-theme.tar.gz */scalable/actions/document-save-as.svg --strip-components=3
	tar --wildcards -xzvf tango-icon-theme.tar.gz */scalable/actions/document-open.svg --strip-components=3
	tar --wildcards -xzvf tango-icon-theme.tar.gz */scalable/actions/system-log-out.svg --strip-components=3
	tar --wildcards -xzvf tango-icon-theme.tar.gz */scalable/actions/view-refresh.svg --strip-components=3
	tar --wildcards -xzvf tango-icon-theme.tar.gz */scalable/categories/preferences-system.svg --strip-components=3
	tar --wildcards -xzvf tango-icon-theme.tar.gz */scalable/devices/media-floppy.svg --strip-components=3
	tar --wildcards -xzvf tango-icon-theme.tar.gz */scalable/devices/drive-harddisk.svg --strip-components=3
	tar --wildcards -xzvf tango-icon-theme.tar.gz */scalable/devices/network-wireless.svg --strip-components=3
	tar --wildcards -xzvf tango-icon-theme.tar.gz */scalable/mimetypes/image-x-generic.svg --strip-components=3
	tar --wildcards -xzvf tango-icon-theme.tar.gz */scalable/apps/accessories-calculator.svg --strip-components=3
	tar --wildcards -xzvf tango-icon-theme.tar.gz */scalable/status/network-transmit-receive.svg --strip-components=3
	tar --wildcards -xzvf tango-icon-theme.tar.gz */scalable/status/network-offline.svg --strip-components=3
	tar --wildcards -xzvf tango-icon-theme.tar.gz */scalable/status/dialog-information.svg --strip-components=3

	mv -v *.svg res/
}


echo -e "--------------------------------------------------------------------------------"
dl_findqwt
echo -e "--------------------------------------------------------------------------------"
dl_tangoicons
echo -e "--------------------------------------------------------------------------------"
