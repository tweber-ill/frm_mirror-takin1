/*
 * Scattering Triangle
 * @author tweber
 * @date feb-2014
 */
#include "taz.h"

int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	::setlocale(LC_ALL, "C");
	QLocale::setDefault(QLocale::English);

	TazDlg dlg(0);
	return dlg.exec();
}
