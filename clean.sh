#!/bin/bash

echo -e "Cleaning stuff made by themakefile..."
make -f themakefile clean


if [ -f Makefile ]
then
	echo -e "\nCleaning stuff made by Makefile..."
	make clean
fi

rm -f CMakeCache.txt
rm -rf CMakeFiles
