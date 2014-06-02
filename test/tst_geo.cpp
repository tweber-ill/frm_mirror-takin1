// gcc -DUSE_LAPACK -o tst_geo tst_geo.cpp ../helper/linalg.cpp -std=c++11 -lstdc++ -lm -I/usr/include/lapacke -llapacke -llapack

#include "../helper/geo.h"

typedef ublas::vector<double> vec;

int main()
{
	vec v0(3), dir0_0(3), dir0_1(3);
	vec v1(3), dir1_0(3), dir1_1(3);

	v0[0] = 0.; v0[1] = 0.; v0[2] = 0.;
	v1[0] = 3.; v1[1] = 0.; v1[2] = 0.;

	dir0_0[0] = 1.; dir0_0[1] = 0.; dir0_0[2] = 0.;
	dir0_1[0] = 0.; dir0_1[1] = 1.; dir0_1[2] = 0.;

	dir1_0[0] = 0.; dir1_0[1] = 1.; dir1_0[2] = 0.;
	dir1_1[0] = 0.; dir1_1[1] = 0.; dir1_1[2] = 1.;

	Plane<double> plane0(v0, dir0_0, dir0_1);
	Plane<double> plane1(v1, dir1_0, dir1_1);

	std::cout << "Plane 0: " << plane0 << std::endl;
	std::cout << "Plane 1: " << plane1 << std::endl;

	Line<double> line;
	if(plane0.intersect(plane1, line))
		std::cout << "Line: " << line << std::endl;
	else
		std::cout << "Error." << std::endl;
	return 0;
}
