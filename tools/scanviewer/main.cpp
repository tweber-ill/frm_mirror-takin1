/*
 * Scan viewer
 * @author tweber
 * @date mar-2015
 * @copyright GPLv2
 */

#include <QtCore/QLocale>
#include <QtGui/QApplication>
#include "scanviewer.h"

int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	::setlocale(LC_ALL, "C");
	QLocale::setDefault(QLocale::English);

	ScanViewerDlg dlg(0);
	dlg.setWindowFlags(Qt::Window);
	dlg.show();

	int iRet = app.exec();
	return iRet;
}
