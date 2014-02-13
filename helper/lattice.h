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

		Lattice GetRecip() const;
};

#endif
