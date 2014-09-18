/*
 * resolution ellipse calculation
 * @author tweber
 * @date 14-may-2013
 *
 * @desc This is a reimplementation in C++ of the file rc_projs.m of the
 *    			rescal5 package by Zinkin, McMorrow, Tennant, Farhi, and Wildes:
 *    			http://www.ill.eu/en/instruments-support/computing-for-science/cs-software/all-software/matlab-ill/rescal-for-matlab/
 */

#include "ellipse.h"
#include "helper/linalg.h"
#include "helper/linalg2.h"
#include "helper/math.h"
#include "helper/rand.h"

#include "cn.h"


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

ublas::vector<double> Ellipse::operator()(double t) const
{
	ublas::vector<double> vec(2);

    vec[0] = x_hwhm*std::cos(2.*M_PI*t)*std::cos(phi) - y_hwhm*std::sin(2.*M_PI*t)*std::sin(phi) + x_offs;
    vec[1] = x_hwhm*std::cos(2.*M_PI*t)*std::sin(phi) + y_hwhm*std::sin(2.*M_PI*t)*std::cos(phi) + y_offs;

    return vec;
}

void Ellipse::GetCurvePoints(std::vector<double>& x, std::vector<double>& y,
							unsigned int iPoints)
{
	x.resize(iPoints);
	y.resize(iPoints);

	for(unsigned int i=0; i<iPoints; ++i)
	{
		double dT = double(i)/double(iPoints-1);
		ublas::vector<double> vec = operator()(dT);

		x[i] = vec[0];
		y[i] = vec[1];
	}
}

// --------------------------------------------------------------------------------

static const std::string g_strLabels[] = {"Q_{para} (1/A)", "Q_{ortho} (1/A)", "Q_z (1/A)", "E (meV)"};

/*
 * this is a 1:1 C++ reimplementation of 'proj_elip' from 'mcresplot'
 * iX, iY: dimensions to plot
 * iInt: dimension to integrate
 * iRem1, iRem2: dimensions to remove
 */
Ellipse calc_res_ellipse(const ublas::matrix<double>& reso,
									const ublas::vector<double>& Q_avg,
									int iX, int iY, int iInt, int iRem1, int iRem2)
{
	Ellipse ell;
	ell.x_offs = ell.y_offs = 0.;

	// labels only valid for non-rotated system
	ell.x_lab = g_strLabels[iX];
	ell.y_lab = g_strLabels[iY];


	ublas::matrix<double> res_mat = reso;
	ublas::vector<double> Q_offs = Q_avg;

	if(iRem1>-1)
	{
		res_mat = remove_elems(res_mat, iRem1);
		Q_offs = remove_elem(Q_offs, iRem1);

		if(iInt>=iRem1) --iInt;
		if(iRem2>=iRem1) --iRem2;
		if(iX>=iRem1) --iX;
		if(iY>=iRem1) --iY;
	}

	if(iRem2>-1)
	{
		res_mat = remove_elems(res_mat, iRem2);
		Q_offs = remove_elem(Q_offs, iRem2);

		if(iInt>=iRem2) --iInt;
		if(iX>=iRem2) --iX;
		if(iY>=iRem2) --iY;
	}

	if(iInt>-1)
	{
		res_mat = gauss_int(res_mat, iInt);
		Q_offs = remove_elem(Q_offs, iInt);

		if(iX>=iInt) --iX;
		if(iY>=iInt) --iY;
	}

	std::vector<ublas::vector<double> > evecs;
	std::vector<double> evals;
	::eigenvec_sym(res_mat, evecs, evals);

	// sort by axis length: long axis is first axis
	::sort_eigenvecs<double>(evecs, evals, 1, [](double d) -> double { return 1./d; });

	/*std::cout << "Eigenvalues: ";
	for(double dEv : evals)
		std::cout << dEv << " ";
	std::cout << std::endl;*/

	ublas::matrix<double> evecs_rot = column_matrix(evecs);
	ell.phi = rotation_angle(evecs_rot)[0];

	// if rotation angle >= 90° -> choose other axis as first axis
	if(std::fabs(ell.phi) >= M_PI/2.)
	{
		std::swap(evecs[0], evecs[1]);
		std::swap(evals[0], evals[1]);

		if(ell.phi > 0.)
		{
			evecs[0] = -evecs[0];
			evals[0] = -evals[0];
		}

		evecs_rot = column_matrix(evecs);
		ell.phi = rotation_angle(evecs_rot)[0];
	}


	ublas::matrix<double> res_rot;
	res_rot = prod(res_mat, evecs_rot);
	res_rot = prod(trans(evecs_rot), res_rot);

	ell.x_hwhm = SIGMA2HWHM/sqrt(res_rot(0,0));
	ell.y_hwhm = SIGMA2HWHM/sqrt(res_rot(1,1));

	ell.x_offs = Q_offs[iX];
	ell.y_offs = Q_offs[iY];

	ell.area = get_ellipsoid_volume(res_mat);
	ell.slope = std::tan(ell.phi);


	// sanity check
	double dMyPhi = ell.phi/M_PI*180.;
	double dPhiShirane = 0.5*atan(2.*res_mat(0,1) / (res_mat(0,0)-res_mat(1,1))) / M_PI*180.;
	if(!::float_equal(dMyPhi, dPhiShirane, 0.01)
		&& !::float_equal(dMyPhi-90., dPhiShirane, 0.01))
	{
		log_warn("Calculated ellipse phi = ", dMyPhi, " deg",
				" deviates from theoretical phi = ", dPhiShirane,
				" deg.");
	}

	return ell;
}

