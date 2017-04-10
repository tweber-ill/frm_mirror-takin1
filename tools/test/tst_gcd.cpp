/**
 * @author Tobias Weber <tobias.weber@tum.de>
 * @license GPLv2
 */

#include "../helper/linalg.h"

int main()
{
	ublas::vector<int> vec = make_vec({12,6,12,-6,2});
	std::cout << get_gcd_vec(vec) << std::endl;
	return 0;
}
