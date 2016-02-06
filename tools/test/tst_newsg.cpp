// gcc -DNO_QT -I../.. -o tst_newsg tst_newsg.cpp ../../helper/spacegroup.cpp ../../helper/crystalsys.cpp ../../tlibs/log/log.cpp -lstdc++ -std=c++11 -lm

#include <iostream>
#include "../../helper/spacegroup.h"

void tst_sg()
{
	const t_mapSpaceGroups *pSGs = get_space_groups();
	for(const t_mapSpaceGroups::value_type& sg : *pSGs)
	{
		std::cout << sg.second.GetName() << ": ";
		std::vector<ublas::matrix<double>> vecTrafos = sg.second.GetTrafos();
		std::cout << vecTrafos.size() << " trafos" << std::endl;
	}

}

int main()
{
	init_space_groups();
	tst_sg();

	return 0;
}