// --------------------------------------------------------------------------------

Ellipsoid calc_res_ellipsoid(const ublas::matrix<double>& reso,
										const ublas::vector<double>& Q_avg,
										int iX, int iY, int iZ, int iInt, int iRem)
{
	Ellipsoid ell;
	ell.x_offs = ell.y_offs = ell.z_offs = 0.;

	// labels only valid for non-rotated system
	ell.x_lab = g_strLabels[iX];
	ell.y_lab = g_strLabels[iY];
	ell.z_lab = g_strLabels[iZ];


	ublas::matrix<double> res_mat = reso;
	ublas::vector<double> Q_offs = Q_avg;

	if(iRem>-1)
	{
		res_mat = remove_elems(res_mat, iRem);
		Q_offs = remove_elem(Q_offs, iRem);

		if(iInt>=iRem) --iInt;
		if(iX>=iRem) --iX;
		if(iY>=iRem) --iY;
		if(iZ>=iRem) --iZ;

		//std::cout << "rem: " << res_mat << std::endl;
	}

	if(iInt>-1)
	{
		res_mat = gauss_int(res_mat, iInt);
		Q_offs = remove_elem(Q_offs, iInt);

		if(iX>=iInt) --iX;
		if(iY>=iInt) --iY;
		if(iZ>=iInt) --iZ;

		//std::cout << "int: " << res_mat << std::endl;
	}

	std::vector<ublas::vector<double> > evecs;
	std::vector<double> evals;
	::eigenvec_sym(res_mat, evecs, evals);
	::sort_eigenvecs<double>(evecs, evals, 1, [](double d) -> double { return 1./d; });

	ell.rot = column_matrix(evecs);
	//std::vector<double> vecRot = rotation_angle(ell.rot);
	//ell.alpha = vecRot[0];
	//ell.beta = vecRot[1];
	//ell.gamma = vecRot[2];

	ublas::matrix<double> res_rot;
	res_rot = prod(res_mat, ell.rot);
	res_rot = prod(trans(ell.rot), res_rot);

	ell.x_hwhm = SIGMA2HWHM/sqrt(res_rot(0,0));
	ell.y_hwhm = SIGMA2HWHM/sqrt(res_rot(1,1));
	ell.z_hwhm = SIGMA2HWHM/sqrt(res_rot(2,2));

	ell.x_offs = Q_offs[iX];
	ell.y_offs = Q_offs[iY];
	ell.z_offs = Q_offs[iZ];

	ell.vol = get_ellipsoid_volume(res_mat);

	//std::cout << ell.rot << std::endl;
	//std::cout << quat_to_rot3(rot3_to_quat(ell.rot)) << std::endl;
	//std::cout << "alpha=" << ell.alpha/M_PI*180. << ", beta=" << ell.beta/M_PI*180. << ", gamma="<<ell.gamma/M_PI*180. << std::endl;

	/*
	std::cout << "rot0: " << ell.rot << std::endl;
	ublas::matrix<double> rot_x = rotation_matrix_3d_z(ell.gamma);
	ublas::matrix<double> rot_y = rotation_matrix_3d_y(ell.beta);
	ublas::matrix<double> rot_z = rotation_matrix_3d_x(ell.alpha);
	ublas::matrix<double> rot_0 = ublas::prod(rot_x, rot_y);
	ublas::matrix<double> rot_1 = ublas::prod(rot_0, rot_z);
	std::cout << "rot1: " << rot_1 << std::endl;
	 */

	return ell;
}

