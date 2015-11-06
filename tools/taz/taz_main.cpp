/*
 * TAS tool
 * @author tweber
 * @date feb-2014
 * @license GPLv2
 */
#include "taz.h"
#include "tlibs/string/spec_char.h"
#include "tlibs/helper/log.h"
#include "tlibs/version.h"
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

		// tlibs version check
		tl::log_info("Using tLibs version ", tl::get_tlibs_version(), ".");
		if(!tl::check_tlibs_version(TLIBS_VERSION))
		{
			tl::log_crit("Version mismatch in tLibs. Please recompile.");
			tl::log_crit("tLibs versions: library: ", tl::get_tlibs_version(),
				", headers: ", TLIBS_VERSION, ".");
			return -1;
		}


		#if defined Q_WS_X11 && !defined NO_3D
			XInitThreads();
			QGL::setPreferredPaintEngine(QPaintEngine::OpenGL);
		#endif

		tl::init_spec_chars();

		// qt needs to be able to copy these structs when emitting signals from a different thread
		qRegisterMetaType<TriangleOptions>("TriangleOptions");
		qRegisterMetaType<CrystalOptions>("CrystalOptions");
		qRegisterMetaType<std::string>("std::string");
		qRegisterMetaType<CacheVal>("CacheVal");

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
