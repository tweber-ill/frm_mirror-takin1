#!/bin/bash
#
# @date 22-mar-17
# @author tw
#
# makes the framework libraries locally usable
#

TOOL=install_name_tool
QTVER="5.8.0_1"
PRG="takin.app"


declare -a filestochange=(
	"${PRG}/Contents/Frameworks/QtCore.framework/Versions/5/QtCore"
	"${PRG}/Contents/Frameworks/QtWidgets.framework/Versions/5/QtWidgets"
	"${PRG}/Contents/Frameworks/QtGui.framework/Versions/5/QtGui"
	"${PRG}/Contents/Frameworks/QtConcurrent.framework/Versions/5/QtConcurrent"
	"${PRG}/Contents/Frameworks/QtOpenGL.framework/Versions/5/QtOpenGL"
	"${PRG}/Contents/Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport"
	"${PRG}/Contents/Frameworks/QtSvg.framework/Versions/5/QtSvg"
	"${PRG}/Contents/Frameworks/QtXml.framework/Versions/5/QtXml"
	"${PRG}/Contents/Frameworks/QtXmlPatterns.framework/Versions/5/QtXmlPatterns"
	"${PRG}/Contents/PlugIns/printsupport/libcocoaprintersupport.dylib"
	"${PRG}/Contents/PlugIns/imageformats/libqsvg.dylib"
	"${PRG}/Contents/PlugIns/imageformats/libqicns.dylib"
	"${PRG}/Contents/PlugIns/imageformats/libqjpeg.dylib"
	"${PRG}/Contents/PlugIns/iconengines/libqsvgicon.dylib"
	"${PRG}/Contents/PlugIns/platforms/libqcocoa.dylib"
	"${PRG}/Contents/Frameworks/qwt.framework/Versions/6/qwt"
	"${PRG}/Contents/Frameworks/libfreetype.6.dylib"
	"${PRG}/Contents/Frameworks/libpng16.16.dylib"
	"${PRG}/Contents/Frameworks/libboost_iostreams.dylib"
	"${PRG}/Contents/Frameworks/libboost_filesystem.dylib"
	"${PRG}/Contents/Frameworks/libboost_python.dylib"
	"${PRG}/Contents/Frameworks/libboost_regex.dylib"
	"${PRG}/Contents/Frameworks/libboost_system.dylib"
	"${PRG}/Contents/bin/takin"
	"${PRG}/Contents/bin/convofit"
	"${PRG}/Contents/bin/convoseries"
)

