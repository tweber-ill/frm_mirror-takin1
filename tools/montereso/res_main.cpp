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
#include <unordered_map>
#include <string>

static void add_param(std::unordered_map<std::string, std::string>& map, const std::string& strLine)
{
	std::string str = strLine.substr(1);

	std::pair<std::string, std::string> pair = split_first<std::string>(str, ":", true);
	if(pair.first == "Param")
	{
		std::pair<std::string, std::string> pairParam = split_first<std::string>(pair.second, "=", true);
		pair.first = pair.first + "_" + pairParam.first;
		pair.second = pairParam.second;
	}

	map.insert(pair);
}

template<class t_map>
static void print_map(std::ostream& ostr, const t_map& map)
{
	for(typename t_map::const_iterator iter=map.begin(); iter!=map.end(); ++iter)
		ostr << (*iter).first << ": " << (*iter).second << "\n";
}

enum class FileType
{
	NEUTRON_Q_LIST,
	NEUTRON_KIKF_LIST,

	UNKNOWN
};

static bool load_mc_list(const char* pcFile, Resolution& res)
{
	FileType ft = FileType::NEUTRON_Q_LIST;

	std::ifstream ifstr(pcFile);
	if(!ifstr.is_open())
	{
		log_err("Cannot open \"", pcFile, "\".");
		return 0;
	}

	// neutron Q list
	std::vector<double> vecQx, vecQy, vecQz, vecE;

	// neutron ki, kf list
	std::vector<double> vecKi[3], vecKf[3], vecPos[3];
	std::vector<double> vecPi, vecPf;
	std::string strLine;

	std::unordered_map<std::string, std::string> mapParams;

	bool bEndOfHeader = 0;

	unsigned int uiNumNeutr = 0;
	while(std::getline(ifstr, strLine))
	{
		trim(strLine);
		if(strLine.length()==0)
			continue;
		else if(strLine[0]=='#')
		{
			add_param(mapParams, strLine);
			continue;
		}

		if(!bEndOfHeader)
		{
			bEndOfHeader = 1;

			try
			{
				if(mapParams.at("variables") == "ki_x ki_y ki_z kf_x kf_y kf_z x y z p_i p_f")
				{
					log_info("File is a ki, kf list.");
					ft = FileType::NEUTRON_KIKF_LIST;
				}
			}
			catch(const std::out_of_range& ex)
			{
				log_info("File is a Q list.");
				ft = FileType::NEUTRON_Q_LIST;
			}
		}

		std::istringstream istr(strLine);

		if(ft == FileType::NEUTRON_Q_LIST)
		{
			double dQx=0., dQy=0., dQz=0., dE=0.;

			istr >> dQx >> dQy >> dQz >> dE;

			vecQx.push_back(dQx);
			vecQy.push_back(dQy);
			vecQz.push_back(dQz);
			vecE.push_back(dE);
		}
		else if(ft == FileType::NEUTRON_KIKF_LIST)
		{
			double dKi[3], dKf[3], dPos[3], dPi=0., dPf=0.;

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

		++uiNumNeutr;
	}

	log_info("Number of neutrons in file: ", uiNumNeutr);
	//print_map(std::cout, mapParams);

	if(ft == FileType::NEUTRON_Q_LIST)
		res = calc_res(vecQx.size(), vecQx.data(), vecQy.data(), vecQz.data(), vecE.data());
	else if(ft == FileType::NEUTRON_KIKF_LIST)
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

static EllipseDlg* show_ellipses(const Resolution& res)
{
	EllipseDlg* pdlg = new EllipseDlg(0);
	pdlg->show();
	pdlg->SetParams(res.res, res.Q_avg);

	return pdlg;
}


int main(int argc, char **argv)
{
	::setlocale(LC_ALL, "C");
	if(argc <= 1)
	{
		log_err("No input file given.");
		std::cout << "Usage: " << argv[0] << " <file>" << std::endl;
		return -1;
	}

	const char* pcFile = argv[argc-1];

	/*for(int iArg=1; iArg<argc; ++iArg)
	{
		if(strcmp(argv[iArg], "-m") == 0)
			ft = FileType::NEUTRON_KIKF_LIST;
	}*/

	Resolution res;

	log_info("Loading neutron list from \"", pcFile, "\".");
	if(!load_mc_list(pcFile, res))
		return -1;


	QLocale::setDefault(QLocale::English);
	QApplication app(argc, argv);

	EllipseDlg* pElliDlg = show_ellipses(res);
	int iRet = app.exec();

	if(pElliDlg) delete pElliDlg;
	return iRet;
}
