/*
 * Bravais Lattice Calculations
 * @author tweber
 * @date 13-feb-2014
 */

#include "lattice.h"

Lattice::Lattice(double a, double b, double c,
				double alpha, double beta, double gamma)
{

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
