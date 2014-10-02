#CC = gcc
CC = clang
DEFINES = 
QWT_VER = 6

ifeq ($(QWT_VER), 6)
	QWT_INC = -I/usr/include/qwt6 -I/usr/include/qwt -I/usr/include/qwt-qt4
	QWT_LIB = -lqwt
	DEFINES += -DUSE_QWT6
else
	QWT_INC = -I/usr/include/qwt5 -I/usr/include/qwt-qt4
	QWT_LIB = -lqwt
#	QWT_LIB = -lqwt-qt4
endif

INC = -I/usr/include/qt4 -I/usr/local/include -I/usr/include/qt4/QtCore \
	-I/usr/include/qt4/QtGui -I/usr/include/QtCore -I/usr/include/QtGui \
	-I/usr/include/lapacke \
	${QWT_INC}

LIB_DIRS = -L/usr/lib64 -L/usr/lib/x86_64-linux-gnu -L/usr/local/lib

DEFINES += -DUSE_LAPACK
FLAGS = ${INC} -O2 -march=native -std=c++11 -DNDEBUG ${DEFINES}
#FLAGS = ${INC} -std=c++11 -ggdb ${DEFINES}

STD_LIBS = -lstdc++ -lm -lboost_system
STD_LIBS += -lpthread
LAPACK_LIBS = -L/usr/local/lib64 -llapacke -llapack -lblas -lgfortran
QT_LIBS = -L/usr/lib64/qt4 -L/usr/lib/x86_64-linux-gnu -L /usr/lib/qt4/lib \
	-lQtCore -lQtGui -lQtXml -lQtXmlPatterns -lQtOpenGL -lQtSvg \
	-lGL -lGLU -lX11
LIBS_TAZ = -L/usr/lib64 ${STD_LIBS} ${QT_LIBS}
LIBS_RESO = -L/usr/lib64 ${QWT_LIB} ${STD_LIBS} ${QT_LIBS} ${LAPACK_LIBS}


OBJ_COMMON = obj/log.o obj/linalg.o obj/linalg2.o obj/string.o obj/rand.o \
	obj/spec_char.o obj/EllipseDlg.o obj/ellipse.o
OBJ_TAZ = obj/taz_main.o obj/taz.o obj/recip3d.o \
	obj/scattering_triangle.o obj/tas_layout.o \
	obj/RecipParamDlg.o obj/RealParamDlg.o \
	obj/SpurionDlg.o obj/NeutronDlg.o obj/SrvDlg.o obj/nicos.o \
	obj/xml.o obj/tcp.o obj/lattice.o obj/spacegroup.o \
	obj/plotgl.o obj/cn.o obj/pop.o obj/ResoDlg.o obj/EllipseDlg3D.o
OBJ_MONTERESO = obj/montereso_res.o obj/montereso_res_main.o


takin: ${OBJ_COMMON} ${OBJ_TAZ}
	${CC} ${FLAGS} -o bin/takin $+ ${LIBS_TAZ} ${LIBS_RESO}
	strip bin/takin

montereso: ${OBJ_COMMON} ${OBJ_MONTERESO}
	${CC} ${FLAGS} -o bin/montereso $+ ${STD_LIBS} ${QT_LIBS} ${QWT_LIB} ${LAPACK_LIBS}
	strip montereso


obj/taz_main.o: tools/taz/taz_main.cpp tools/taz/taz.h
	${CC} ${FLAGS} -c -o $@ $<
obj/taz.o: tools/taz/taz.cpp tools/taz/taz.h
	${CC} ${FLAGS} -c -o $@ $<
obj/recip3d.o: tools/taz/recip3d.cpp tools/taz/recip3d.h
	${CC} ${FLAGS} -c -o $@ $<
obj/scattering_triangle.o: tools/taz/scattering_triangle.cpp tools/taz/scattering_triangle.h
	${CC} ${FLAGS} -c -o $@ $<
obj/tas_layout.o: tools/taz/tas_layout.cpp tools/taz/tas_layout.h
	${CC} ${FLAGS} -c -o $@ $<
