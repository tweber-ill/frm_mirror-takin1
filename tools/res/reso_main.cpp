/*
 * mieze-tool
 * @author tweber
 * @date 01-may-2013
 */

#include "ResoDlg.h"
#include "../../settings.h"
#include <QtGui/QApplication>

#ifdef Q_WS_X11
	extern "C" int XInitThreads();
#endif

int main(int argc, char **argv)
{
	try
	{
		#ifdef Q_WS_X11
			XInitThreads();
		#endif
		QGL::setPreferredPaintEngine(QPaintEngine::OpenGL);


		QSettings *pGlobals = Settings::GetGlobals();
		QApplication a(argc, argv);

		ResoDlg wnd(0);
		wnd.exec();
	}
	catch(const std::exception& ex)
	{
		std::cerr << "\n********************************************************************************\n"
					 << "CRITICAL ERROR: " << ex.what()
					 << "\n********************************************************************************\n"
					 << std::endl;
	}

	Settings::free();
	return 0;
}

#include "../../subwnd.moc"
