/*
 * SG-List
 * @author tweber
 * @date oct-2015
 * @copyright GPLv2
 */

#include <clocale>
#include <QLocale>
#include <QApplication>
#include <iostream>
#include "SgListDlg.h"
#include "helper/spacegroup.h"


int main(int argc, char** argv)
{
	if(!init_space_groups())
	{
		std::cerr << "Space group table not found!" << std::endl;
		return -1;
	}

	QApplication app(argc, argv);

	std::setlocale(LC_ALL, "C");
	QLocale::setDefault(QLocale::English);

	SgListDlg dlg(0);
	dlg.setWindowFlags(Qt::Window);
	dlg.show();

	int iRet = app.exec();
	return iRet;
}
