CC = gcc
INC = -I/usr/include/qt4 -I/usr/local/include -I/usr/include/qt4/QtCore -I/usr/include/qt4/QtGui -I./tools/plot_gpl/src/qtterminal
LIB_DIRS = -L/usr/lib64 -L/usr/lib/x86_64-linux-gnu -L/usr/local/lib

DEFINES = 
FLAGS = ${INC} -O2 -march=native -std=c++11 -DNDEBUG ${DEFINES}
#FLAGS = ${INC} -std=c++11 -ggdb ${DEFINES}

STD_LIBS = -lsupc++ -lstdc++ -lm
LAPACK_LIBS = -L/usr/local/lib64 -llapacke -llapack -lblas -lgfortran
QT_LIBS = -L/usr/lib64/qt4 -L/usr/lib/x86_64-linux-gnu -L /usr/lib/qt4/lib \
	-lQtCore -lQtGui -lQtXml -lQtXmlPatterns -lQtOpenGL \
	-lGL -lGLU -lX11
LIBS_TAZ = -L/usr/lib64 ${STD_LIBS} ${QT_LIBS}


taz: obj/taz.o obj/taz_main.o obj/scattering_triangle.o obj/tas_layout.o obj/lattice.o obj/plotgl.o \
	obj/recip3d.o obj/spec_char.o obj/string.o obj/xml.o
	${CC} ${FLAGS} -o bin/taz obj/taz.o obj/taz_main.o obj/scattering_triangle.o obj/tas_layout.o \
			obj/lattice.o obj/plotgl.o obj/recip3d.o obj/spec_char.o obj/string.o \
			obj/xml.o \
			${LIBS_TAZ}
	strip bin/taz


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

obj/LatticeDlg.o: dialogs/LatticeDlg.cpp dialogs/LatticeDlg.h
	${CC} ${FLAGS} -c -o obj/LatticeDlg.o dialogs/LatticeDlg.cpp


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


obj/plotgl.o: plot/plotgl.cpp plot/plotgl.h
	${CC} ${FLAGS} -c -o obj/plotgl.o plot/plotgl.cpp



clean:
	find bin -regex 'bin/[_a-zA-Z0-9]*' | xargs rm -f
	rm -f bin/*.exe
	rm -f obj/*.o
	rm -f ui/*.h
	rm -f *.moc
	rm -f tools/taz/*.moc
	rm -f plot/*.moc
