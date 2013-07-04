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
#include "../../helper/linalg.h"
#include "../../helper/math.h"
#include "cn.h"


ublas::vector<double> Ellipse::operator()(double t) const
{
	ublas::vector<double> vec(2);

    vec[0] = x_hwhm*std::cos(2.*M_PI*t)*std::cos(phi) - y_hwhm*std::sin(2.*M_PI*t)*std::sin(phi) + x_offs;
    vec[1] = x_hwhm*std::cos(2.*M_PI*t)*std::sin(phi) + y_hwhm*std::sin(2.*M_PI*t)*std::cos(phi) + y_offs;

    return vec;
}

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
	static const std::string strLabels[] = {"Q_para (1/A)", "Q_ortho (1/A)", "Q_z (1/A)", "E (meV)"};

	Ellipse ell;
	ell.x_offs = ell.y_offs = 0.;

	ell.x_lab = strLabels[iX];
	ell.y_lab = strLabels[iY];


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

	ublas::matrix<double> m_int = res_mat;
	if(iInt>-1)
	{
		m_int = gauss_int(res_mat, iInt);
		Q_offs = remove_elem(Q_offs, iInt);

		if(iX>=iInt) --iX;
		if(iY>=iInt) --iY;
	}

	ublas::matrix<double> m(2,2);
	m(0,0)=m_int(iX,iX); m(0,1)=m_int(iX,iY);
	m(1,0)=m_int(iY,iX); m(1,1)=m_int(iY,iY);

	std::vector<ublas::vector<double> > evecs;
	std::vector<double> evals;
	::eigenvec_sym(m, evecs, evals);

	ublas::matrix<double> evecs_rot = column_matrix(evecs);
	ell.phi = rotation_angle(evecs_rot)[0];

	if(::fabs(ell.phi) > M_PI/4.)
	{
		ublas::matrix<double> rot_m90 = rotation_matrix_2d(-M_PI/2.);
		evecs_rot = ublas::prod(evecs_rot, rot_m90);
		ell.phi = rotation_angle(evecs_rot)[0];
	}
	if(ell.phi == -M_PI)
		ell.phi = 0.;

	//std::cout << ell.phi/M_PI*180. << std::endl;

	// formula A4.61 from Shirane
	//ell.phi = 0.5*atan(2.*m(0,1) / (m(0,0)-m(1,1)));
	//std::cout << ell.phi/M_PI*180. << std::endl;
	//ublas::matrix<double> rot = rotation_matrix_2d(ell.phi);

	ublas::matrix<double> res_rot;
	res_rot = prod(m, evecs_rot);
	res_rot = prod(trans(evecs_rot), res_rot);

	ell.x_hwhm = SIGMA2HWHM/sqrt(res_rot(0,0));
	ell.y_hwhm = SIGMA2HWHM/sqrt(res_rot(1,1));

	ell.x_offs = Q_offs[iX];
	ell.y_offs = Q_offs[iY];

	return ell;
}

Ellipsoid calc_res_ellipsoid(const ublas::matrix<double>& reso,
										const ublas::vector<double>& Q_avg,
										int iX, int iY, int iZ, int iInt, int iRem)
{
	static const std::string strLabels[] = {"Q_para (1/A)", "Q_ortho (1/A)", "Q_z (1/A)", "E (meV)"};

	Ellipsoid ell;
	ell.x_offs = ell.y_offs = ell.z_offs = 0.;

	ell.x_lab = strLabels[iX];
	ell.y_lab = strLabels[iY];
	ell.z_lab = strLabels[iZ];


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

std::ostream& operator<<(std::ostream& ostr, const Ellipse& ell)
{
	ostr << "phi = " << ell.phi/M_PI*180. << " deg \n";
	ostr << "x_hwhm = " << ell.x_hwhm << ", ";
	ostr << "y_hwhm = " << ell.y_hwhm << "\n";
	ostr << "x_offs = " << ell.x_offs << ", ";
	ostr << "y_offs = " << ell.y_offs << "\n";
	ostr << "x_lab = " << ell.x_lab << ", ";
	ostr << "y_lab = " << ell.y_lab;

	return ostr;
}
