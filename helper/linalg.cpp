/*
 * linalg helpers
 *
 * @author: tweber
 * @date: 30-apr-2013
 */

#include "linalg.h"
#include "math.h"
#include "../main/settings.h"

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
    	if(Settings::Get<int>("general/debug_level") >= 1)
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
    	if(Settings::Get<int>("general/debug_level") >= 1)
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

	if(determinant(column_matrix(evecs)) < 0.)
		evecs[0] = -evecs[0];

    delete[] pMem;
    return bOk;
}
