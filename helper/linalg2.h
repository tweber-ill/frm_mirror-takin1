/*
 * advanced linalg helpers
 *
 * @author: tweber
 * @date: 30-apr-2013
 */

#ifndef __MIEZE_LINALG2__
#define __MIEZE_LINALG2__


#include "math.h"
#include "linalg.h"


template<typename T=double>
bool qr(const ublas::matrix<T>& M, ublas::matrix<T>& Q, ublas::matrix<T>& R)
{
	log_err("No specialisation of \"qr\" available for this type.");
	return false;
}

template<>
bool qr<double>(const ublas::matrix<double>& M,
		ublas::matrix<double>& Q,
		ublas::matrix<double>& R);


template<typename T=double>
bool eigenvec(const ublas::matrix<T>& mat, 
		std::vector<ublas::vector<T> >& evecs_real, std::vector<ublas::vector<T> >& evecs_imag, 
		std::vector<T>& evals_real, std::vector<T>& evals_imag)
{
	log_err("No specialisation of \"eigenvec\" available for this type.");
	return false;
}

template<typename T=double>
bool eigenvec_sym(const ublas::matrix<T>& mat, std::vector<ublas::vector<T> >& evecs, std::vector<T>& evals)
{
	log_err("No specialisation of \"eigenvec_sym\" available for this type.");
	return false;
}

template<>
bool eigenvec<double>(const ublas::matrix<double>& mat,
			std::vector<ublas::vector<double> >& evecs_real,
			std::vector<ublas::vector<double> >& evecs_imag,
			std::vector<double>& evals_real, 
			std::vector<double>& evals_imag);
template<>
bool eigenvec_sym<double>(const ublas::matrix<double>& mat,
			std::vector<ublas::vector<double> >& evecs,
			std::vector<double>& evals);


template<typename T=double>
void sort_eigenvecs(std::vector<ublas::vector<T> >& evecs,
					std::vector<T>& evals, bool bOrder=0, T (*pEvalFkt)(T)=0)
{
	if(evecs.size() != evals.size())
		return;

	struct Evec
	{
		ublas::vector<T> vec;
		T val;
	};

	std::vector<Evec> myevecs;
	myevecs.reserve(evecs.size());

	for(unsigned int i=0; i<evecs.size(); ++i)
	{
		Evec ev;
		ev.vec = evecs[i];
		ev.val = evals[i];

		myevecs.push_back(ev);
	}


	std::sort(myevecs.begin(), myevecs.end(),
			[&](const Evec& evec1, const Evec& evec2) -> bool
			{
				bool b;
				if(pEvalFkt)
					b = pEvalFkt(evec1.val) < pEvalFkt(evec2.val);
				else
					b = evec1.val < evec2.val;

				if(bOrder) b = !b;
				return b;
			});


	for(unsigned int i=0; i<evecs.size(); ++i)
	{
		evecs[i] = myevecs[i].vec;
		evals[i] = myevecs[i].val;
	}
}

#endif
