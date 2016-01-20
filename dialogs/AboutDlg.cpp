/*
 * About Dialog
 * @author Tobias Weber
 * @date nov-2015
 * @license GPLv2
 */

#include "AboutDlg.h"

#include <boost/config.hpp>
#include <boost/version.hpp>
#include <qwt_global.h>
#include "tlibs/version.h"

#include "tlibs/string/string.h"
#include "../helper/formfact.h"
#include "../helper/globals.h"
#include <sstream>


#define TAKIN_VER "0.9.5"


AboutDlg::AboutDlg(QWidget* pParent, QSettings *pSett)
	: QDialog(pParent), m_pSettings(pSett)
{
	setupUi(this);

	labelVersion->setText("Version " TAKIN_VER);
	labelWritten->setText("Written by Tobias Weber");
	labelYears->setText("2014 - 2015");

	labelCC->setText(QString("Built with ") + QString(BOOST_COMPILER));
	labelBuildDate->setText(QString("Build date: ") +
		QString(__DATE__) + ", " + QString(__TIME__));


	// -------------------------------------------------------------------------


	std::ostringstream ostrLibs;
	ostrLibs << "<html><body>";

	ostrLibs << "<dl>";

	ostrLibs << "<dt>Uses Qt version " << QT_VERSION_STR << "</dt>";
	ostrLibs << "<dd><a href=\"http://qt-project.org\">http://qt-project.org</a></dd>";

	ostrLibs << "<dt>Uses Qwt version " << QWT_VERSION_STR << "</dt>";
	ostrLibs << "<dd><a href=\"http://qwt.sourceforge.net\">http://qwt.sourceforge.net</a></dd>";

	std::string strBoost = BOOST_LIB_VERSION;
	tl::find_all_and_replace<std::string>(strBoost, "_", ".");
	ostrLibs << "<dt>Uses Boost version " << strBoost << "</dt>";
	ostrLibs << "<dd><a href=\"http://www.boost.org\">http://www.boost.org</a></dd>";

#ifndef NO_LAPACK
	ostrLibs << "<dt>Uses Lapack/e version 3</dt>";
	ostrLibs << "<dd><a href=\"http://www.netlib.org/lapack\">http://www.netlib.org/lapack</a></dd>";
#endif

	ostrLibs << "<dt>Uses tLibs version " << TLIBS_VERSION << "</dt>";

	ostrLibs << "<dt>Uses Clipper crystallography library</dt>";
	ostrLibs << "<dd><a href=\"http://www.ysbl.york.ac.uk/~cowtan/clipper\">http://www.ysbl.york.ac.uk/~cowtan/clipper</a></dd>";

	ostrLibs << "<dt>Uses resolution algorithms ported from Rescal version 5</dt>";
	ostrLibs << "<dd><a href=\"http://www.ill.eu/en/html/instruments-support/computing-for-science/cs-software/all-software/matlab-ill/rescal-for-matlab\">http://www.ill.eu/en/html/instruments-support/computing-for-science/cs-software/all-software/matlab-ill/rescal-for-matlab</a></dd>";

	ostrLibs << "<dt>Uses Tango icons</dt>";
	ostrLibs << "<dd><a href=\"http://tango.freedesktop.org\">http://tango.freedesktop.org</a></dd>";

	ostrLibs << "</dl>";

	ostrLibs << "<p>See the LICENSES file in the Takin root directory.</p>";
	ostrLibs << "</body></html>";

	labelLibraries->setText(ostrLibs.str().c_str());


	// -------------------------------------------------------------------------


	FormfactList lstff;
	ScatlenList lstsl;

	std::ostringstream ostrConst;
	ostrConst << "<html><body>";

	ostrConst << "<dl>";

	ostrConst << "<dt>Physical constants from Boost Units</dt>";
	ostrConst << "<dd><a href=\"http://www.boost.org/doc/libs/release/libs/units/\">http://www.boost.org/doc/libs/release/libs/units/</a></dd>";

	if(g_bHasFormfacts)
	{
		ostrConst << "<dt>" << lstff.GetSource() <<"</dt>";
		ostrConst << "<dd><a href=\"" << lstff.GetSourceUrl() << "\">" << lstff.GetSourceUrl() << "</a></dd>";
	}
	if(g_bHasScatlens)
	{
		ostrConst << "<dt>" << lstsl.GetSource() <<"</dt>";
		ostrConst << "<dd><a href=\"" << lstsl.GetSourceUrl() << "\">" << lstsl.GetSourceUrl() << "</a></dd>";
	}

	ostrConst << "</dl>";
	ostrConst << "</body></html>";
	labelConst->setText(ostrConst.str().c_str());
}


#include "AboutDlg.moc"
