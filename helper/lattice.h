/*
 * Bravais Lattice Calculations
 * @author tweber
 * @date 13-feb-2014
 */

#ifndef __LATTICE_H__
#define __LATTICE_H__

#include "linalg.h"
#include "math.h"
#include "neutrons.hpp"


template<typename T=double>
bool reciprocal(const ublas::matrix<T>& matReal, ublas::matrix<T>& matRecip)
{
	ublas::matrix<double> matInv;
	if(!inverse<double>(ublas::trans(matReal), matInv))
		return false;

	matRecip = 2.*M_PI*matInv;
	return true;
}


template<typename T=double>
class Lattice
{
	protected:
		ublas::vector<T> m_vecs[3];

	public:
		Lattice(T a, T b, T c, T alpha, T beta, T gamma);
		Lattice(const ublas::vector<T>& vec0,
				const ublas::vector<T>& vec1,
				const ublas::vector<T>& vec2);
		Lattice(const Lattice<T>& lattice);
		Lattice();
		virtual ~Lattice();

		bool IsInited() const { return m_vecs[0].size()!=0; }

		// Euler ZXZ rotation
		void RotateEuler(T dPhi, T dTheta, T dPsi);
		// Euler vecRecipZ vecRecipX vecRecipZ rotation
		void RotateEulerRecip(const ublas::vector<T>& vecRecipX,
						const ublas::vector<T>& vecRecipY,
						const ublas::vector<T>& vecRecipZ,
						T dPhi, T dTheta, T dPsi);

		Lattice GetRecip() const;

		ublas::vector<T> GetPos(T h, T k, T l) const;
		ublas::vector<T> GetHKL(const ublas::vector<T>& vec) const;

		T GetAlpha() const;
		T GetBeta() const;
		T GetGamma() const;

		T GetA() const;
		T GetB() const;
		T GetC() const;

		T GetVol() const;

		const ublas::vector<T>& GetVec(unsigned int i) const { return m_vecs[i]; }
		ublas::matrix<T> GetMetric() const;
};

template<typename T> Lattice<T>::Lattice() {}
template<typename T> Lattice<T>::~Lattice() {}

template<typename T>
Lattice<T>::Lattice(T a, T b, T c, T alpha, T beta, T gamma)
{
	m_vecs[0].resize(3,0);
	m_vecs[1].resize(3,0);
	m_vecs[2].resize(3,0);

	fractional_basis_from_angles(a,b,c, alpha,beta,gamma, m_vecs[0],m_vecs[1],m_vecs[2]);
}

template<typename T>
Lattice<T>::Lattice(const ublas::vector<T>& vec0,
				const ublas::vector<T>& vec1,
				const ublas::vector<T>& vec2)
{
	this->m_vecs[0] = vec0;
	this->m_vecs[1] = vec1;
	this->m_vecs[2] = vec2;
}

template<typename T>
Lattice<T>::Lattice(const Lattice<T>& lattice)
{
	this->m_vecs[0] = lattice.m_vecs[0];
	this->m_vecs[1] = lattice.m_vecs[1];
	this->m_vecs[2] = lattice.m_vecs[2];
}

template<typename T>
void Lattice<T>::RotateEuler(T dPhi, T dTheta, T dPsi)
{
	ublas::matrix<T> mat1 = ::rotation_matrix_3d_z(dPhi);
	ublas::matrix<T> mat2 = ::rotation_matrix_3d_x(dTheta);
	ublas::matrix<T> mat3 = ::rotation_matrix_3d_z(dPsi);

	ublas::matrix<T> mat21 = ublas::prod(mat2,mat1);
	ublas::matrix<T> mat = ublas::prod(mat3, mat21);

	for(unsigned int i=0; i<3; ++i)
		m_vecs[i] = ublas::prod(mat, m_vecs[i]);
}

