/*
 * advanced linalg helpers (which depend on lapack/e)
 *
 * @author: tweber
 * @date: 30-apr-2013
 */

#include "linalg2.h"
#include <memory>

extern "C"
{
	#include <lapacke.h>
	//#include <mkl_lapacke.h>
}

template<>
bool eigenvec<double>(const ublas::matrix<double>& mat,
				std::vector<ublas::vector<double> >& evecs_real,
				std::vector<ublas::vector<double> >& evecs_imag,
				std::vector<double>& evals_real,
				std::vector<double>& evals_imag)
{
	typedef double T;

	if(mat.size1() != mat.size2())
		return false;
	if(mat.size1()==0 || mat.size1()==1)
		return false;

	const unsigned int iOrder = mat.size1();
	evecs_real.resize(iOrder);
	evecs_imag.resize(iOrder);
	evals_real.resize(iOrder);
	evals_imag.resize(iOrder);
	for(unsigned int i=0; i<iOrder; ++i)
	{
		evecs_real[i].resize(iOrder);
		evecs_imag[i].resize(iOrder);
	}

/*
	// is matrix already diagonal?
	if(is_diag_matrix(mat))
	{
		for(unsigned int i=0; i<iOrder; ++i)
		{
			evals_real[i] = mat(i,i);

			evecs_real[i] = ublas::zero_vector<T>(iOrder);
			evecs_real[i][i] = 1.;


			evals_imag[i] = 0.;
			evecs_imag[i] = ublas::zero_vector<T>(iOrder);
		}

		return true;
	}
*/

	bool bOk = true;

	std::unique_ptr<T, std::default_delete<T[]>> 
		uptrMem(new T[iOrder*iOrder + iOrder*iOrder + iOrder*iOrder + iOrder + iOrder]);
	T *pMem = uptrMem.get();

	T *pMatrix = pMem;
	for(unsigned int i=0; i<iOrder; ++i)
		for(unsigned int j=0; j<iOrder; ++j)
			pMatrix[i*iOrder + j] = mat(i,j);

	T *p_eigenvecs = pMatrix + iOrder*iOrder;
	//T *p_eigenvecs_l = p_eigenvecs + iOrder*iOrder;
	T *peigenvals_real = p_eigenvecs/*_l*/ + iOrder*iOrder;
	T *peigenvals_imag = peigenvals_real + iOrder;

	int iInfo = LAPACKE_dgeev(LAPACK_ROW_MAJOR, 'N', 'V', iOrder, (T*)pMatrix, iOrder,
					(T*)peigenvals_real, (T*)peigenvals_imag,
					/*(double*)p_eigenvecs_l*/0, iOrder, (T*)p_eigenvecs, iOrder);

	if(iInfo!=0)
	{
		log_err("Could not solve eigenproblem", " (lapack error ", iInfo , ").");
		bOk = false;
	}

	for(unsigned int i=0; i<iOrder; ++i)
	{
		bool bIsReal = 0;
		if(float_equal<T>(peigenvals_imag[i], 0.))
			bIsReal = 1;

		if(bIsReal)
		{
			for(unsigned int j=0; j<iOrder; ++j)
			{
				evecs_real[i][j] = p_eigenvecs[j*iOrder + i];
				evecs_imag[i][j] = 0.;

				evals_real[i] = peigenvals_real[i];
				evals_imag[i] = peigenvals_imag[i];
			}
		}
		else
		{
			for(unsigned int j=0; j<iOrder; ++j)
			{
				evecs_real[i][j] = p_eigenvecs[j*iOrder + i];
				evecs_imag[i][j] = p_eigenvecs[j*iOrder + i+1];

				evecs_real[i+1][j] = p_eigenvecs[j*iOrder + i];
				evecs_imag[i+1][j] = -p_eigenvecs[j*iOrder + i+1];
			}

			evals_real[i] = peigenvals_real[i];
			evals_imag[i] = peigenvals_imag[i];
			evals_real[i+1] = peigenvals_real[i+1];
			evals_imag[i+1] = peigenvals_imag[i+1];

			++i; // check: (next eigenval) == -(currrent eigenval)
		}

		//evecs_real[i] /= ublas::norm_2(evecs_real[i]);
		//evecs_imag[i] /= ublas::norm_2(evecs_imag[i]);
	}

	return bOk;
}

