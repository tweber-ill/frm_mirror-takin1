// test prefactor of shirane formula A 4.65

#include "../../tlibs/math/neutrons.hpp"
using namespace tl;
using namespace tl::co;

int main()
{
	auto factor = hbar*hbar / m_n;
	std::cout << "Factor: " << factor / meV / angstrom / angstrom
				<< " meV A^2"
				<< std::endl;

	return 0;
}