template<typename T>
void Lattice<T>::RotateEulerRecip(const ublas::vector<T>& vecRecipX,
				const ublas::vector<T>& vecRecipY,
				const ublas::vector<T>& vecRecipZ,
				T dPhi, T dTheta, T dPsi)
{
	// get real vectors
	const unsigned int iDim=3;
	ublas::matrix<T> matReal = column_matrix({vecRecipX, vecRecipY, vecRecipZ});
	if(matReal.size1()!=matReal.size2() || matReal.size1()!=iDim)
		throw Err("Invalid real matrix.");

	ublas::matrix<T> matRecip;
	if(!reciprocal(matReal, matRecip))
		throw Err("Reciprocal matrix could not be calculated.");

	ublas::vector<T> vecX = get_column(matRecip,0);
	ublas::vector<T> vecY = get_column(matRecip,1);
	ublas::vector<T> vecZ = get_column(matRecip,2);

	T dLenX = ublas::norm_2(vecX);
	T dLenY = ublas::norm_2(vecY);
	T dLenZ = ublas::norm_2(vecZ);

	if(float_equal(dLenX, 0.) || float_equal(dLenY, 0.) || float_equal(dLenZ, 0.)
		|| ::isnan(dLenX) || ::isnan(dLenY) || ::isnan(dLenZ))
	{
		throw Err("Invalid reciprocal matrix.");
	}

	vecX /= dLenX;
	vecY /= dLenY;
	vecZ /= dLenZ;

	//std::cout << "x = " << vecX << std::endl;
	//std::cout << "y = " << vecY << std::endl;
	//std::cout << "z = " << vecZ << std::endl;


	// rotate around real vectors
	ublas::matrix<T> mat1 = ::rotation_matrix(vecZ, dPhi);
	ublas::matrix<T> mat2 = ::rotation_matrix(vecX, dTheta);
	ublas::matrix<T> mat3 = ::rotation_matrix(vecZ, dPsi);

	ublas::matrix<T> mat21 = ublas::prod(mat2,mat1);
	ublas::matrix<T> mat = ublas::prod(mat3, mat21);

	for(unsigned int i=0; i<3; ++i)
		m_vecs[i] = ublas::prod(mat, m_vecs[i]);
}

template<typename T> T Lattice<T>::GetAlpha() const
{ return std::acos(ublas::inner_prod(m_vecs[1]/GetB(), m_vecs[2]/GetC())); }
template<typename T> T Lattice<T>::GetBeta() const
{ return std::acos(ublas::inner_prod(m_vecs[0]/GetA(), m_vecs[2]/GetC())); }
template<typename T> T Lattice<T>::GetGamma() const
{ return std::acos(ublas::inner_prod(m_vecs[0]/GetA(), m_vecs[1]/GetB())); }

template<typename T> T Lattice<T>::GetA() const
{ return std::sqrt(ublas::inner_prod(m_vecs[0], m_vecs[0])); }
template<typename T> T Lattice<T>::GetB() const
{ return std::sqrt(ublas::inner_prod(m_vecs[1], m_vecs[1])); }
template<typename T> T Lattice<T>::GetC() const
{ return std::sqrt(ublas::inner_prod(m_vecs[2], m_vecs[2])); }

template<typename T>
T Lattice<T>::GetVol() const
{
	return ::get_volume(column_matrix({m_vecs[0], m_vecs[1], m_vecs[2]}));
}

/*
 (x)   (v0_x v1_x v2_x) (h)
 (y) = (v0_y v1_y v2_y) (k)
 (z)   (v0_z v1_z v2_z) (l)
 */
template<typename T>
ublas::vector<T> Lattice<T>::GetPos(T h, T k, T l) const
{
	return h*m_vecs[0] + k*m_vecs[1] + l*m_vecs[2];
}

/*
 (h)   (v0_x v1_x v2_x)^(-1) (x)
 (k) = (v0_y v1_y v2_y)      (y)
 (l)   (v0_z v1_z v2_z)      (z)
 */
template<typename T>
ublas::vector<T> Lattice<T>::GetHKL(const ublas::vector<T>& vec) const
{
	ublas::matrix<T> mat = column_matrix({m_vecs[0], m_vecs[1], m_vecs[2]});

	ublas::matrix<T> matInv;
	if(!inverse<T>(mat, matInv))
		throw Err("Miller indices could not be calculated.");

	return ublas::prod(matInv, vec);
}

template<typename T>
Lattice<T> Lattice<T>::GetRecip() const
{
	const unsigned int iDim=3;
	ublas::matrix<T> matReal = column_matrix({m_vecs[0], m_vecs[1], m_vecs[2]});
	if(matReal.size1()!=matReal.size2() || matReal.size1()!=iDim)
		throw Err("Invalid real lattice matrix.");

	ublas::matrix<T> matRecip;
	if(!reciprocal(matReal, matRecip))
		throw Err("Reciprocal lattice could not be calculated.");

	return Lattice<T>(get_column(matRecip,0),
					get_column(matRecip,1),
					get_column(matRecip,2));
}

template<typename T>
ublas::matrix<T> Lattice<T>::GetMetric() const
{
	return column_matrix({m_vecs[0], m_vecs[1], m_vecs[2]});
}



