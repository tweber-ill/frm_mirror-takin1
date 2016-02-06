// gcc -DNO_QT -I../.. -o tst_refl tst_refl.cpp ../../helper/spacegroup_clp.cpp ../../helper/crystalsys.cpp ../../tlibs/log/log.cpp -lstdc++ -std=c++11 -lclipper-core -lm

#include <iostream>
#include "../../helper/spacegroup_clp.h"

void check_allowed_refls()
{
	const int HKL_MAX = 10;
	unsigned int iSG = 0;

	const t_mapSpaceGroups *pSGs = get_space_groups();
	for(const t_mapSpaceGroups::value_type& sg : *pSGs)
	{
		++iSG;
		//if(iSG < 200) continue;

		const std::string& strName = sg.second.GetName();
		std::cout << "Checking (" << iSG << ") " << strName << " ... ";

		for(int i=-HKL_MAX; i<=HKL_MAX; ++i)
		for(int j=-HKL_MAX; j<=HKL_MAX; ++j)
		for(int k=-HKL_MAX; k<=HKL_MAX; ++k)
		{
			if(sg.second.HasReflection(i,j,k) != sg.second.HasReflection2(i,j,k))
			{
				std::cout << "Failed at " << i << " " << j << " " << k << std::endl;
				return;
			}
		}

		std::cout << "OK" << std::endl;
	}
}

int main()
{
	init_space_groups();
	check_allowed_refls();

	return 0;
}
