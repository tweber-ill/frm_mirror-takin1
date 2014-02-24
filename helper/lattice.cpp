/*
 * Bravais Lattice Calculations
 * @author tweber
 * @date 13-feb-2014
 */

#include "lattice.h"


Lattice::Lattice(double a, double b, double c,
				double alpha, double beta, double gamma)
{
	m_vecs[0].resize(3,0);
	m_vecs[1].resize(3,0);
	m_vecs[2].resize(3,0);

	skew_basis_from_angles(a,b,c, alpha,beta,gamma, m_vecs[0],m_vecs[1],m_vecs[2]);
}

Lattice::Lattice(const ublas::vector<double>& vec0,
				const ublas::vector<double>& vec1,
				const ublas::vector<double>& vec2)
{
	this->m_vecs[0] = vec0;
	this->m_vecs[1] = vec1;
	this->m_vecs[2] = vec2;
}

Lattice::Lattice(const Lattice& lattice)
{
	this->m_vecs[0] = lattice.m_vecs[0];
	this->m_vecs[1] = lattice.m_vecs[1];
	this->m_vecs[2] = lattice.m_vecs[2];
}

Lattice::~Lattice()
{}


double Lattice::GetAlpha() const
{ return std::acos(ublas::inner_prod(m_vecs[1]/GetB(), m_vecs[2]/GetC())); }
double Lattice::GetBeta() const
{ return std::acos(ublas::inner_prod(m_vecs[0]/GetA(), m_vecs[2]/GetC())); }
double Lattice::GetGamma() const
{ return std::acos(ublas::inner_prod(m_vecs[0]/GetA(), m_vecs[1]/GetB())); }

double Lattice::GetA() const
{ return std::sqrt(ublas::inner_prod(m_vecs[0], m_vecs[0])); }
double Lattice::GetB() const
{ return std::sqrt(ublas::inner_prod(m_vecs[1], m_vecs[1])); }
double Lattice::GetC() const
{ return std::sqrt(ublas::inner_prod(m_vecs[2], m_vecs[2])); }

double Lattice::GetVol() const
{
	return ::get_volume(column_matrix(
			std::vector<ublas::vector<double> >{m_vecs[0], m_vecs[1], m_vecs[2]})
			);
}

ublas::vector<double> Lattice::GetPos(double h, double k, double l) const
{
	return h*m_vecs[0] + k*m_vecs[1] + l*m_vecs[2];
}

Lattice Lattice::GetRecip() const
{
	const unsigned int iDim=3;

	ublas::matrix<double> matReal(iDim,iDim);
	for(unsigned int iVec=0; iVec<iDim; ++iVec)
		for(unsigned int iComp=0; iComp<iDim; ++iComp)
			matReal(iComp, iVec) = m_vecs[iVec][iComp];

	ublas::matrix<double> matInv;
	if(!inverse<double>(ublas::trans(matReal), matInv))
		std::cerr << "Could not invert matrix." << std::endl;

	ublas::matrix<double> matRecip = 2.*M_PI*matInv;
	return Lattice(get_column(matRecip,0),
					get_column(matRecip,1),
					get_column(matRecip,2));
}


// gcc -o 0 helper/lattice.cpp -std=c++11 -lstdc++ -lm
/*
int main()
{
	double alpha = 60. / 180.*M_PI;
	double beta = 120. / 180.*M_PI;
	double gamma = 90. / 180.*M_PI;
	Lattice l(1., 2., 3., alpha, beta, gamma);

	std::cout << "a = " << l.GetA() << std::endl;
	std::cout << "b = " << l.GetB() << std::endl;
	std::cout << "c = " << l.GetC() << std::endl;

	std::cout << "alpha = " << l.GetAlpha()/M_PI*180. << std::endl;
	std::cout << "beta = " << l.GetBeta()/M_PI*180. << std::endl;
	std::cout << "gamma = " << l.GetGamma()/M_PI*180. << std::endl;

	std::cout << "vol = " << l.GetVol() << std::endl;


	Lattice rec = l.GetRecip();

	std::cout << "\nrecip a = " << rec.GetA() << std::endl;
	std::cout << "recip b = " << rec.GetB() << std::endl;
	std::cout << "recip c = " << rec.GetC() << std::endl;

	std::cout << "recip alpha = " << rec.GetAlpha()/M_PI*180. << std::endl;
	std::cout << "recip beta = " << rec.GetBeta()/M_PI*180. << std::endl;
	std::cout << "recip gamma = " << rec.GetGamma()/M_PI*180. << std::endl;

	std::cout << "recip vol = " << rec.GetVol() << std::endl;

	return 0;
}
*/