template<typename T=double>
ublas::matrix<T> get_UB(const Lattice<T>& lattice_real,
						const ublas::vector<T>& _vec1,
						const ublas::vector<T>& _vec2)
{
	using t_vec = ublas::vector<T>;
	using t_mat = ublas::matrix<T>;

	t_mat matB = lattice_real.GetRecip().GetMetric();


	t_vec vec1 = _vec1;
	t_vec vec2 = _vec2;

	t_vec vecUp = ::cross_3(vec1, vec2);
	vec2 = ::cross_3(vecUp, vec1);

	vec1 /= ublas::norm_2(vec1);
	vec2 /= ublas::norm_2(vec2);
	vecUp /= ublas::norm_2(vecUp);


	t_mat matU = ::row_matrix({vec1, vec2, vecUp});
	t_mat matUB = ublas::prod(matU, matB);

	return matUB;
}


template<typename T=double>
void get_tas_angles(const Lattice<T>& lattice_real,
						const ublas::vector<T>& _vec1, const ublas::vector<T>& _vec2,
						T dKi, T dKf,
						T dh, T dk, T dl,
						bool bSense,
						T *pTheta, T *pTwoTheta)
{
	using t_vec = ublas::vector<T>;
	using t_mat = ublas::matrix<T>;

	//try
	{
		t_mat matUB = get_UB(lattice_real, _vec1, _vec2);

		t_vec vechkl = ::make_vec({dh, dk, dl});
		t_vec vecQ = ublas::prod(matUB, vechkl);
		double dQ = ublas::norm_2(vecQ);

		if(std::fabs(vecQ[2]) > 1e-6)
			throw Err("Position not in scattering plane.");

		*pTwoTheta = get_sample_twotheta(dKi/angstrom, dKf/angstrom, dQ/angstrom, bSense) / units::si::radians;
		T dKiQ = get_angle_ki_Q(dKi/angstrom, dKf/angstrom, dQ/angstrom, /*bSense*/1) / units::si::radians;
		vecQ.resize(2,true);

		T dAngleKiOrient1 = -dKiQ - vec_angle(vecQ);
		*pTheta = dAngleKiOrient1 + M_PI;		// a3 convention
		*pTheta -= M_PI/2.;				// theta here
		if(!bSense) *pTheta = -*pTheta;
	}
	/*catch(const std::exception& ex)
	{
		log_err(ex.what());
	}*/
}

template<typename T=double>
void get_hkl_from_tas_angles(const Lattice<T>& lattice_real,
						const ublas::vector<T>& _vec1, const ublas::vector<T>& _vec2,
						T dm, T da, T th_m, T th_a, T _th_s, T _tt_s,
						bool bSense_m, bool bSense_a, bool bSense_s,
						T* h, T* k, T* l,
						T* pki=0, T* pkf=0, T* pE=0, T* pQ=0)
{
	using t_vec = ublas::vector<T>;
	using t_mat = ublas::matrix<T>;

	T th_s = _th_s;
	T tt_s = _tt_s;
	if(!bSense_s)
	{
		th_s = -th_s;
		tt_s = -tt_s;
	}

	T ki = ::get_mono_k(th_m*units::si::radians, dm*angstrom, bSense_m)*angstrom;
	T kf = ::get_mono_k(th_a*units::si::radians, da*angstrom, bSense_a)*angstrom;
	T E = ::get_energy_transfer(ki/angstrom, kf/angstrom) / one_meV;
	T Q = ::get_sample_Q(ki/angstrom, kf/angstrom, tt_s*units::si::radians)*angstrom;
	T kiQ = ::get_angle_ki_Q(ki/angstrom, kf/angstrom, Q/angstrom, /*bSense_s*/1) / units::si::radians;

	th_s += M_PI/2.;					// theta here
	T Qvec1 = M_PI - th_s - kiQ;		// a3 convention


	t_mat matUB = get_UB(lattice_real, _vec1, _vec2);
	t_mat matUBinv;
	if(!::inverse(matUB, matUBinv))
		throw Err("Cannot invert UB.");

	t_mat rot = ::rotation_matrix_3d_z(Qvec1);
	t_vec vecQ = ublas::prod(rot, make_vec({Q,0.,0.}));
	t_vec vechkl = ublas::prod(matUBinv, vecQ);

	if(vechkl.size() != 3)
		throw Err("Cannot determine hkl.");

	*h = vechkl[0];
	*k = vechkl[1];
	*l = vechkl[2];

	if(pki) *pki = ki;
	if(pkf) *pkf = kf;
	if(pE) *pE = E;
	if(pQ) *pQ = Q;
}

#endif