declare -a changefrom=(
	"/usr/local/Cellar/qt5/${QTVER}/lib/QtCore.framework/Versions/5/QtCore"
	"/usr/local/opt/qt5/lib/QtCore.framework/Versions/5/QtCore"
	"/usr/local/Cellar/qt5/${QTVER}/lib/QtGui.framework/Versions/5/QtGui"
	"/usr/local/opt/qt5/lib/QtGui.framework/Versions/5/QtGui"
	"/usr/local/Cellar/qt5/${QTVER}/lib/QtWidgets.framework/Versions/5/QtWidgets"
	"/usr/local/opt/qt5/lib/QtWidgets.framework/Versions/5/QtWidgets"
	"/usr/local/Cellar/qt5/${QTVER}/lib/QtOpenGL.framework/Versions/5/QtOpenGL"
	"/usr/local/opt/qt5/lib/QtOpenGL.framework/Versions/5/QtOpenGL"
	"/usr/local/Cellar/qt5/${QTVER}/lib/QtConcurrent.framework/Versions/5/QtConcurrent"
	"/usr/local/opt/qt5/lib/QtConcurrent.framework/Versions/5/QtConcurrent"
	"/usr/local/Cellar/qt5/${QTVER}/lib/QtXml.framework/Versions/5/QtXml"
	"/usr/local/opt/qt5/lib/QtXml.framework/Versions/5/QtXml"
	"/usr/local/Cellar/qt5/${QTVER}/lib/QtXmlPatterns.framework/Versions/5/QtXmlPatterns"
	"/usr/local/opt/qt5/lib/QtXmlPatterns.framework/Versions/5/QtXmlPatterns"
	"/usr/local/Cellar/qt5/${QTVER}/lib/QtSvg.framework/Versions/5/QtSvg"
	"/usr/local/opt/qt5/lib/QtSvg.framework/Versions/5/QtSvg"
	"/usr/local/Cellar/qt5/${QTVER}/lib/QtPrintSupport.framework/Versions/5/QtPrintSupport"
	"/usr/local/opt/qt5/lib/QtPrintSupport.framework/Versions/5/QtPrintSupport"
	"/usr/local/opt/qwt/lib/qwt.framework/Versions/6/qwt"
	"/usr/lib/libz.1.dylib"
	"/usr/lib/libbz2.1.0.dylib"
	"/usr/local/opt/minuit2/lib/libMinuit2.0.dylib"
	"/usr/local/opt/boost/lib/libboost_system.dylib"
	"/usr/local/opt/boost/lib/libboost_filesystem.dylib"
	"/usr/local/opt/boost/lib/libboost_iostreams.dylib"
	"/usr/local/opt/boost/lib/libboost_regex.dylib"
	"/usr/local/opt/boost/lib/libboost_program_options.dylib"
	"/usr/local/opt/boost-python/lib/libboost_python.dylib"
)

declare -a changeto=(
	"@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"
	"@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"
	"@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui"
	"@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui"
	"@executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets"
	"@executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets"
	"@executable_path/../Frameworks/QtOpenGL.framework/Versions/5/QtOpenGL"
	"@executable_path/../Frameworks/QtOpenGL.framework/Versions/5/QtOpenGL"
	"@executable_path/../Frameworks/QtConcurrent.framework/Versions/5/QtConcurrent"
	"@executable_path/../Frameworks/QtConcurrent.framework/Versions/5/QtConcurrent"
	"@executable_path/../Frameworks/QtXml.framework/Versions/5/QtXml"
	"@executable_path/../Frameworks/QtXml.framework/Versions/5/QtXml"
	"@executable_path/../Frameworks/QtXmlPatterns.framework/Versions/5/QtXmlPatterns"
	"@executable_path/../Frameworks/QtXmlPatterns.framework/Versions/5/QtXmlPatterns"
	"@executable_path/../Frameworks/QtSvg.framework/Versions/5/QtSvg"
	"@executable_path/../Frameworks/QtSvg.framework/Versions/5/QtSvg"
	"@executable_path/../Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport"
	"@executable_path/../Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport"
	"@executable_path/../Frameworks/qwt.framework/Versions/6/qwt"
	"@executable_path/../Frameworks/libz.1.dylib"
	"@executable_path/../Frameworks/libbz2.1.0.dylib"
	"@executable_path/../Frameworks/libMinuit2.0.dylib"
	"@executable_path/../Frameworks/libboost_system.dylib"
	"@executable_path/../Frameworks/libboost_filesystem.dylib"
	"@executable_path/../Frameworks/libboost_iostreams.dylib"
	"@executable_path/../Frameworks/libboost_regex.dylib"
	"@executable_path/../Frameworks/libboost_program_options.dylib"
	"@executable_path/../Frameworks/libboost_python.dylib"
)

CNT=$(expr ${#changefrom[*]} - 1)


for cfile in ${filestochange[@]}; do
	echo -e "Processing binary \"${cfile}\"..."

	for idx in $(seq 0 ${CNT}); do
		cfrom=${changefrom[$idx]}
		cto=${changeto[$idx]}

		echo -e "\tChanging \"${cfrom}\"\n\t -> \"${cto}\"."
		${TOOL} -change ${cfrom} ${cto} ${cfile}
	done

	echo -e ""
done
