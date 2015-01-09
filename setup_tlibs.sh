#!/bin/bash

TLIBS=tlibs-master.tar.bz2
TLIBS_DIR=tlibs

if [ -L ${TLIBS_DIR} ]
then
	echo -e "Error: Symbolic link \"${TLIBS_DIR}\" exists. Aborting.";
	exit -1;
fi

rm ${TLIBS}

if ! wget http://forge.frm2.tum.de/cgit/cgit.cgi/frm2/mira/tlibs.git/snapshot/${TLIBS}
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
rm ${TLIBS}
