/*
 * resolution ellipse calculation
 * @author tweber
 * @date 14-may-2013
 * @copyright GPLv2
 *
 * @desc This is a reimplementation in C++ of the file rc_projs.m of the
 *    			rescal5 package by Zinkin, McMorrow, Tennant, Farhi, and Wildes:
 *    			http://www.ill.eu/en/instruments-support/computing-for-science/cs-software/all-software/matlab-ill/rescal-for-matlab/
 */

#include "ellipse.h"
#include "tlibs/math/linalg2.h"
#include "tlibs/math/quat.h"
#include "tlibs/math/math.h"
#include "tlibs/math/rand.h"

#include "cn.h"

typedef double t_real;
typedef ublas::matrix<t_real> t_mat;
typedef ublas::vector<t_real> t_vec;


// --------------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& ostr, const Ellipse& ell)
{
	ostr << "phi = " << ell.phi/M_PI*180. << " deg \n";
	ostr << "slope = " << ell.slope << " deg \n";
	ostr << "x_hwhm = " << ell.x_hwhm << ", ";
	ostr << "y_hwhm = " << ell.y_hwhm << "\n";
	ostr << "x_offs = " << ell.x_offs << ", ";
	ostr << "y_offs = " << ell.y_offs << "\n";
	ostr << "x_lab = " << ell.x_lab << ", ";
	ostr << "y_lab = " << ell.y_lab << "\n";
	ostr << "area = " << ell.area;

	return ostr;
}


std::ostream& operator<<(std::ostream& ostr, const Ellipsoid4d& ell)
{
	ostr << "x_hwhm = " << ell.x_hwhm << ", ";
	ostr << "y_hwhm = " << ell.y_hwhm << ", ";
	ostr << "z_hwhm = " << ell.z_hwhm << ", ";
	ostr << "w_hwhm = " << ell.w_hwhm << "\n";
	ostr << "x_offs = " << ell.x_offs << ", ";
	ostr << "y_offs = " << ell.y_offs << ", ";
	ostr << "z_offs = " << ell.z_offs << ", ";
	ostr << "w_offs = " << ell.w_offs << "\n";
	ostr << "x_lab = " << ell.x_lab << ", ";
	ostr << "y_lab = " << ell.y_lab << ", ";
	ostr << "z_lab = " << ell.z_lab << ", ";
	ostr << "w_lab = " << ell.w_lab << "\n";
	ostr << "volume = " << ell.vol;

	return ostr;
}

// --------------------------------------------------------------------------------

t_vec Ellipse::operator()(t_real t) const
{
	t_vec vec(2);

    vec[0] = x_hwhm*std::cos(2.*M_PI*t)*std::cos(phi) - y_hwhm*std::sin(2.*M_PI*t)*std::sin(phi) + x_offs;
    vec[1] = x_hwhm*std::cos(2.*M_PI*t)*std::sin(phi) + y_hwhm*std::sin(2.*M_PI*t)*std::cos(phi) + y_offs;

    return vec;
}

void Ellipse::GetCurvePoints(std::vector<t_real>& x, std::vector<t_real>& y,
							unsigned int iPoints, t_real *pLRTB)
{
	x.resize(iPoints);
	y.resize(iPoints);

	for(unsigned int i=0; i<iPoints; ++i)
	{
		t_real dT = t_real(i)/t_real(iPoints-1);
		t_vec vec = operator()(dT);

		x[i] = vec[0];
		y[i] = vec[1];
	}

	if(pLRTB)	// bounding rect
	{
		auto pairX = std::minmax_element(x.begin(), x.end());
		auto pairY = std::minmax_element(y.begin(), y.end());

		*(pLRTB+0) = *pairX.first;	// left
		*(pLRTB+1) = *pairX.second;	// right
		*(pLRTB+2) = *pairY.second;	// top
		*(pLRTB+3) = *pairY.first;	// bottom
	}
}

// --------------------------------------------------------------------------------

template<class T = t_real>
static void elli_gauss_int(tl::QuadEllipsoid<T>& quad, unsigned int iIdx)
{
	ublas::matrix<T> m_Qint = ellipsoid_gauss_int(quad.GetQ(), iIdx);
	quad.RemoveElems(iIdx);
	quad.SetQ(m_Qint);
}


static const std::string g_strLabels[] = {"Q_{para} (1/A)", "Q_{ortho} (1/A)", "Q_z (1/A)", "E (meV)"};

/*
 * this is a 1:1 C++ reimplementation of 'proj_elip' from 'mcresplot'
 * iX, iY: dimensions to plot
 * iInt: dimension to integrate
 * iRem1, iRem2: dimensions to remove
 */
