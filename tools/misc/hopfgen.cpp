// gcc -o hopfgen hopfgen.cpp -std=c++11 -lstdc++ -lm

#include "../../tlibs/math/linalg.h"
#include <iostream>

namespace ublas = boost::numeric::ublas;

int main()
{
	unsigned iMaxOrder = 3;
	double dStartAngle = 30.;
	double dIncAngle = 60.;
	double dScale = 0.028;

	double dStartInt = 0.05;
	double dIntScale = 0.01;

	double dSigQ = 0.002;
	double dSigE = 0.01;

	ublas::vector<double> vecN = tl::make_vec({1., 0., 0.});
	ublas::vector<double> vecY = tl::make_vec({0.,-1.,0.});
	//ublas::vector<double> vecN = tl::make_vec({1., 1., 0.});
	//ublas::vector<double> vecY = tl::make_vec({1.,-1.,0.});
	ublas::vector<double> vecX = vecN;

	vecX /= ublas::norm_2(vecX);
	vecY /= ublas::norm_2(vecY);


	std::cout << "# nuclear peak\n";
	std::cout << vecN[0] << " " << vecN[1] << " " << vecN[2]
		<< "  " << dSigQ << " " << dSigE << "  1\n\n";

	for(unsigned iOrder=0; iOrder<iMaxOrder; ++iOrder)
	{
		std::cout << "# hopf peaks, order " << iOrder << "\n";

		ublas::vector<double> vecLast;
		bool bHasLast = 0;
		for(double dAngle=dStartAngle; dAngle<=dStartAngle+360.; dAngle+=dIncAngle)
		{
			ublas::vector<double> vec =
				std::cos(dAngle/180.*M_PI)*vecY +
				std::sin(dAngle/180.*M_PI)*vecX;

			vec *= dScale*double(iOrder+1);
			vec += vecN;
			double dIntensity = dStartInt * std::pow(dIntScale, iOrder);

			if(dAngle < dStartAngle + 360.)
			{
				std::cout << vec[0] << " " << vec[1] << " " << vec[2]
					<< "  " << dSigQ << " " << dSigE << "  " << dIntensity << "\n";
			}

			if(bHasLast)
			for(unsigned iOrd=0; iOrd<iOrder; ++iOrd)
			{
				ublas::vector<double> vecBetween =
					vecLast + double(iOrd+1)*(vec - vecLast)/double(iOrder+1);

				std::cout << vecBetween[0] << " " << vecBetween[1] << " " << vecBetween[2]
					<< "  " << dSigQ << " " << dSigE << "  " << dIntensity << "\n";
			}

			vecLast = vec;
			bHasLast = 1;
		}

		std::cout << "\n";
	}

	std::cout.flush();
	return 0;
}
