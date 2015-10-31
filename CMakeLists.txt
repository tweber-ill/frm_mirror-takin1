project(takin)
cmake_minimum_required(VERSION 3.0)

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_BUILD_TYPE Release)


list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}" ".")
set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

find_package(Boost REQUIRED COMPONENTS iostreams system filesystem regex python REQUIRED)
find_package(Threads REQUIRED)
find_package(Qt4 REQUIRED)
find_package(PythonLibs REQUIRED)
find_package(PNG REQUIRED)
find_package(Qwt REQUIRED)


#message("Project source dir: ${PROJECT_SOURCE_DIR}")
#message("CMake source dir: ${CMAKE_SOURCE_DIR}")
#message("Binary dir: ${CMAKE_BINARY_DIR}")
message("Install path prefix: ${CMAKE_INSTALL_PREFIX}")
message("Using qwt version ${QWT_MAJOR_VERSION}")
message("Qt moc: ${QT_MOC_EXECUTABLE}\nQt uic: ${QT_UIC_EXECUTABLE}")


add_definitions(-DUSE_NET -DUSE_IOSTR -DUSE_BOOST_REX -DUSE_PY -DUSE_GIL)
add_definitions(-DQT_VER=4 -DQWT_VER=${QWT_MAJOR_VERSION})
add_definitions(-DNO_LAPACK -DNO_3D -DNO_CLP)
add_definitions(-DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")
add_definitions(${Boost_CXX_FLAGS})


include_directories("${PROJECT_SOURCE_DIR}" ".")
include_directories("${Boost_INCLUDE_DIRS}/..")
include_directories("${PYTHON_INCLUDE_DIRS}")
include_directories("${PNG_INCLUDE_DIRS}")
include_directories("${QWT_INCLUDE_DIRS}")


set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTOUIC OFF)

add_definitions(-std=c++11)
add_definitions(-march=native -s)


add_executable(takin tools/taz/taz_main.cpp
	tools/taz/taz.cpp tools/taz/tas_layout.cpp tools/taz/taz_crys.cpp tools/taz/taz_file.cpp
	tools/taz/tas_layout.cpp tools/taz/scattering_triangle.cpp
	tools/taz/nicos.cpp tools/taz/sics.cpp tools/taz/taz_net.cpp

	tools/res/cn.cpp tools/res/pop.cpp tools/res/eck.cpp
	tools/res/ellipse.cpp tools/res/ResoDlg.cpp

	tools/monteconvo/ConvoDlg.cpp tools/monteconvo/SqwParamDlg.cpp tools/monteconvo/TASReso.cpp
	tools/monteconvo/sqw.cpp tools/monteconvo/sqw_py.cpp

	tools/scanviewer/scanviewer.cpp

	dialogs/SettingsDlg.cpp dialogs/FilePreviewDlg.cpp
	dialogs/GotoDlg.cpp dialogs/DWDlg.cpp dialogs/DynPlaneDlg.cpp
	dialogs/NeutronDlg.cpp dialogs/SpurionDlg.cpp dialogs/PowderDlg.cpp
	dialogs/RecipParamDlg.cpp dialogs/RealParamDlg.cpp
	dialogs/SrvDlg.cpp dialogs/NetCacheDlg.cpp
	dialogs/EllipseDlg.cpp

	helper/globals.cpp helper/spacegroup.cpp helper/crystalsys.cpp

	tlibs/file/loadinstr.cpp tlibs/file/recent.cpp tlibs/file/xml.cpp
	tlibs/helper/log.cpp
	tlibs/math/linalg2.cpp tlibs/math/rand.cpp
	tlibs/net/tcp.cpp
	tlibs/string/spec_char.cpp

#	helper/plotgl.cpp tools/taz/recip3d.cpp dialogs/EllipseDlg3D.cpp tlibs/gfx/gl.cpp
)


target_link_libraries(takin ${Boost_LIBRARIES} Threads::Threads
	Qt4::QtCore Qt4::QtGui Qt4::QtSvg #Qt4::QtOpenGL
	${PYTHON_LIBRARIES} ${PNG_LIBRARIES} ${QWT_LIBRARIES}
)



# pre-build
add_custom_target(prebuild-script
	COMMAND ${PROJECT_SOURCE_DIR}/prebuild.sh ${QT_UIC_EXECUTABLE} ${QT_MOC_EXECUTABLE} ${PROJECT_SOURCE_DIR}
)
add_dependencies(takin prebuild-script)


# post-build
add_custom_command(TARGET takin POST_BUILD
	COMMAND strip ${CMAKE_BINARY_DIR}/bin/takin
	MAIN_DEPENDENCY takin
)


install(TARGETS takin DESTINATION bin)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/res ${PROJECT_SOURCE_DIR}/test DESTINATION share/takin)
install(FILES ${PROJECT_SOURCE_DIR}/COPYING ${PROJECT_SOURCE_DIR}/LICENSES DESTINATION share/takin)
