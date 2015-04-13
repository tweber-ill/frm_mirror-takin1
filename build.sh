#!/bin/bash


if [ ! -d tlibs ] || [ ! -f tlibs/AUTHORS ]
then
	echo -e "Error: tlibs not installed. Use ./setup_tlibs.sh"
	exit -1;
fi




echo -e "Prebuilding..."
if ! ./prebuild.sh
then
	echo -e "Error: Prebuild failed";
	exit -1;
fi




NPROC=$(which nproc 2>/dev/null)
if [ "$NPROC" == "" ]; then NPROC="/usr/bin/nproc"; fi


if [ ! -x $NPROC ]
then
	CPUCNT=4
else
	CPUCNT=$($NPROC)
fi

echo -e "\nBuilding using $CPUCNT processes..."
make -j${CPUCNT} -f themakefile
