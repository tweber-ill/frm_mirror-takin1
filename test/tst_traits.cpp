#include <iostream>
#include <vector>
#include <map>
#include "../helper/linalg.h"
#include "../helper/traits.h"


int main()
{
	std::cout << "vec: " << get_type_dim<std::vector<double>>::value << std::endl;
	std::cout << "ublas vec: " << get_type_dim<ublas::vector<double>>::value << std::endl;
	std::cout << "ublas mat: " << get_type_dim<ublas::matrix<double>>::value << std::endl;
	std::cout << "map: " << get_type_dim<std::map<int, double>>::value << std::endl;
	return 0;
}
