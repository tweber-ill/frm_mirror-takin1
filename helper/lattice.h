/*
 * Bravais Lattice Calculations
 * @author tweber
 * @date 13-feb-2014
 */

#ifndef __LATTICE_H__
#define __LATTICE_H__

#include "linalg.h"
#include "math.h"


template<typename T=double>
bool reciprocal(const ublas::matrix<T>& matReal, ublas::matrix<T>& matRecip)
{
	ublas::matrix<double> matInv;
	if(!inverse<double>(ublas::trans(matReal), matInv))
		return false;

	matRecip = 2.*M_PI*matInv;
	return true;
}


class Lattice
{
	protected:
		ublas::vector<double> m_vecs[3];

	public:
		Lattice(double a, double b, double c, double alpha, double beta, double gamma);
		Lattice(const ublas::vector<double>& vec0,
				const ublas::vector<double>& vec1,
				const ublas::vector<double>& vec2);
		Lattice(const Lattice& lattice);
		Lattice();
		virtual ~Lattice();

		bool IsInited() const { return m_vecs[0].size()!=0; }

		// Euler ZXZ rotation
		void RotateEuler(double dPhi, double dTheta, double dPsi);
		// Euler vecRecipZ vecRecipX vecRecipZ rotation
		void RotateEulerRecip(const ublas::vector<double>& vecRecipX,
						const ublas::vector<double>& vecRecipY,
						const ublas::vector<double>& vecRecipZ,
						double dPhi, double dTheta, double dPsi);

		Lattice GetRecip() const;

		ublas::vector<double> GetPos(double h, double k, double l) const;
		ublas::vector<double> GetHKL(const ublas::vector<double>& vec) const;

		double GetAlpha() const;
		double GetBeta() const;
		double GetGamma() const;

		double GetA() const;
		double GetB() const;
		double GetC() const;

		double GetVol() const;

		const ublas::vector<double>& GetVec(unsigned int i) const { return m_vecs[i]; }
		ublas::matrix<double> GetMetric() const;
};

extern bool get_tas_angles(const Lattice& lattice_real,
						const ublas::vector<double>& vec1, const ublas::vector<double>& vec2,
						double dKi, double dKf,
						double dh, double dk, double dl,
						double *pTheta, double *pTwoTheta);

#endif
