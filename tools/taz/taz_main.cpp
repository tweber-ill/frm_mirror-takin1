/*
 * Scattering Triangle
 * @author tweber
 * @date feb-2014
 */
#include "taz.h"
#include "helper/spec_char.h"

#ifdef Q_WS_X11
	extern "C" int XInitThreads();
#endif

int main(int argc, char** argv)
{
	#ifdef Q_WS_X11
		XInitThreads();
	#endif

	init_spec_chars();

	QGL::setPreferredPaintEngine(QPaintEngine::OpenGL);

	QApplication app(argc, argv);

	::setlocale(LC_ALL, "C");
	QLocale::setDefault(QLocale::English);

	TazDlg dlg(0);
	int iRet = dlg.exec();

	deinit_spec_chars();
	return iRet;
}
