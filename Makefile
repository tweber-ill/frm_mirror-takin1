CC = gcc
DEFINES = 
QWT_VER = 6

ifeq ($(QWT_VER), 6)
	QWT_INC = -I/usr/include/qwt6 -I/usr/include/qwt
	QWT_LIB = -lqwt
	DEFINES += -DUSE_QWT6
else
	QWT_INC = -I/usr/include/qwt5
	QWT_LIB = -lqwt
endif

INC = -I/usr/include/qt4 -I/usr/local/include -I/usr/include/qt4/QtCore \
	-I/usr/include/qt4/QtGui -I/usr/include/QtCore -I/usr/include/QtGui \
	-I/usr/include/lapacke \
	${QWT_INC}

LIB_DIRS = -L/usr/lib64 -L/usr/lib/x86_64-linux-gnu -L/usr/local/lib

DEFINES += -DUSE_LAPACK
#FLAGS = ${INC} -O2 -march=native -std=c++11 -DNDEBUG ${DEFINES}
FLAGS = ${INC} -std=c++11 -ggdb ${DEFINES}

STD_LIBS = -lstdc++ -lm
LAPACK_LIBS = -L/usr/local/lib64 -llapacke -llapack -lblas -lgfortran
QT_LIBS = -L/usr/lib64/qt4 -L/usr/lib/x86_64-linux-gnu -L /usr/lib/qt4/lib \
	-lQtCore -lQtGui -lQtXml -lQtXmlPatterns -lQtOpenGL -lQtSvg \
	-lGL -lGLU -lX11
LIBS_TAZ = -L/usr/lib64 ${STD_LIBS} ${QT_LIBS}
LIBS_RESO = -L/usr/lib64 ${QWT_LIB} ${STD_LIBS} ${QT_LIBS} ${LAPACK_LIBS}


taz: obj/taz.o obj/taz_main.o obj/scattering_triangle.o obj/tas_layout.o obj/lattice.o obj/plotgl.o \
	obj/recip3d.o obj/spec_char.o obj/string.o obj/xml.o obj/spacegroup.o \
	obj/RecipParamDlg.o obj/RealParamDlg.o \
	obj/cn.o obj/pop.o obj/ellipse.o obj/ResoDlg.o obj/linalg.o \
	obj/EllipseDlg.o obj/EllipseDlg3D.o
	${CC} ${FLAGS} -o bin/taz obj/taz.o obj/taz_main.o obj/scattering_triangle.o obj/tas_layout.o \
			obj/lattice.o obj/plotgl.o obj/recip3d.o obj/spec_char.o obj/string.o \
			obj/xml.o obj/spacegroup.o \
			obj/RecipParamDlg.o obj/RealParamDlg.o \
			obj/cn.o obj/pop.o obj/ellipse.o obj/ResoDlg.o obj/linalg.o \
			obj/EllipseDlg.o obj/EllipseDlg3D.o \
			${LIBS_TAZ} ${LIBS_RESO}
#	strip bin/taz


obj/taz_main.o: tools/taz/taz_main.cpp tools/taz/taz.h
	${CC} ${FLAGS} -c -o obj/taz_main.o tools/taz/taz_main.cpp

obj/taz.o: tools/taz/taz.cpp tools/taz/taz.h
	${CC} ${FLAGS} -c -o obj/taz.o tools/taz/taz.cpp

obj/recip3d.o: tools/taz/recip3d.cpp tools/taz/recip3d.h
	${CC} ${FLAGS} -c -o obj/recip3d.o tools/taz/recip3d.cpp

obj/scattering_triangle.o: tools/taz/scattering_triangle.cpp tools/taz/scattering_triangle.h
	${CC} ${FLAGS} -c -o obj/scattering_triangle.o tools/taz/scattering_triangle.cpp

obj/tas_layout.o: tools/taz/tas_layout.cpp tools/taz/tas_layout.h
	${CC} ${FLAGS} -c -o obj/tas_layout.o tools/taz/tas_layout.cpp

obj/RecipParamDlg.o: dialogs/RecipParamDlg.cpp dialogs/RecipParamDlg.h
	${CC} ${FLAGS} -c -o obj/RecipParamDlg.o dialogs/RecipParamDlg.cpp

obj/RealParamDlg.o: dialogs/RealParamDlg.cpp dialogs/RealParamDlg.h
	${CC} ${FLAGS} -c -o obj/RealParamDlg.o dialogs/RealParamDlg.cpp



obj/string.o: helper/string.cpp helper/string.h
	${CC} ${FLAGS} -c -o obj/string.o helper/string.cpp

obj/spec_char.o: helper/spec_char.cpp helper/spec_char.h
	${CC} ${FLAGS} -c -o obj/spec_char.o helper/spec_char.cpp

obj/xml.o: helper/xml.cpp helper/xml.h
	${CC} ${FLAGS} -c -o obj/xml.o helper/xml.cpp

obj/linalg.o: helper/linalg.cpp helper/linalg.h
	${CC} ${FLAGS} -c -o obj/linalg.o helper/linalg.cpp

obj/lattice.o: helper/lattice.cpp helper/lattice.h
	${CC} ${FLAGS} -c -o obj/lattice.o helper/lattice.cpp

obj/spacegroup.o: helper/spacegroup.cpp helper/spacegroup.h
	${CC} ${FLAGS} -c -o obj/spacegroup.o helper/spacegroup.cpp


obj/plotgl.o: plot/plotgl.cpp plot/plotgl.h
	${CC} ${FLAGS} -c -o obj/plotgl.o plot/plotgl.cpp



obj/cn.o: tools/res/cn.cpp tools/res/cn.h
	${CC} ${FLAGS} -c -o obj/cn.o tools/res/cn.cpp

obj/pop.o: tools/res/pop.cpp tools/res/pop.h
	${CC} ${FLAGS} -c -o obj/pop.o tools/res/pop.cpp

obj/ellipse.o: tools/res/ellipse.cpp tools/res/ellipse.h
	${CC} ${FLAGS} -c -o obj/ellipse.o tools/res/ellipse.cpp

obj/ResoDlg.o: tools/res/ResoDlg.cpp tools/res/ResoDlg.h
	${CC} ${FLAGS} -c -o obj/ResoDlg.o tools/res/ResoDlg.cpp

obj/EllipseDlg.o: dialogs/EllipseDlg.cpp dialogs/EllipseDlg.h
	${CC} ${FLAGS} -c -o obj/EllipseDlg.o dialogs/EllipseDlg.cpp

obj/EllipseDlg3D.o: dialogs/EllipseDlg3D.cpp dialogs/EllipseDlg3D.h
	${CC} ${FLAGS} -c -o obj/EllipseDlg3D.o dialogs/EllipseDlg3D.cpp


clean:
	find bin -regex 'bin/[_a-zA-Z0-9]*' | xargs rm -f
	rm -f bin/*.exe
	rm -f obj/*.o
	rm -f ui/*.h
	rm -f *.moc
	rm -f tools/taz/*.moc
	rm -f tools/res/*.moc
	rm -f plot/*.moc
	rm -f dialogs/*.moc
