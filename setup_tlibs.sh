#!/bin/bash

TLIBS=tlibs-master.tar.bz2
REPO=http://forge.frm2.tum.de/cgit/cgit.cgi/frm2/mira/tlibs.git/snapshot
TLIBS_DIR=tlibs

function dl_tlibs
{
	echo -e "Downloading tlibs..."
	rm -f ${TLIBS}

	if ! wget ${REPO}/${TLIBS}
	then
		echo -e "Error: Cannot download tlibs.";
		exit -1;
	fi

	if ! tar -xjvf ${TLIBS}
	then
		echo -e "Error: Cannot extract tlibs.";
		exit -1;
	fi

	mv tlibs-master ${TLIBS_DIR}
	rm -f ${TLIBS}
}


if [ -L ${TLIBS_DIR} ]
then
	echo -e "Error: Symbolic link \"${TLIBS_DIR}\" already exists. Aborting.";
	exit -1;
fi


rm -rf ${TLIBS_DIR}
dl_tlibs
