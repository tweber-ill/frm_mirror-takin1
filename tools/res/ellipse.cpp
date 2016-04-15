/*
 * resolution ellipse calculation
 * @author tweber
 * @date 14-may-2013
 * @license GPLv2
 *
 * @desc This is a reimplementation in C++ of the file rc_projs.m of the
 *    			rescal5 package by Zinkin, McMorrow, Tennant, Farhi, and Wildes:
 *    			http://www.ill.eu/en/instruments-support/computing-for-science/cs-software/all-software/matlab-ill/rescal-for-matlab/
 */

#include "ellipse.h"
#include "tlibs/math/linalg2.h"
#include "tlibs/math/quat.h"
#include "tlibs/math/math.h"
#include "cn.h"

typedef t_real_elli t_real;
typedef ublas::matrix<t_real> t_mat;
typedef ublas::vector<t_real> t_vec;


// --------------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& ostr, const Ellipse& ell)
{
	ostr << "phi = " << tl::r2d(ell.phi) << " deg \n";
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

    vec[0] = x_hwhm*std::cos(2.*tl::get_pi<t_real>()*t)*std::cos(phi) - y_hwhm*std::sin(2.*tl::get_pi<t_real>()*t)*std::sin(phi) + x_offs;
    vec[1] = x_hwhm*std::cos(2.*tl::get_pi<t_real>()*t)*std::sin(phi) + y_hwhm*std::sin(2.*tl::get_pi<t_real>()*t)*std::cos(phi) + y_offs;

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
static const std::string g_strLabelsHKL[] = {"h (rlu)", "k (rlu)", "l (rlu)", "E (meV)"};
static const std::string g_strLabelsHKLOrient[] = {"Reflex 1 (rlu)", "Reflex 2 (rlu)", "Up (rlu)", "E (meV)"};

const std::string& ellipse_labels(int iCoord, EllipseCoordSys sys)
{
	switch(sys)
	{
		case EllipseCoordSys::RLU:
			return g_strLabelsHKL[iCoord];
		case EllipseCoordSys::RLU_ORIENT:
			return g_strLabelsHKLOrient[iCoord];
		case EllipseCoordSys::AUTO:
		case EllipseCoordSys::Q_AVG:
		default:
			return g_strLabels[iCoord];
	}
}


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
		std::cout << "evals: " << dEval << ", ";
	std::cout << "\nevecs: " << matRot << std::endl;
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
	t_real dMyPhi = tl::r2d(ell.phi);
	t_real dPhiShirane = tl::r2d(t_real(0.5)*atan(2.*res_mat0(0,1) / (res_mat0(0,0)-res_mat0(1,1))));
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

	//tl::log_info("Principal axes: ", quad.GetQ());
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