obj/RecipParamDlg.o: dialogs/RecipParamDlg.cpp dialogs/RecipParamDlg.h
	${CC} ${FLAGS} -c -o $@ $<
obj/RealParamDlg.o: dialogs/RealParamDlg.cpp dialogs/RealParamDlg.h
	${CC} ${FLAGS} -c -o $@ $<
obj/SpurionDlg.o: dialogs/SpurionDlg.cpp dialogs/SpurionDlg.h
	${CC} ${FLAGS} -c -o $@ $<
obj/NeutronDlg.o: dialogs/NeutronDlg.cpp dialogs/NeutronDlg.h
	${CC} ${FLAGS} -c -o $@ $<
obj/SrvDlg.o: dialogs/SrvDlg.cpp dialogs/SrvDlg.h
	${CC} ${FLAGS} -c -o $@ $<
obj/nicos.o: tools/taz/nicos.cpp tools/taz/nicos.h
	${CC} ${FLAGS} -c -o $@ $<

obj/string.o: helper/string.cpp helper/string.h
	${CC} ${FLAGS} -c -o $@ $<
obj/spec_char.o: helper/spec_char.cpp helper/spec_char.h
	${CC} ${FLAGS} -c -o $@ $<
obj/xml.o: helper/xml.cpp helper/xml.h
	${CC} ${FLAGS} -c -o $@ $<
obj/log.o: helper/log.cpp helper/log.h
	${CC} ${FLAGS} -c -o $@ $<
obj/tcp.o: helper/tcp.cpp helper/tcp.h
	${CC} ${FLAGS} -c -o $@ $<
obj/linalg.o: helper/linalg.cpp helper/linalg.h helper/geo.h
	${CC} ${FLAGS} -c -o $@ $<
obj/linalg2.o: helper/linalg2.cpp helper/linalg2.h helper/geo.h
	${CC} ${FLAGS} -c -o $@ $<
obj/lattice.o: helper/lattice.cpp helper/lattice.h
	${CC} ${FLAGS} -c -o $@ $<
obj/spacegroup.o: helper/spacegroup.cpp helper/spacegroup.h
	${CC} ${FLAGS} -c -o $@ $<
obj/rand.o: helper/rand.cpp helper/rand.h
	${CC} ${FLAGS} -c -o $@ $<
obj/plotgl.o: helper/plotgl.cpp helper/plotgl.h
	${CC} ${FLAGS} -c -o $@ $<

obj/cn.o: tools/res/cn.cpp tools/res/cn.h
	${CC} ${FLAGS} -c -o $@ $<
obj/pop.o: tools/res/pop.cpp tools/res/pop.h
	${CC} ${FLAGS} -c -o $@ $<
obj/ellipse.o: tools/res/ellipse.cpp tools/res/ellipse.h
	${CC} ${FLAGS} -c -o $@ $<
obj/ResoDlg.o: tools/res/ResoDlg.cpp tools/res/ResoDlg.h
	${CC} ${FLAGS} -c -o $@ $<
obj/EllipseDlg.o: dialogs/EllipseDlg.cpp dialogs/EllipseDlg.h
	${CC} ${FLAGS} -c -o $@ $<
obj/EllipseDlg3D.o: dialogs/EllipseDlg3D.cpp dialogs/EllipseDlg3D.h
	${CC} ${FLAGS} -c -o $@ $<

obj/montereso_res.o: tools/montereso/res.cpp tools/montereso/res.h
	${CC} ${FLAGS} -c -o $@ $<
obj/montereso_res_main.o: tools/montereso/res_main.cpp
	${CC} ${FLAGS} -c -o $@ $<



clean:
	find bin -regex 'bin/[_a-zA-Z0-9]*' | xargs rm -f
	rm -f bin/*.exe
	rm -f obj/*.o
	rm -f ui/*.h
	rm -f *.moc
	rm -f tools/taz/*.moc
	rm -f tools/res/*.moc
	rm -f helper/*.moc
	rm -f dialogs/*.moc