template<>
bool eigenvec_sym<double>(const ublas::matrix<double>& mat,
				std::vector<ublas::vector<double> >& evecs,
				std::vector<double>& evals)
{
	typedef double T;

	if(mat.size1() != mat.size2())
		return false;
	if(mat.size1()==0 || mat.size1()==1)
		return false;

	const unsigned int iOrder = mat.size1();
	evecs.resize(iOrder);
	evals.resize(iOrder);
	for(unsigned int i=0; i<iOrder; ++i)
			evecs[i].resize(iOrder);


	bool bOk = true;

	std::unique_ptr<T, std::default_delete<T[]>> 
		uptrMem(new T[iOrder*iOrder + iOrder]);
	T *pMem = uptrMem.get();
	T *pMatrix = pMem;

	for(unsigned int i=0; i<iOrder; ++i)
			for(unsigned int j=0; j<iOrder; ++j)
					pMatrix[i*iOrder + j] = mat(i,j);

	T *peigenvals_real = pMatrix + iOrder*iOrder;
	int iInfo = LAPACKE_dsyev(LAPACK_ROW_MAJOR, 'V', 'U',
					iOrder, (T*)pMatrix, iOrder,
					(T*)peigenvals_real);

	if(iInfo!=0)
	{
		log_err("Could not solve eigenproblem", " (lapack error ", iInfo, ").");
		bOk = false;
	}

	for(unsigned int i=0; i<iOrder; ++i)
	{
		for(unsigned int j=0; j<iOrder; ++j)
				evecs[i][j] = pMatrix[j*iOrder + i];
		//evecs[i] /= ublas::norm_2(evecs[i]);
		evals[i] = peigenvals_real[i];
	}

	if(determinant<ublas::matrix<T>>(column_matrix(evecs)) < 0.)
		evecs[0] = -evecs[0];

	return bOk;
}


template<>
bool qr<double>(const ublas::matrix<double>& M,
				ublas::matrix<double>& Q,
				ublas::matrix<double>& R)
{
	const typename ublas::matrix<double>::size_type m = M.size1();
	const typename ublas::matrix<double>::size_type n = M.size2();

	const unsigned int iTauSize = m;//std::min<unsigned int>(m,n);

	std::unique_ptr<double, std::default_delete<double[]>> 
		uptrMem(new double[n*m + iTauSize]);
	double *pMem = uptrMem.get();

	double *pMat = pMem;
	double *pTau = pMem + n*m;

	for(unsigned int i=0; i<m; ++i)
		for(unsigned int j=0; j<n; ++j)
			pMat[i*n + j] = M(i,j);

	// see: http://www.math.utah.edu/software/lapack/lapack-d/dgeqrf.html
	int iInfo = LAPACKE_dgeqrf(LAPACK_ROW_MAJOR, m, n, pMat, n, pTau);
	//std::cout << "dgeqrt: " << iInfo << std::endl;

	R = ublas::matrix<double>(m,n);
	for(unsigned int i=0; i<m; ++i)
		for(unsigned int j=0; j<n; ++j)
		{
			if(j>=i)
				R(i,j) = pMat[i*n + j];
			else
				R(i,j) = 0.;
		}
	//std::cout << "R = " << R << std::endl;

	ublas::vector<double> v(iTauSize);

	const ublas::matrix<double> ident = ublas::identity_matrix<double>(iTauSize);
	Q = ident;

	for(unsigned int k=1; k<=iTauSize; ++k)
	{
		double dTau = pTau[k-1];
		//std::cout << "tau " << k << " = " << dTau << std::endl;

		for(unsigned int i=1; i<=k-1; ++i)
			v[i-1] = 0.;
		v[k-1] = 1.;

		for(unsigned int i=k+1; i<=iTauSize; ++i)
			v[i-1] = pMat[(i-1)*n + (k-1)];

		ublas::matrix<double> VV = ublas::outer_prod(v, ublas::trans(v));
		ublas::matrix<double> H = ident - dTau*VV;

		Q = ublas::prod(Q, H);
	}

	//std::cout << "Q = " << Q << std::endl;
	return (iInfo==0);
}
