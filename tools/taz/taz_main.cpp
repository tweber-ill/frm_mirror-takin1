/*
 * TAS tool
 * @author tweber
 * @date feb-2014
 */
#include "taz.h"
#include "tlibs/string/spec_char.h"
#include "tlibs/helper/log.h"
#include "dialogs/NetCacheDlg.h"

#include <QMetaType>


#ifdef Q_WS_X11
	extern "C" int XInitThreads();
#endif

int main(int argc, char** argv)
{
	try
	{
		tl::log_info("Starting up Takin.");

		#ifdef Q_WS_X11
			XInitThreads();
		#endif

		tl::init_spec_chars();

		// qt needs to be able to copy these structs when emitting signals from a different thread
		qRegisterMetaType<TriangleOptions>("TriangleOptions");
		qRegisterMetaType<CrystalOptions>("CrystalOptions");
		qRegisterMetaType<std::string>("std::string");
		qRegisterMetaType<CacheVal>("CacheVal");

		QGL::setPreferredPaintEngine(QPaintEngine::OpenGL);

		QApplication app(argc, argv);

		::setlocale(LC_ALL, "C");
		QLocale::setDefault(QLocale::English);

		TazDlg dlg(0);
		if(argc > 1)
			dlg.Load(argv[1]);
		dlg.show();
		int iRet = app.exec();

		tl::deinit_spec_chars();

		tl::log_info("Shutting down Takin.");
		return iRet;
	}
	catch(const std::exception& ex)
	{
		tl::log_crit(ex.what());
	}

	return -1;
}
