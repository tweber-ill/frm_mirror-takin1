/*
 * Bravais Lattice Calculations
 * @author tweber
 * @date 13-feb-2014
 */

#include "lattice.h"

template class Lattice<double>;


template
void get_tas_angles<double>(const Lattice<double>& lattice_real,
						const ublas::vector<double>& _vec1, const ublas::vector<double>& _vec2,
						double dKi, double dKf,
						double dh, double dk, double dl,
						bool bSense,
						double *pTheta, double *pTwoTheta);

template
void get_hkl_from_tas_angles<double>(const Lattice<double>& lattice_real,
						const ublas::vector<double>& _vec1, const ublas::vector<double>& _vec2,
						double dm, double da, double th_m, double th_a, double _th_s, double tt_s,
						bool bSense_m, bool bSense_a, bool bSense_s,
						double* h, double* k, double* l,
						double*, double*, double*, double*);
