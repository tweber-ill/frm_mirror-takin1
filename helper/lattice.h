/*
 * Bravais Lattice Calculations
 * @author tweber
 * @date 13-feb-2014
 */

#ifndef __LATTICE_H__
#define __LATTICE_H__

#include "linalg.h"
#include "math.h"

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
		virtual ~Lattice();

		// Euler ZXZ rotation
		void RotateEuler(double dPhi, double dTheta, double dPsi);

		Lattice GetRecip() const;

		ublas::vector<double> GetPos(double h, double k, double l) const;

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

#endif
