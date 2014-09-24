// clang -o tst_cov tst_cov.cpp -lstdc++ -std=c++11

#include <iostream>
#include <vector>
#include <initializer_list>
#include <type_traits>
#include "../helper/linalg.h"


int main()
{
	std::vector<ublas::vector<double>> vals
	{
		make_vec({1.0, 2.0, 3.0}),
		make_vec({1.1, 2.2, 3.4}),
		make_vec({0.9, 1.8, 2.9}),
		make_vec({1.0, 2.1, 3.1}),
		make_vec({1.4, 2.1, 3.6}),
	};

	std::cout << "Measured values:\n";
	std::copy(vals.begin(), vals.end(), std::ostream_iterator<decltype(vals[0])>(std::cout,"\n"));
	std::cout << std::endl;

	ublas::matrix<double> mat = covariance(vals);
	std::cout << "Covarance:\n" << mat << std::endl;

	return 0;
}
