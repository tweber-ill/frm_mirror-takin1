// gcc -I/usr/include/python2.7 -o tst_py test/tst_py.cpp monteconvo/sqw_py.cpp -std=c++11 -lstdc++ -lboost_python -lpython2.7

#include <iostream>
#include <iomanip>

#include "../monteconvo/sqw_py.h"
using t_var = SqwBase::t_var;


void print_vars(const SqwPy& sqw)
{
	std::vector<t_var> vecVars = sqw.GetVars();

	std::cout << std::endl;
	std::cout << std::setw(15) << std::left << "Variable"
		<< std::setw(15) << std::left << "Type"
		<< std::setw(15) << std::left << "Value" << std::endl;
	for(unsigned i=0; i<3*15; ++i)
		std::cout << "-";
	std::cout << std::endl;

	for(const t_var& var : vecVars)
	{
		std::cout << std::setw(15) << std::left << std::get<0>(var)
			<< std::setw(15) << std::left << std::get<1>(var)
			<< std::setw(15) << std::left << std::get<2>(var)
			<< std::endl;
	}
	std::cout << std::endl;
}

int main()
{
	SqwPy sqw("test/tstsqw.py");

	print_vars(sqw);
	double dS = sqw(1., 0., 0., 0.);
	std::cout << "S = " << dS << std::endl;

	std::vector<t_var> vecVars = sqw.GetVars();
	std::get<2>(vecVars[0]) = "987.6";
	sqw.SetVars(vecVars);

	print_vars(sqw);
	dS = sqw(1., 0., 0., 0.);
	std::cout << "S = " << dS << std::endl;

	return 0;
}
