/*
 * TAS tool
 * @author tweber
 * @date feb-2014
 * @license GPLv2
 */

#include "taz.h"
#include "tlibs/string/spec_char.h"
#include "tlibs/log/log.h"
#include "tlibs/version.h"
#include "dialogs/NetCacheDlg.h"
#include "dialogs/AboutDlg.h"
#include "helper/globals.h"
#include "helper/spacegroup.h"

#include <clocale>
#include <memory>
#include <QMetaType>
#include <QMessageBox>
#include <QCoreApplication>


#ifdef Q_WS_X11
	extern "C" int XInitThreads();
#endif


#define TAKIN_CHECK " Please check if Takin is correctly installed and the current working directory is set to the Takin main directory."


static void add_logfile(std::ofstream* postrLog, bool bAdd=1)
{
	if(!postrLog || !postrLog->is_open())
	{
		tl::log_err("Cannot open log file.");
		return;
	}

	for(tl::Log* plog : { &tl::log_info, &tl::log_warn, &tl::log_err, &tl::log_crit, &tl::log_debug })
	{
		if(bAdd)
			plog->AddOstr(postrLog, 0, 0);
		else
			plog->RemoveOstr(postrLog);
	}

	if(!bAdd) postrLog->operator<<(std::endl);
}

int main(int argc, char** argv)
{
	try
	{
		std::ofstream ofstrLog("takin.log", std::ios_base::out|std::ios_base::app);
		add_logfile(&ofstrLog, 1);

		tl::log_info("Starting up Takin version ", TAKIN_VER, ".");

		#if defined Q_WS_X11 && !defined NO_3D
			XInitThreads();
			QGL::setPreferredPaintEngine(QPaintEngine::OpenGL);
		#endif

		// qt needs to be able to copy these structs when emitting signals from a different thread
		qRegisterMetaType<TriangleOptions>("TriangleOptions");
		qRegisterMetaType<CrystalOptions>("CrystalOptions");
		qRegisterMetaType<std::string>("std::string");
		qRegisterMetaType<CacheVal>("CacheVal");

		std::unique_ptr<QApplication> app(new QApplication(argc, argv));
		std::setlocale(LC_ALL, "C");
		QLocale::setDefault(QLocale::English);

		std::string strApp = QCoreApplication::applicationDirPath().toStdString();
		tl::log_info("Application path: ", strApp);
		add_resource_path(strApp);
		add_resource_path(strApp + "/..");
		add_resource_path(strApp + "/resources");
		add_resource_path(strApp + "/Resources");
		add_resource_path(strApp + "/../resources");
		add_resource_path(strApp + "/../Resources");


		// ------------------------------------------------------------
		// tlibs version check
		tl::log_info("Using tLibs version ", tl::get_tlibs_version(), ".");
		if(!tl::check_tlibs_version(TLIBS_VERSION))
		{
			tl::log_crit("Version mismatch in tLibs. Please recompile.");
			tl::log_crit("tLibs versions: library: ", tl::get_tlibs_version(),
				", headers: ", TLIBS_VERSION, ".");

			QMessageBox::critical(0, "Takin - Error", "Broken build: Mismatch in tlibs version.");
			return -1;
		}

		// check tables
		g_bHasScatlens = (find_resource("res/scatlens.xml") != "");
		g_bHasFormfacts = (find_resource("res/ffacts.xml") != "");
		if(!g_bHasScatlens)
		{
			const char* pcErr = "Scattering length table could not be found." TAKIN_CHECK;
			tl::log_err(pcErr);

			QMessageBox::critical(0, "Takin - Error", pcErr);
			return -1;
		}
		if(!g_bHasFormfacts)
		{
			const char* pcErr = "Form factor coefficient table could not be found." TAKIN_CHECK;
			tl::log_err(pcErr);

			QMessageBox::critical(0, "Takin - Error", pcErr);
			return -1;
		}

		if(!init_space_groups())
		{
			const char* pcErr = "Space group table could not be found!" TAKIN_CHECK;
			tl::log_err(pcErr);

			QMessageBox::critical(0, "Takin - Error", pcErr);
			return -1;
		}

		tl::init_spec_chars();


		// check if icons are available
		if(find_resource("res/document-new.svg") == "")
		{
			const char* pcErr = "Takin resources could not be found!" TAKIN_CHECK;
			tl::log_err(pcErr);

			QMessageBox::critical(0, "Takin - Error", pcErr);
			return -1;
		}


		// ------------------------------------------------------------
		std::unique_ptr<TazDlg> dlg(new TazDlg(0));
		if(argc > 1)
			dlg->Load(argv[1]);
		dlg->show();
		int iRet = app->exec();


		// ------------------------------------------------------------
		tl::deinit_spec_chars();
		tl::log_info("Shutting down Takin.");
		add_logfile(&ofstrLog, 0);

		return iRet;
	}
	catch(const std::exception& ex)
	{
		tl::log_crit(ex.what());
	}
	return -1;
}
