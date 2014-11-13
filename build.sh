#!/bin/bash


echo -e "Prebuilding..."
if ! ./prebuild.sh
then
	echo -e "Error: Prebuild failed";
	exit -1;
fi


echo -e "\nBuilding..."
make -j4 -f themakefile
