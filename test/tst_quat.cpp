// gcc -o tst_quat tst_quat.cpp -lstdc++ -lm -std=c++11

#include <iostream>

#include "../helper/linalg.h"
#include "../helper/quat.h"

int main()
{
	//ublas::matrix<double> rot = rotation_matrix(make_vec({1.,1.,1.}), 1.23);
	ublas::matrix<double> rot = rotation_matrix_3d_x(1.23);
	math::quaternion<double> quat = rot3_to_quat(rot);
	ublas::matrix<double> rot2 = quat_to_rot3(quat);

	std::cout << "rot: " << rot << std::endl;
	std::cout << "quat: " << quat << std::endl;
	std::cout << "rot2: " << rot2 << std::endl;



	math::quaternion<double> q1(0,1,0,0);
	math::quaternion<double> q2(0,0,1,0);

	std::cout << std::endl;
	std::cout << "slerp 0: " << slerp(q1, q2, 0.) << std::endl;
	std::cout << "slerp 0.5: " << slerp(q1, q2, 0.5) << std::endl;
	std::cout << "slerp 1: " << slerp(q1, q2, 1.) << std::endl;

	std::cout << std::endl;
	std::cout << "lerp 0: " << lerp(q1, q2, 0.) << std::endl;
	std::cout << "lerp 0.5: " << lerp(q1, q2, 0.5) << std::endl;
	std::cout << "lerp 1: " << lerp(q1, q2, 1.) << std::endl;


	std::cout << std::endl;
	math::quaternion<double> quat_s = stereo_proj(quat);
	std::cout << "stereo proj: " << quat_s << std::endl;
	std::cout << "inv stereo proj: " << stereo_proj_inv(quat_s) << std::endl;
	return 0;
}
