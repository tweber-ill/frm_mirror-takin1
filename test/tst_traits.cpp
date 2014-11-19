#include <iostream>
#include <vector>
#include <map>
#include "../helper/linalg.h"
#include "../helper/traits.h"


int main()
{
	std::cout << "vec: " << is_1d_type<std::vector<double>>::value << std::endl;
	std::cout << "ublas vec: " << is_1d_type<ublas::vector<double>>::value << std::endl;
	std::cout << "ublas mat: " << is_1d_type<ublas::matrix<double>>::value << std::endl;
	std::cout << "map: " << is_1d_type<std::map<int, double>>::value << std::endl;

	std::cout << "ublas mat: " << is_2d_type<ublas::matrix<double>>::value << std::endl;
	return 0;
}