Ellipse calc_res_ellipse(const t_mat& reso,
									const t_vec& Q_avg,
									int iX, int iY, int iInt, int iRem1, int iRem2)
{
	Ellipse ell;
	ell.quad.SetDim(4);
	ell.quad.SetQ(reso);

	ell.x_offs = ell.y_offs = 0.;

	// labels only valid for non-rotated system
	ell.x_lab = g_strLabels[iX];
	ell.y_lab = g_strLabels[iY];


	t_vec Q_offs = Q_avg;

	if(iRem1>-1)
	{
		ell.quad.RemoveElems(iRem1);
		Q_offs = tl::remove_elem(Q_offs, iRem1);

		if(iInt>=iRem1) --iInt;
		if(iRem2>=iRem1) --iRem2;
		if(iX>=iRem1) --iX;
		if(iY>=iRem1) --iY;
	}

	if(iRem2>-1)
	{
		ell.quad.RemoveElems(iRem2);
		Q_offs = tl::remove_elem(Q_offs, iRem2);

		if(iInt>=iRem2) --iInt;
		if(iX>=iRem2) --iX;
		if(iY>=iRem2) --iY;
	}

	if(iInt>-1)
	{
		elli_gauss_int(ell.quad, iInt);
		Q_offs = tl::remove_elem(Q_offs, iInt);

		if(iX>=iInt) --iX;
		if(iY>=iInt) --iY;
	}

	std::vector<t_real> evals;
	t_mat matRot;

	tl::QuadEllipsoid<t_real> quad(2);
	ell.quad.GetPrincipalAxes(matRot, evals, &quad);

	/*std::cout << "matrix: " << ell.quad.GetQ() << std::endl;
	for(t_real dEval : evals)
		std::cout << "Evals: " << dEval << ", ";
	std::cout << std::endl;*/

	ell.phi = tl::rotation_angle(matRot)[0];

	// if rotation angle >= 90° -> choose other axis as first axis
	/*if(std::fabs(ell.phi) >= M_PI/2.)
	{
		std::swap(evecs[0], evecs[1]);
		std::swap(evals[0], evals[1]);
		evecs[0] = -evecs[0];

		if(ell.phi < 0.)
			evecs[0] = -evecs[0];

		evecs_rot = column_matrix(evecs);
		ell.phi = rotation_angle(evecs_rot)[0];
	}*/

	/*if(std::fabs(ell.phi) > std::fabs(M_PI-std::fabs(ell.phi)))
	{
		evecs[0] = -evecs[0];
		evecs_rot = column_matrix(evecs);
		ell.phi = rotation_angle(evecs_rot)[0];
	}*/


	ell.x_hwhm = tl::SIGMA2HWHM * quad.GetRadius(0);
	ell.y_hwhm = tl::SIGMA2HWHM * quad.GetRadius(1);

	ell.x_offs = Q_offs[iX];
	ell.y_offs = Q_offs[iY];

	ell.area = quad.GetVolume();
	ell.slope = std::tan(ell.phi);


#ifndef NDEBUG
	// sanity check, see Shirane p. 267
	t_mat res_mat0 = ell.quad.GetQ();
	t_real dMyPhi = ell.phi/M_PI*180.;
	t_real dPhiShirane = 0.5*atan(2.*res_mat0(0,1) / (res_mat0(0,0)-res_mat0(1,1))) / M_PI*180.;
	if(!tl::float_equal(dMyPhi, dPhiShirane, 0.01)
		&& !tl::float_equal(dMyPhi-90., dPhiShirane, 0.01))
	{
		tl::log_warn("Calculated ellipse phi = ", dMyPhi, " deg",
				" deviates from theoretical phi = ", dPhiShirane,
				" deg.");
	}
#endif

	return ell;
}

// --------------------------------------------------------------------------------

