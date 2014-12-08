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


static bool load_mc_list(const char* pcFile, Resolution& res)
{
	std::ifstream ifstr(pcFile);
	if(!ifstr.is_open())
	{
		log_err("Cannot open \"", pcFile, "\".");
		return 0;
	}


	std::vector<double> vecKi[3], vecKf[3], vecPos[3];
	std::vector<double> vecPi, vecPf;
	std::string strLine;

	while(std::getline(ifstr, strLine))
	{
		trim(strLine);
		if(strLine.length()==0 || strLine[0]=='#')
			continue;

		double dKi[3], dKf[3], dPos[3], dPi=0., dPf=0.;

		std::istringstream istr(strLine);
		istr >> dKi[0] >> dKi[2] >> dKi[1];
		istr >> dKf[0] >> dKf[2] >> dKf[1];
		istr >> dPos[0] >> dPos[2] >> dPos[1];
		istr >> dPi >> dPf;

		dKi[1] = -dKi[1];
		dKf[1] = -dKf[1];
		dPos[1] = -dPos[1];
		
		for(short s=0; s<3; ++s)
		{
			vecKi[s].push_back(dKi[s]);
			vecKf[s].push_back(dKf[s]);
			vecPos[s].push_back(dPos[s]);
		}
		vecPi.push_back(dPi);
		vecPf.push_back(dPf);
	}

	log_info("Number of neutrons in file: ", vecPi.size());

	res = calc_res(vecPi.size(), 
					vecKi[0].data(), vecKi[1].data(), vecKi[2].data(),
					vecKf[0].data(), vecKf[1].data(), vecKf[2].data(),
					vecPi.data(), vecPf.data());
	if(!res.bHasRes)
	{
		log_err("Cannot calculate resolution matrix.");
		return 0;
	}
	
	return 1;	
}

static bool load_neutron_list(const char* pcFile, Resolution& res)
{
	std::ifstream ifstr(pcFile);
	if(!ifstr.is_open())
	{
		log_err("Cannot open \"", pcFile, "\".");
		return 0;
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

	res = calc_res(vecQx.size(), vecQx.data(), vecQy.data(), vecQz.data(), vecE.data());
	if(!res.bHasRes)
	{
		log_err("Cannot calculate resolution matrix.");
		return 0;
	}
	
	return 1;
}

static EllipseDlg* show_ellipses(const Resolution& res)
{
	EllipseDlg* pdlg = new EllipseDlg(0);
	pdlg->show();
	pdlg->SetParams(res.res, res.Q_avg);
	
	return pdlg;
}


enum class FileType
{
	NEUTRON_LIST,
	MONTE_CARLO,
	
	UNKNOWN
};

int main(int argc, char **argv)
{
	::setlocale(LC_ALL, "C");
	if(argc <= 1)
	{
		log_err("No input file given.");
		
		std::cout << "Usage: " << argv[0] << " [-m] <file>" << std::endl;
		return -1;
	}

	const char* pcFile = argv[argc-1];
	FileType ft = FileType::NEUTRON_LIST;
	
	for(int iArg=1; iArg<argc; ++iArg)
	{
		if(strcmp(argv[iArg], "-m") == 0)
			ft = FileType::MONTE_CARLO;
	}

	Resolution res;
	
	if(ft == FileType::NEUTRON_LIST)
	{
		log_info("Loading neutron list from \"", pcFile, "\".");
		if(!load_neutron_list(pcFile, res))
			return -1;
	}
	else if(ft == FileType::MONTE_CARLO)
	{
		log_info("Loading MC neutrons from \"", pcFile, "\".");
		if(!load_mc_list(pcFile, res))
			return -1;
	}
	else
	{
		log_err("Unknown file type.");
		return -1;
	}



	QLocale::setDefault(QLocale::English);
	QApplication app(argc, argv);

	EllipseDlg* pElliDlg = show_ellipses(res);
	int iRet = app.exec();

	if(pElliDlg) delete pElliDlg;
	return iRet;
}
