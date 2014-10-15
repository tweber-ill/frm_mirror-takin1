/*
 * Bravais Lattice Calculations
 * @author tweber
 * @date 13-feb-2014
 */

#include "lattice.h"
#include "neutrons.hpp"

Lattice::Lattice()
{}

Lattice::Lattice(double a, double b, double c,
				double alpha, double beta, double gamma)
{
	m_vecs[0].resize(3,0);
	m_vecs[1].resize(3,0);
	m_vecs[2].resize(3,0);

	fractional_basis_from_angles(a,b,c, alpha,beta,gamma, m_vecs[0],m_vecs[1],m_vecs[2]);
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

void Lattice::RotateEuler(double dPhi, double dTheta, double dPsi)
{
	ublas::matrix<double> mat1 = ::rotation_matrix_3d_z(dPhi);
	ublas::matrix<double> mat2 = ::rotation_matrix_3d_x(dTheta);
	ublas::matrix<double> mat3 = ::rotation_matrix_3d_z(dPsi);

	ublas::matrix<double> mat21 = ublas::prod(mat2,mat1);
	ublas::matrix<double> mat = ublas::prod(mat3, mat21);

	for(unsigned int i=0; i<3; ++i)
		m_vecs[i] = ublas::prod(mat, m_vecs[i]);
}

void Lattice::RotateEulerRecip(const ublas::vector<double>& vecRecipX,
				const ublas::vector<double>& vecRecipY,
				const ublas::vector<double>& vecRecipZ,
				double dPhi, double dTheta, double dPsi)
{
	// get real vectors
	const unsigned int iDim=3;
	ublas::matrix<double> matReal =
					column_matrix(std::vector<ublas::vector<double> >
								{vecRecipX, vecRecipY, vecRecipZ});
	if(matReal.size1()!=matReal.size2() || matReal.size1()!=iDim)
		throw Err("Invalid real matrix.");

	ublas::matrix<double> matRecip;
	if(!reciprocal(matReal, matRecip))
		throw Err("Reciprocal matrix could not be calculated.");

	ublas::vector<double> vecX = get_column(matRecip,0);
	ublas::vector<double> vecY = get_column(matRecip,1);
	ublas::vector<double> vecZ = get_column(matRecip,2);

	double dLenX = ublas::norm_2(vecX);
	double dLenY = ublas::norm_2(vecY);
	double dLenZ = ublas::norm_2(vecZ);

	if(float_equal(dLenX, 0.) || float_equal(dLenY, 0.) || float_equal(dLenZ, 0.)
		|| ::isnan(dLenX) || ::isnan(dLenY) || ::isnan(dLenZ))
	{
		throw Err("Invalid reciprocal matrix.");
		return;
	}

	vecX /= dLenX;
	vecY /= dLenY;
	vecZ /= dLenZ;

	//std::cout << "x = " << vecX << std::endl;
	//std::cout << "y = " << vecY << std::endl;
	//std::cout << "z = " << vecZ << std::endl;


	// rotate around real vectors
	ublas::matrix<double> mat1 = ::rotation_matrix(vecZ, dPhi);
	ublas::matrix<double> mat2 = ::rotation_matrix(vecX, dTheta);
	ublas::matrix<double> mat3 = ::rotation_matrix(vecZ, dPsi);

	ublas::matrix<double> mat21 = ublas::prod(mat2,mat1);
	ublas::matrix<double> mat = ublas::prod(mat3, mat21);

	for(unsigned int i=0; i<3; ++i)
		m_vecs[i] = ublas::prod(mat, m_vecs[i]);
}

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

/*
 (x)   (v0_x v1_x v2_x) (h)
 (y) = (v0_y v1_y v2_y) (k)
 (z)   (v0_z v1_z v2_z) (l)
 */
ublas::vector<double> Lattice::GetPos(double h, double k, double l) const
{
	return h*m_vecs[0] + k*m_vecs[1] + l*m_vecs[2];
}

/*
 (h)   (v0_x v1_x v2_x)^(-1) (x)
 (k) = (v0_y v1_y v2_y)      (y)
 (l)   (v0_z v1_z v2_z)      (z)
 */
ublas::vector<double> Lattice::GetHKL(const ublas::vector<double>& vec) const
{
	ublas::matrix<double> mat =
					column_matrix(std::vector<ublas::vector<double> >
								{m_vecs[0], m_vecs[1], m_vecs[2]});

	ublas::matrix<double> matInv;
	if(!inverse<double>(mat, matInv))
		throw Err("Miller indices could not be calculated.");

	return ublas::prod(matInv, vec);
}

Lattice Lattice::GetRecip() const
{
	const unsigned int iDim=3;
	ublas::matrix<double> matReal =
					column_matrix(std::vector<ublas::vector<double> >
								{m_vecs[0], m_vecs[1], m_vecs[2]});
	if(matReal.size1()!=matReal.size2() || matReal.size1()!=iDim)
		throw Err("Invalid real lattice matrix.");

	ublas::matrix<double> matRecip;
	if(!reciprocal(matReal, matRecip))
		throw Err("Reciprocal lattice could not be calculated.");

	return Lattice(get_column(matRecip,0),
					get_column(matRecip,1),
					get_column(matRecip,2));
}


ublas::matrix<double> Lattice::GetMetric() const
{
	std::vector<ublas::vector<double> > vecs = {m_vecs[0], m_vecs[1], m_vecs[2]};
	return column_matrix(vecs);
}



bool get_tas_angles(const Lattice& lattice_real,
					const ublas::vector<double>& _vec1, const ublas::vector<double>& _vec2,
					double dKi, double dKf,
					double dh, double dk, double dl,
					double *pTheta, double *pTwoTheta)
{
	using t_vec = ublas::vector<double>;
	using t_mat = ublas::matrix<double>;

	try
	{
		t_mat matB = lattice_real.GetRecip().GetMetric();


		t_vec vec1 = _vec1;
		t_vec vec2 = _vec2;

		t_vec vecUp = ::cross_3(vec1, vec2);
		vec2 = ::cross_3(vecUp, vec1);

		vec1 /= ublas::norm_2(vec1);
		vec2 /= ublas::norm_2(vec2);
		vecUp /= ublas::norm_2(vecUp);

		std::vector<ublas::vector<double> > vecs = {vec1, vec2, vecUp};
		t_mat matU = ::row_matrix(vecs);


		t_mat matUB = ublas::prod(matU, matB);


		t_vec vechkl = ::make_vec({dh, dk, dl});
		t_vec vecQ = ublas::prod(matUB, vechkl);
		double dQ = ublas::norm_2(vecQ);

		if(std::fabs(vecQ[2]) > 1e-6)
			throw Err("Position not in scattering plane.");

		*pTwoTheta = get_sample_twotheta(dKi/angstrom, dKf/angstrom, dQ/angstrom) / units::si::radians;
		double dKiQ = get_angle_ki_Q(dKi/angstrom, dKf/angstrom, dQ/angstrom) / units::si::radians;

		*pTheta = -dKiQ - std::atan2(vecQ[0], vecQ[1]) + M_PI;
	}
	catch(const std::exception& ex)
	{
		log_err(ex.what());
		return false;
	}

	return true;
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