Ellipsoid calc_res_ellipsoid(const t_mat& reso,
							const t_vec& Q_avg,
							int iX, int iY, int iZ, int iInt, int iRem)
{
	Ellipsoid ell;

	ell.quad.SetDim(4);
	ell.quad.SetQ(reso);

	ell.x_offs = ell.y_offs = ell.z_offs = 0.;

	// labels only valid for non-rotated system
	ell.x_lab = g_strLabels[iX];
	ell.y_lab = g_strLabels[iY];
	ell.z_lab = g_strLabels[iZ];


	t_vec Q_offs = Q_avg;

	if(iRem>-1)
	{
		ell.quad.RemoveElems(iRem);
		Q_offs = tl::remove_elem(Q_offs, iRem);

		if(iInt>=iRem) --iInt;
		if(iX>=iRem) --iX;
		if(iY>=iRem) --iY;
		if(iZ>=iRem) --iZ;
	}

	if(iInt>-1)
	{
		elli_gauss_int(ell.quad, iInt);
		Q_offs = tl::remove_elem(Q_offs, iInt);

		if(iX>=iInt) --iX;
		if(iY>=iInt) --iY;
		if(iZ>=iInt) --iZ;
	}

	std::vector<t_real> evals;
	tl::QuadEllipsoid<t_real> quad(3);
	ell.quad.GetPrincipalAxes(ell.rot, evals, &quad);

	ell.x_hwhm = tl::SIGMA2HWHM * quad.GetRadius(0);
	ell.y_hwhm = tl::SIGMA2HWHM * quad.GetRadius(1);
	ell.z_hwhm = tl::SIGMA2HWHM * quad.GetRadius(2);

	ell.x_offs = Q_offs[iX];
	ell.y_offs = Q_offs[iY];
	ell.z_offs = Q_offs[iZ];

	ell.vol = quad.GetVolume();
	return ell;
}

// --------------------------------------------------------------------------------

Ellipsoid4d calc_res_ellipsoid4d(const t_mat& reso, const t_vec& Q_avg)
{
	Ellipsoid4d ell;
	ell.quad.SetDim(4);
	ell.quad.SetQ(reso);

	std::vector<t_real> evals;
	tl::QuadEllipsoid<t_real> quad(4);
	ell.quad.GetPrincipalAxes(ell.rot, evals, &quad);

	ell.x_hwhm = tl::SIGMA2HWHM * quad.GetRadius(0);
	ell.y_hwhm = tl::SIGMA2HWHM * quad.GetRadius(1);
	ell.z_hwhm = tl::SIGMA2HWHM * quad.GetRadius(2);
	ell.w_hwhm = tl::SIGMA2HWHM * quad.GetRadius(3);

	ell.x_offs = Q_avg[0];
	ell.y_offs = Q_avg[1];
	ell.z_offs = Q_avg[2];
	ell.w_offs = Q_avg[3];

	// labels only valid for non-rotated system
	ell.x_lab = g_strLabels[0];
	ell.y_lab = g_strLabels[1];
	ell.z_lab = g_strLabels[2];
	ell.w_lab = g_strLabels[3];

	ell.vol = quad.GetVolume();

	//std::cout << ell << std::endl;
	return ell;
}


void mc_neutrons(const Ellipsoid4d& ell4d, unsigned int iNum,
				const McNeutronOpts& opts,
				std::vector<t_vec>& vecResult)
{
	static bool bInited = 0;
	if(!bInited)
	{
		tl::init_rand();
		bInited = 1;
	}

	t_vec vecTrans(4);
	vecTrans[0] = ell4d.x_offs;
	vecTrans[1] = ell4d.y_offs;
	vecTrans[2] = ell4d.z_offs;
	vecTrans[3] = ell4d.w_offs;

	const t_mat& rot = ell4d.rot;
	if(vecResult.size() != iNum)
		vecResult.resize(iNum);

	//tl::log_debug("Qvec0 = ", opts.dAngleQVec0/M_PI*180.);
	t_mat matQVec0 = tl::rotation_matrix_2d(-opts.dAngleQVec0);
	matQVec0.resize(4,4, true);
	matQVec0(2,2) = matQVec0(3,3) = 1.;
	matQVec0(2,0) = matQVec0(2,1) = matQVec0(2,3) = 0.;
	matQVec0(3,0) = matQVec0(3,1) = matQVec0(3,2) = 0.;
	matQVec0(0,2) = matQVec0(0,3) = 0.;
	matQVec0(1,2) = matQVec0(1,3) = 0.;

	t_mat matUBinvQVec0 = ublas::prod(opts.matUBinv, matQVec0);

	for(unsigned int iCur=0; iCur<iNum; ++iCur)
	{
		t_vec vecMC = tl::rand_norm_nd<t_vec>({0.,0.,0.,0.},
			{ell4d.x_hwhm*tl::HWHM2SIGMA, ell4d.y_hwhm*tl::HWHM2SIGMA,
			ell4d.z_hwhm*tl::HWHM2SIGMA, ell4d.w_hwhm*tl::HWHM2SIGMA});

		vecMC = ublas::prod(rot, vecMC);
		if(!opts.bCenter)
			vecMC += vecTrans;

		if(opts.coords == McNeutronCoords::ANGS)
			vecMC = ublas::prod(matQVec0, vecMC);
		else if(opts.coords == McNeutronCoords::RLU)
			vecMC = ublas::prod(matUBinvQVec0, vecMC);

		vecResult[iCur] = std::move(vecMC);
	}
}
