/**
 * @author Tobias Weber <tobias.weber@tum.de>
 * @license GPLv2
 */

#include "../helper/linalg.h"
#include "../helper/quat.h"
#include <iostream>

int main()
{
	ublas::matrix<double> mat = rotation_matrix_2d(M_PI/2.);
	std::cout << mat << std::endl;

	std::cout << rotation_angle(mat)[0] << std::endl;

	return 0;
}
