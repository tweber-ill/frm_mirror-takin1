/*
 * Scattering Triangle
 * @author tweber
 * @date feb-2014
 */
#include "taz.h"
#include "helper/spec_char.h"
#include <QMetaType>

#ifdef Q_WS_X11
	extern "C" int XInitThreads();
#endif

int main(int argc, char** argv)
{
	#ifdef Q_WS_X11
		XInitThreads();
	#endif

	init_spec_chars();

	// qt needs to be able to copy these structs when emitting signals from a different thread
	qRegisterMetaType<TriangleOptions>("TriangleOptions");
	qRegisterMetaType<CrystalOptions>("CrystalOptions");

	QGL::setPreferredPaintEngine(QPaintEngine::OpenGL);

	QApplication app(argc, argv);

	::setlocale(LC_ALL, "C");
	QLocale::setDefault(QLocale::English);

	TazDlg dlg(0);
	if(argc > 1)
		dlg.Load(argv[1]);
	dlg.show();
	int iRet = app.exec();

	deinit_spec_chars();
	return iRet;
}
