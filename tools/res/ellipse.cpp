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

/*
 * this is a 1:1 C++ reimplementation of 'proj_elip' from 'mcresplot'
 * iX, iY: dimensions to plot
 * iInt: dimension to integrate
 * iRem1, iRem2: dimensions to remove
 */
Ellipse calc_res_ellipse(const ublas::matrix<double>& reso, int iX, int iY, int iInt, int iRem1, int iRem2)
{
		static const std::string strLabels[] = {"Q_para (1/A)", "Q_ortho (1/A)", "Q_z (1/A)", "E (meV)"};

		Ellipse ell;
		ell.x_offs = ell.y_offs = 0.;

        ell.x_lab = strLabels[iX];
        ell.y_lab = strLabels[iY];


		ublas::matrix<double> res_mat = reso;
        //vector<double> Q_offs = reso.Q_avg;

		if(iRem1>-1)
		{
				res_mat = remove_elems(res_mat, iRem1);
				//Q_offs = remove_elem(Q_offs, iRem1);

				if(iInt>=iRem1) --iInt;
				if(iRem2>=iRem1) --iRem2;
				if(iX>=iRem1) --iX;
				if(iY>=iRem1) --iY;
		}

        if(iRem2>-1)
        {
                res_mat = remove_elems(res_mat, iRem2);
                //Q_offs = remove_elem(Q_offs, iRem2);

                if(iInt>=iRem2) --iInt;
                if(iX>=iRem2) --iX;
                if(iY>=iRem2) --iY;
        }

        ublas::matrix<double> m_int = res_mat;
        if(iInt>-1)
        {
                m_int = gauss_int(res_mat, iInt);
                //Q_offs = remove_elem(Q_offs, iInt);

                if(iX>=iInt) --iX;
                if(iY>=iInt) --iY;
        }

        ublas::matrix<double> m(2,2);
        m(0,0)=m_int(iX,iX); m(0,1)=m_int(iX,iY);
        m(1,0)=m_int(iY,iX); m(1,1)=m_int(iY,iY);

        std::vector<ublas::vector<double> > evecs;
        std::vector<double> evals;
        ::eigenvec_sym(m, evecs, evals);
        ell.phi = rotation_angle(column_matrix(evecs))[0];

        // formula A4.61 from Shirane
        //ell.phi = 0.5*atan(2.*m(0,1) / (m(0,0)-m(1,1)));

        ublas::matrix<double> rot = rotation_matrix_2d(ell.phi);
        ublas::matrix<double> res_rot;
        res_rot = prod(m, rot);
        res_rot = prod(trans(rot), res_rot);

        ell.x_hwhm = SIGMA2HWHM/sqrt(res_rot(0,0));
        ell.y_hwhm = SIGMA2HWHM/sqrt(res_rot(1,1));

        //ell.x_offs = Q_offs[iX];
        //ell.y_offs = Q_offs[iY];

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
