/*
 * Montereso
 * @author tweber
 * @date 2012, 22-sep-2014
 */

#include "res.h"
#include "helper/log.h"
#include "helper/string.h"
#include "dialogs/EllipseDlg.h"

#include <fstream>
#include <vector>
#include <string>

int main(int argc, char **argv)
{
	::setlocale(LC_ALL, "C");


	if(argc <= 1)
	{
		log_err("No input file given.");
		return -1;
	}

	std::ifstream ifstr(argv[1]);
	if(!ifstr.is_open())
	{
		log_err("Cannot open \"", argv[1], "\".");
		return -1;
	}


	std::vector<double> vecQx, vecQy, vecQz, vecE;
	std::string strLine;

	while(std::getline(ifstr, strLine))
	{
		trim(strLine);
		if(strLine.length()==0 || strLine[0]=='#')
			continue;

		double dQx=0., dQy=0., dQz=0., dE=0.;
		std::istringstream istr(strLine);
		istr >> dQx >> dQy >> dQz >> dE;

		vecQx.push_back(dQx);
		vecQy.push_back(dQy);
		vecQz.push_back(dQz);
		vecE.push_back(dE);
	}

	log_info("Number of neutrons in file: ", vecQx.size());

	Resolution res = calc_res(vecQx.size(), vecQx.data(), vecQy.data(), vecQz.data(), vecE.data());
	if(!res.bHasRes)
	{
		log_err("Cannot calculate resolution matrix.");
		return -1;
	}


	QLocale::setDefault(QLocale::English);
	QApplication app(argc, argv);

	EllipseDlg dlg(0);
	dlg.show();
	dlg.SetParams(res.res, res.Q_avg);
	int iRet = app.exec();


	return iRet;
}
