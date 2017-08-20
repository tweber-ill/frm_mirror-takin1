#!/bin/bash
#
# creates a DEB distro
# @author Tobias Weber <tobias.weber@tum.de>
# @license GPLv2
#


# installation directory
INSTDIR=~/tmp/takin


# directories
mkdir -p ${INSTDIR}/usr/local/bin
mkdir -p ${INSTDIR}/usr/local/lib
mkdir -p ${INSTDIR}/usr/local/share/takin/res
mkdir -p ${INSTDIR}/DEBIAN


# control file
echo -e "Package: takin\nVersion: 1.5.3" > ${INSTDIR}/DEBIAN/control
echo -e "Description: inelastic neutron scattering software" >> ${INSTDIR}/DEBIAN/control
echo -e "Maintainer: n/a" >> ${INSTDIR}/DEBIAN/control
echo -e "Architecture: $(dpkg --print-architecture)" >> ${INSTDIR}/DEBIAN/control
echo -e "Section: base\nPriority: optional" >> ${INSTDIR}/DEBIAN/control
echo -e "Depends: libstdc++6, libboost, libqtcore4 (>=4.8.0), libqtgui4 (>=4.8.0), libqtopengl4 (>=4.8.0), libqtsvg4 (>=4.8.0), libqtxml4 (>=4.8.0), libqwt, python, freetype6\n" >> ${INSTDIR}/DEBIAN/control


# copy files
cp -v bin/takin			${INSTDIR}/usr/local/bin
cp -v bin/convofit		${INSTDIR}/usr/local/bin
cp -v bin/convoseries	${INSTDIR}/usr/local/bin
cp -rv res/*			${INSTDIR}/usr/local/share/takin/res/
cp -v COPYING			${INSTDIR}/usr/local/share/takin
cp -v LICENSES			${INSTDIR}/usr/local/share/takin
cp -v LITERATURE		${INSTDIR}/usr/local/share/takin
cp -v AUTHORS			${INSTDIR}/usr/local/share/takin
cp -v /usr/local/lib/libMinuit2.so.0 ${INSTDIR}/usr/local/lib


# stripping
strip -v ${INSTDIR}/usr/local/bin/*
strip -v ${INSTDIR}/usr/local/lib/*


# build package
cd ${INSTDIR}/..
dpkg --build takin
