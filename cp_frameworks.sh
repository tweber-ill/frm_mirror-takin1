#!/bin/bash
#
# @date 12-apr-17
# @author Tobias Weber <tobias.weber@tum.de>
# @license GPLv2
#
# copy framework libs
#


PRG="takin.app"
DST_DIR="${PRG}/Contents/Frameworks/"


declare -a SRC_LIBS=(
	"/usr/local/opt/qt5/lib/QtCore.framework"
	"/usr/local/opt/qt5/lib/QtGui.framework"
	"/usr/local/opt/qt5/lib/QtWidgets.framework"
	"/usr/local/opt/qt5/lib/QtOpenGL.framework"
	"/usr/local/opt/qt5/lib/QtConcurrent.framework"
	"/usr/local/opt/qt5/lib/QtXml.framework"
	"/usr/local/opt/qt5/lib/QtSvg.framework"
	"/usr/local/opt/qt5/lib/QtPrintSupport.framework"
	"/usr/local/opt/qwt/lib/qwt.framework"
	"/usr/lib/libz.1.dylib"
	"/usr/lib/libbz2.1.0.dylib"
	"/usr/local/opt/minuit2/lib/libMinuit2.0.dylib"
	"/usr/local/opt/boost/lib/libboost_system.dylib"
	"/usr/local/opt/boost/lib/libboost_filesystem.dylib"
	"/usr/local/opt/boost/lib/libboost_iostreams.dylib"
	"/usr/local/opt/boost/lib/libboost_regex.dylib"
	"/usr/local/opt/boost/lib/libboost_program_options.dylib"
	"/usr/local/opt/boost-python/lib/libboost_python.dylib"
	"/usr/local/opt/freetype/lib/libfreetype.6.dylib"
	"/usr/local/opt/libpng/lib/libpng16.16.dylib"
)



# create dirs
mkdir -pv ${DST_DIR}

# copy libs
for srclib in ${SRC_LIBS[@]}; do
	echo -e "Copying library \"${cfile}\"..."
	cp -rv ${srclib} ${DST_DIR}
done


# attribs
chmod -R -u+rw ${DST_DIR}


# delete unnecessary files
find "${DST_DIR}" -type d -name "Headers" -print0 | xargs -0 rm -rfv
find "${DST_DIR}" -type d -name "Current" -print0 | xargs -0 rm -rfv
