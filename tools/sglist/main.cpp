/*
 * SG-List
 * @author tweber
 * @date oct-2015
 * @copyright GPLv2
 */

#include <QLocale>
#include <QApplication>
#include "SgListDlg.h"

int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	::setlocale(LC_ALL, "C");
	QLocale::setDefault(QLocale::English);

	SgListDlg dlg(0);
	dlg.setWindowFlags(Qt::Window);
	dlg.show();

	int iRet = app.exec();
	return iRet;
}
