/*
 * linalg helpers
 *
 * @author: tweber
 * @date: 30-apr-2013
 */

#include "linalg.h"
#include "math.h"

#ifdef USE_LAPACK
extern "C"
{
        #include <lapacke.h>
        //#include <mkl_lapacke.h>
}

template<>
bool eigenvec<double>(const ublas::matrix<double>& mat,
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


	// is matrix already diagonal?
	if(is_diag_matrix(mat))
	{
		for(unsigned int i=0; i<iOrder; ++i)
		{
			evals[i] = mat(i,i);

			evecs[i] = ublas::zero_vector<T>(iOrder);
			evecs[i][i] = 1.;
		}

		return true;
	}


	bool bOk = true;

	T *pMem = new T[iOrder*iOrder + iOrder*iOrder + iOrder*iOrder + iOrder + iOrder];

	T *pMatrix = pMem;
	for(unsigned int i=0; i<iOrder; ++i)
			for(unsigned int j=0; j<iOrder; ++j)
					pMatrix[i*iOrder + j] = mat(i,j);

	T *p_eigenvecs = pMatrix + iOrder*iOrder;
	T *p_eigenvecs_l = pMem + iOrder*iOrder;
	T *peigenvals_real = p_eigenvecs_l + iOrder;
	T *peigenvals_imag = peigenvals_real + iOrder;

	int iInfo = LAPACKE_dgeev(LAPACK_ROW_MAJOR, 'N', 'V', iOrder, (double*)pMatrix, iOrder,
								(double*)peigenvals_real, (double*)peigenvals_imag,
								(double*)p_eigenvecs_l, iOrder, (double*)p_eigenvecs, iOrder);

	if(iInfo!=0)
	{
		//if(Settings::Get<int>("general/debug_level") >= 1)
			std::cerr << "Error: Could not solve eigenproblem (lapack error " << iInfo << ")."
						<< std::endl;
		bOk = false;
	}

	for(unsigned int i=0; i<iOrder; ++i)
	{
		for(unsigned int j=0; j<iOrder; ++j)
				evecs[i][j] = p_eigenvecs[j*iOrder + i];
		evecs[i] /= ublas::norm_2(evecs[i]);
		evals[i] = peigenvals_real[i];
	}

	delete[] pMem;
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
	T *pMem = new T[iOrder*iOrder + iOrder];
	T *pMatrix = pMem;

	for(unsigned int i=0; i<iOrder; ++i)
			for(unsigned int j=0; j<iOrder; ++j)
					pMatrix[i*iOrder + j] = mat(i,j);

	T *peigenvals_real = pMatrix + iOrder*iOrder;
	int iInfo = LAPACKE_dsyev(LAPACK_ROW_MAJOR, 'V', 'U',
								iOrder, (double*)pMatrix, iOrder,
								(double*)peigenvals_real);

	if(iInfo!=0)
	{
		//if(Settings::Get<int>("general/debug_level") >= 1)
			std::cerr << "Error: Could not solve eigenproblem (lapack error " << iInfo << ")."
						<< std::endl;
		bOk = false;
	}

	for(unsigned int i=0; i<iOrder; ++i)
	{
		for(unsigned int j=0; j<iOrder; ++j)
				evecs[i][j] = pMatrix[j*iOrder + i];
		evecs[i] /= ublas::norm_2(evecs[i]);
		evals[i] = peigenvals_real[i];
	}

	if(determinant<ublas::matrix<double>, double>(column_matrix(evecs)) < 0.)
		evecs[0] = -evecs[0];

	delete[] pMem;
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

	double *pMem = new double[n*m + iTauSize];
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
	delete[] pMem;
	return (iInfo==0);
}

#endif


template<>
double vec_angle_unsigned(const math::quaternion<double>& q1,
				const math::quaternion<double>& q2)
{
	double dot = q1.R_component_1()*q2.R_component_1() +
				q1.R_component_2()*q2.R_component_2() +
				q1.R_component_3()*q2.R_component_3() +
				q1.R_component_4()*q2.R_component_4();

	dot /= math::norm(q1);
	dot /= math::norm(q2);

	return std::acos(dot);
}