// --------------------------------------------------------------------------------

Ellipsoid4d calc_res_ellipsoid4d(const ublas::matrix<double>& reso, const ublas::vector<double>& Q_avg)
{
	Ellipsoid4d ell;

	std::vector<ublas::vector<double> > evecs;
	std::vector<double> evals;
	::eigenvec_sym(reso, evecs, evals);
	::sort_eigenvecs<double>(evecs, evals, 1, [](double d) -> double { return 1./d; });

	ell.rot = column_matrix(evecs);

	ublas::matrix<double> res_rot;
	res_rot = prod(reso, ell.rot);
	res_rot = prod(trans(ell.rot), res_rot);

	ell.x_hwhm = SIGMA2HWHM/sqrt(res_rot(0,0));
	ell.y_hwhm = SIGMA2HWHM/sqrt(res_rot(1,1));
	ell.z_hwhm = SIGMA2HWHM/sqrt(res_rot(2,2));
	ell.w_hwhm = SIGMA2HWHM/sqrt(res_rot(3,3));

	ell.x_offs = Q_avg[0];
	ell.y_offs = Q_avg[1];
	ell.z_offs = Q_avg[2];
	ell.w_offs = Q_avg[3];

	// labels only valid for non-rotated system
	ell.x_lab = g_strLabels[0];
	ell.y_lab = g_strLabels[1];
	ell.z_lab = g_strLabels[2];
	ell.w_lab = g_strLabels[3];

	ell.vol = get_ellipsoid_volume(reso);

	//std::cout << ell << std::endl;
	return ell;
}


void mc_neutrons(const Ellipsoid4d& ell4d, unsigned int iNum, bool bCenter,
				std::vector<ublas::vector<double>>& vecResult)
{
	init_rand();

	ublas::vector<double> vecTrans(4);
	vecTrans[0] = ell4d.x_offs;
	vecTrans[1] = ell4d.y_offs;
	vecTrans[2] = ell4d.z_offs;
	vecTrans[3] = ell4d.w_offs;

	const ublas::matrix<double>& rot = ell4d.rot;
	const ublas::matrix<double> rot_inv = ublas::trans(rot);

	/* orthonormal matrix => inv == trans
	ublas::matrix<double> inv;
	inverse(rot, inv);

	std::cout << "trans: " << rot_inv << std::endl;
	std::cout << "inverse: " << inv << std::endl;
	*/

	vecResult.reserve(iNum);

	for(unsigned int iCur=0; iCur<iNum; ++iCur)
	{
		ublas::vector<double> vecMC = rand_norm_nd<ublas::vector<double>>({0.,0.,0.,0.},
														{ell4d.x_hwhm*HWHM2SIGMA,
														ell4d.y_hwhm*HWHM2SIGMA,
														ell4d.z_hwhm*HWHM2SIGMA,
														ell4d.w_hwhm*HWHM2SIGMA});

		vecMC = ublas::prod(rot_inv, vecMC);
		if(!bCenter)
			vecMC += vecTrans;

		vecResult.push_back(std::move(vecMC));
	}
}
