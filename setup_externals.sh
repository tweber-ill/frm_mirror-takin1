#!/bin/bash

GTAR="$(which gnutar)"
if [ $? -ne 0 ]; then
	GTAR="$(which tar)"
fi

FINDQWT=http://cmake.org/Wiki/images/2/27/FindQwt.cmake
TANGOICONS=http://tango.freedesktop.org/releases/tango-icon-theme-0.8.90.tar.gz
SCATLENS=https://www.ncnr.nist.gov/resources/n-lengths/list.html


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
#	rm -f tmp/tango-icon-theme.tar.gz

	if [ ! -f tmp/tango-icon-theme.tar.gz  ]; then
		echo -e "Downloading Tango icons...\n"

		if ! wget ${TANGOICONS} -O tmp/tango-icon-theme.tar.gz; then
			echo -e "Error: Cannot download Tango icons.";
			exit -1;
		fi
	fi


	echo -e "Extracting Tango icons...\n"
	cd tmp
	${GTAR} --wildcards -xzvf tango-icon-theme.tar.gz */scalable/actions/document-save.svg --strip-components=3
	${GTAR} --wildcards -xzvf tango-icon-theme.tar.gz */scalable/actions/document-save-as.svg --strip-components=3
	${GTAR} --wildcards -xzvf tango-icon-theme.tar.gz */scalable/actions/document-open.svg --strip-components=3
	${GTAR} --wildcards -xzvf tango-icon-theme.tar.gz */scalable/actions/document-new.svg --strip-components=3
	${GTAR} --wildcards -xzvf tango-icon-theme.tar.gz */scalable/actions/list-add.svg --strip-components=3
	${GTAR} --wildcards -xzvf tango-icon-theme.tar.gz */scalable/actions/list-remove.svg --strip-components=3
	${GTAR} --wildcards -xzvf tango-icon-theme.tar.gz */scalable/actions/system-log-out.svg --strip-components=3
	${GTAR} --wildcards -xzvf tango-icon-theme.tar.gz */scalable/actions/view-refresh.svg --strip-components=3
	${GTAR} --wildcards -xzvf tango-icon-theme.tar.gz */scalable/actions/media-playback-start.svg --strip-components=3
	${GTAR} --wildcards -xzvf tango-icon-theme.tar.gz */scalable/actions/media-playback-stop.svg --strip-components=3
	${GTAR} --wildcards -xzvf tango-icon-theme.tar.gz */scalable/categories/preferences-system.svg --strip-components=3
	${GTAR} --wildcards -xzvf tango-icon-theme.tar.gz */scalable/devices/media-floppy.svg --strip-components=3
	${GTAR} --wildcards -xzvf tango-icon-theme.tar.gz */scalable/devices/drive-harddisk.svg --strip-components=3
	${GTAR} --wildcards -xzvf tango-icon-theme.tar.gz */scalable/devices/network-wireless.svg --strip-components=3
	${GTAR} --wildcards -xzvf tango-icon-theme.tar.gz */scalable/mimetypes/image-x-generic.svg --strip-components=3
	${GTAR} --wildcards -xzvf tango-icon-theme.tar.gz */scalable/mimetypes/x-office-spreadsheet-template.svg --strip-components=3
	${GTAR} --wildcards -xzvf tango-icon-theme.tar.gz */scalable/apps/accessories-calculator.svg --strip-components=3
	${GTAR} --wildcards -xzvf tango-icon-theme.tar.gz */scalable/status/network-transmit-receive.svg --strip-components=3
	${GTAR} --wildcards -xzvf tango-icon-theme.tar.gz */scalable/status/network-offline.svg --strip-components=3
	${GTAR} --wildcards -xzvf tango-icon-theme.tar.gz */scalable/status/dialog-information.svg --strip-components=3
	${GTAR} --wildcards -xzvf tango-icon-theme.tar.gz */scalable/status/weather-snow.svg --strip-components=3
	cd ..
	mv -v tmp/*.svg res/
}

function dl_scatlens
{
	if [ ! -f tmp/scatlens.html ]; then
		echo -e "Downloading scattering length list...\n"

		if ! wget ${SCATLENS} -O tmp/scatlens.html; then
			echo -e "Error: Cannot download scattering length list.";
			exit -1;
		fi
	fi
}


mkdir tmp
echo -e "--------------------------------------------------------------------------------"
dl_scatlens
echo -e "--------------------------------------------------------------------------------"
dl_tangoicons
echo -e "--------------------------------------------------------------------------------"
dl_findqwt
echo -e "--------------------------------------------------------------------------------"
