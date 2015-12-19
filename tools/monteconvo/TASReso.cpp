/*
 * loads reso settings
 * @author tweber
 * @date jul-2015
 * @license GPLv2
 */

#include "TASReso.h"
#include "tlibs/math/lattice.h"
#include "tlibs/file/prop.h"
#include "tlibs/log/log.h"

#include <boost/units/io.hpp>


typedef double t_real;
using t_vec = ublas::vector<t_real>;
using t_mat = ublas::matrix<t_real>;


TASReso::TASReso()
{
	m_opts.bCenter = 0;
	m_opts.coords = McNeutronCoords::RLU;
}

TASReso::TASReso(const TASReso& res)
{
	operator=(res);
}

const TASReso& TASReso::operator=(const TASReso& res)
{
	this->m_algo = res.m_algo;
	this->m_foc = res.m_foc;
	this->m_opts = res.m_opts;
	this->m_reso = res.m_reso;
	this->m_res = res.m_res;
	this->m_bKiFix = res.m_bKiFix;
	this->m_dKFix = res.m_dKFix;

	return *this;
}


bool TASReso::LoadLattice(const char* pcXmlFile)
{
	const std::string strXmlRoot("taz/");

	tl::Prop<std::string> xml;
	if(!xml.Load(pcXmlFile, tl::PropType::XML))
	{
		tl::log_err("Cannot load crystal file \"", pcXmlFile, "\".");
		return false;
	}

	t_real a = xml.Query<t_real>((strXmlRoot + "sample/a").c_str(), 0.);
	t_real b = xml.Query<t_real>((strXmlRoot + "sample/b").c_str(), 0.);
	t_real c = xml.Query<t_real>((strXmlRoot + "sample/c").c_str(), 0.);
	t_real alpha = xml.Query<t_real>((strXmlRoot + "sample/alpha").c_str(), 90.) / 180. * M_PI;
	t_real beta = xml.Query<t_real>((strXmlRoot + "sample/beta").c_str(), 90.) / 180. * M_PI;
	t_real gamma = xml.Query<t_real>((strXmlRoot + "sample/gamma").c_str(), 90.) / 180. * M_PI;

	t_real dPlaneX0 = xml.Query<t_real>((strXmlRoot + "plane/x0").c_str(), 1.);
	t_real dPlaneX1 = xml.Query<t_real>((strXmlRoot + "plane/x1").c_str(), 0.);
	t_real dPlaneX2 = xml.Query<t_real>((strXmlRoot + "plane/x2").c_str(), 0.);
	t_real dPlaneY0 = xml.Query<t_real>((strXmlRoot + "plane/y0").c_str(), 0.);
	t_real dPlaneY1 = xml.Query<t_real>((strXmlRoot + "plane/y1").c_str(), 1.);
	t_real dPlaneY2 = xml.Query<t_real>((strXmlRoot + "plane/y2").c_str(), 0.);

	t_vec vec1 = tl::make_vec({dPlaneX0, dPlaneX1, dPlaneX2});
	t_vec vec2 = tl::make_vec({dPlaneY0, dPlaneY1, dPlaneY2});

	if(!SetLattice(a, b, c, alpha, beta, gamma, vec1, vec2))
		return false;

	return true;
}

bool TASReso::LoadRes(const char* pcXmlFile)
{
	const std::string strXmlRoot("taz/");

	tl::Prop<std::string> xml;
	if(!xml.Load(pcXmlFile, tl::PropType::XML))
	{
		tl::log_err("Cannot load resolution file \"", pcXmlFile, "\".");
		return false;
	}

	// CN
	m_reso.mono_d = xml.Query<t_real>((strXmlRoot + "reso/mono_d").c_str(), 0.) * tl::angstrom;
	m_reso.mono_mosaic = xml.Query<t_real>((strXmlRoot + "reso/mono_mosaic").c_str(), 0.) / (180.*60.) * M_PI * tl::radians;
	m_reso.ana_d = xml.Query<t_real>((strXmlRoot + "reso/ana_d").c_str(), 0.) * tl::angstrom;
	m_reso.ana_mosaic = xml.Query<t_real>((strXmlRoot + "reso/ana_mosaic").c_str(), 0.) / (180.*60.) * M_PI * tl::radians;
	m_reso.sample_mosaic = xml.Query<t_real>((strXmlRoot + "reso/sample_mosaic").c_str(), 0.) / (180.*60.) * M_PI * tl::radians;

	m_reso.coll_h_pre_mono = xml.Query<t_real>((strXmlRoot + "reso/h_coll_mono").c_str(), 0.) / (180.*60.) * M_PI * tl::radians;
	m_reso.coll_h_pre_sample = xml.Query<t_real>((strXmlRoot + "reso/h_coll_before_sample").c_str(), 0.) / (180.*60.) * M_PI * tl::radians;
	m_reso.coll_h_post_sample = xml.Query<t_real>((strXmlRoot + "reso/h_coll_after_sample").c_str(), 0.) / (180.*60.) * M_PI * tl::radians;
	m_reso.coll_h_post_ana = xml.Query<t_real>((strXmlRoot + "reso/h_coll_ana").c_str(), 0.) / (180.*60.) * M_PI * tl::radians;

	m_reso.coll_v_pre_mono = xml.Query<t_real>((strXmlRoot + "reso/v_coll_mono").c_str(), 0.) / (180.*60.) * M_PI * tl::radians;
	m_reso.coll_v_pre_sample = xml.Query<t_real>((strXmlRoot + "reso/v_coll_before_sample").c_str(), 0.) / (180.*60.) * M_PI * tl::radians;
	m_reso.coll_v_post_sample = xml.Query<t_real>((strXmlRoot + "reso/v_coll_after_sample").c_str(), 0.) / (180.*60.) * M_PI * tl::radians;
	m_reso.coll_v_post_ana = xml.Query<t_real>((strXmlRoot + "reso/v_coll_ana").c_str(), 0.) / (180.*60.) * M_PI * tl::radians;

	m_reso.dmono_refl = xml.Query<t_real>((strXmlRoot + "reso/mono_refl").c_str(), 0.);
	m_reso.dana_effic = xml.Query<t_real>((strXmlRoot + "reso/ana_effic").c_str(), 0.);

	m_reso.dmono_sense = (xml.Query<int>((strXmlRoot+"reso/mono_scatter_sense").c_str(), 0) ? +1. : -1.);
	m_reso.dana_sense = (xml.Query<int>((strXmlRoot+"reso/ana_scatter_sense").c_str(), 0) ? +1. : -1.);
	m_reso.dsample_sense = (xml.Query<int>((strXmlRoot+"reso/sample_scatter_sense").c_str(), 1) ? +1. : -1.);


	// Pop
	m_reso.mono_w = xml.Query<t_real>((strXmlRoot + "reso/pop_mono_w").c_str(), 0.)*0.01*tl::meters;
	m_reso.mono_h = xml.Query<t_real>((strXmlRoot + "reso/pop_mono_h").c_str(), 0.)*0.01*tl::meters;
	m_reso.mono_thick = xml.Query<t_real>((strXmlRoot + "reso/pop_mono_thick").c_str(), 0.)*0.01*tl::meters;
	m_reso.mono_curvh = xml.Query<t_real>((strXmlRoot + "reso/pop_mono_curvh").c_str(), 0.)*0.01*tl::meters;
	m_reso.mono_curvv = xml.Query<t_real>((strXmlRoot + "reso/pop_mono_curvv").c_str(), 0.)*0.01*tl::meters;
	m_reso.bMonoIsCurvedH = (xml.Query<int>((strXmlRoot + "reso/pop_mono_use_curvh").c_str(), 0) != 0);
	m_reso.bMonoIsCurvedV = (xml.Query<int>((strXmlRoot + "reso/pop_mono_use_curvv").c_str(), 0) != 0);
	m_reso.bMonoIsOptimallyCurvedH = (xml.Query<int>((strXmlRoot + "reso/pop_mono_use_curvh_opt").c_str(), 1) != 0);
	m_reso.bMonoIsOptimallyCurvedV = (xml.Query<int>((strXmlRoot + "reso/pop_mono_use_curvv_opt").c_str(), 1) != 0);

	m_reso.ana_w = xml.Query<t_real>((strXmlRoot + "reso/pop_ana_w").c_str(), 0.)*0.01*tl::meters;
	m_reso.ana_h = xml.Query<t_real>((strXmlRoot + "reso/pop_ana_h").c_str(), 0.)*0.01*tl::meters;
	m_reso.ana_thick = xml.Query<t_real>((strXmlRoot + "reso/pop_ana_thick").c_str(), 0.)*0.01*tl::meters;
	m_reso.ana_curvh = xml.Query<t_real>((strXmlRoot + "reso/pop_ana_curvh").c_str(), 0.)*0.01*tl::meters;
	m_reso.ana_curvv = xml.Query<t_real>((strXmlRoot + "reso/pop_ana_curvv").c_str(), 0.)*0.01*tl::meters;
	m_reso.bAnaIsCurvedH = (xml.Query<int>((strXmlRoot + "reso/pop_ana_use_curvh").c_str(), 0) != 0);
	m_reso.bAnaIsCurvedV = (xml.Query<int>((strXmlRoot + "reso/pop_ana_use_curvv").c_str(), 0) != 0);
	m_reso.bAnaIsOptimallyCurvedH = (xml.Query<int>((strXmlRoot + "reso/pop_ana_use_curvh_opt").c_str(), 1) != 0);
	m_reso.bAnaIsOptimallyCurvedV = (xml.Query<int>((strXmlRoot + "reso/pop_ana_use_curvv_opt").c_str(), 1) != 0);

	m_reso.bSampleCub = (xml.Query<int>((strXmlRoot + "reso/pop_sample_cuboid").c_str(), 0) != 0);
	m_reso.sample_w_q = xml.Query<t_real>((strXmlRoot + "reso/pop_sample_wq").c_str(), 0.)*0.01*tl::meters;
	m_reso.sample_w_perpq = xml.Query<t_real>((strXmlRoot + "reso/pop_sampe_wperpq").c_str(), 0.)*0.01*tl::meters;
	m_reso.sample_h = xml.Query<t_real>((strXmlRoot + "reso/pop_sample_h").c_str(), 0.)*0.01*tl::meters;

	m_reso.bSrcRect = (xml.Query<int>((strXmlRoot + "reso/pop_source_rect").c_str(), 0) != 0);
	m_reso.src_w = xml.Query<t_real>((strXmlRoot + "reso/pop_src_w").c_str(), 0.)*0.01*tl::meters;
	m_reso.src_h = xml.Query<t_real>((strXmlRoot + "reso/pop_src_h").c_str(), 0.)*0.01*tl::meters;

	m_reso.bDetRect = (xml.Query<int>((strXmlRoot + "reso/pop_det_rect").c_str(), 0) != 0);
	m_reso.det_w = xml.Query<t_real>((strXmlRoot + "reso/pop_det_w").c_str(), 0.)*0.01*tl::meters;
	m_reso.det_h = xml.Query<t_real>((strXmlRoot + "reso/pop_det_h").c_str(), 0.)*0.01*tl::meters;

	m_reso.bGuide = (xml.Query<int>((strXmlRoot + "reso/use_guide").c_str(), 0) != 0);
	m_reso.guide_div_h = xml.Query<t_real>((strXmlRoot + "reso/pop_guide_divh").c_str(), 0.) / (180.*60.) * M_PI * tl::radians;
	m_reso.guide_div_v = xml.Query<t_real>((strXmlRoot + "reso/pop_guide_divv").c_str(), 0.) / (180.*60.) * M_PI * tl::radians;

	m_reso.dist_mono_sample = xml.Query<t_real>((strXmlRoot + "reso/pop_dist_mono_sample").c_str(), 0.)*0.01*tl::meters;
	m_reso.dist_sample_ana = xml.Query<t_real>((strXmlRoot + "reso/pop_dist_sample_ana").c_str(), 0.)*0.01*tl::meters;
	m_reso.dist_ana_det = xml.Query<t_real>((strXmlRoot + "reso/pop_dist_ana_det").c_str(), 0.)*0.01*tl::meters;
	m_reso.dist_src_mono = xml.Query<t_real>((strXmlRoot + "reso/pop_dist_src_mono").c_str(), 0.)*0.01*tl::meters;


	// Eck
	m_reso.mono_mosaic_v = xml.Query<t_real>((strXmlRoot + "reso/eck_mono_mosaic_v").c_str(), 0.) / (180.*60.) * M_PI * tl::radians;
	m_reso.ana_mosaic_v = xml.Query<t_real>((strXmlRoot + "reso/eck_ana_mosaic_v").c_str(), 0.) / (180.*60.) * M_PI * tl::radians;
	m_reso.pos_x = xml.Query<t_real>((strXmlRoot + "reso/eck_sample_pos_x").c_str(), 0.)*0.01*tl::meters;
	m_reso.pos_y = xml.Query<t_real>((strXmlRoot + "reso/eck_sample_pos_y").c_str(), 0.)*0.01*tl::meters;
	m_reso.pos_z = xml.Query<t_real>((strXmlRoot + "reso/eck_sample_pos_z").c_str(), 0.)*0.01*tl::meters;


	m_algo = ResoAlgo(xml.Query<int>((strXmlRoot + "reso/algo").c_str(), 0));

	// TODO
	m_reso.mono_numtiles_h = 1;
	m_reso.mono_numtiles_v = 1;
	m_reso.ana_numtiles_h = 1;
	m_reso.ana_numtiles_v = 1;


	// preliminary position
	m_reso.ki = xml.Query<t_real>((strXmlRoot + "reso/ki").c_str(), 0.) / tl::angstrom;
	m_reso.kf = xml.Query<t_real>((strXmlRoot + "reso/kf").c_str(), 0.) / tl::angstrom;
	m_reso.E = xml.Query<t_real>((strXmlRoot + "reso/E").c_str(), 0.) * tl::one_meV;
	m_reso.Q = xml.Query<t_real>((strXmlRoot + "reso/Q").c_str(), 0.) / tl::angstrom;

	m_dKFix = m_bKiFix ? m_reso.ki*tl::angstrom : m_reso.kf*tl::angstrom;
	return true;
}


bool TASReso::SetLattice(t_real a, t_real b, t_real c,
		t_real alpha, t_real beta, t_real gamma,
		const t_vec& vec1, const t_vec& vec2)
{
	tl::Lattice<t_real> latt(a, b, c, alpha, beta, gamma);

	t_mat matB = tl::get_B(latt, 1);
	t_mat matU = tl::get_U(vec1, vec2, &matB);
	t_mat matUB = ublas::prod(matU, matB);

	t_mat matBinv, matUinv;
	bool bHasB = tl::inverse(matB, matBinv);
	bool bHasU = tl::inverse(matU, matUinv);
	t_mat matUBinv = ublas::prod(matBinv, matUinv);

	bool bHasUB = bHasB && bHasU;

	if(!bHasUB)
	{
		tl::log_err("Cannot invert UB matrix");
		return false;
	}


	m_opts.matU = matU;
	m_opts.matB = matB;
	m_opts.matUB = matUB;
	m_opts.matUinv = matUinv;
	m_opts.matBinv = matBinv;
	m_opts.matUBinv = matUBinv;

	ublas::matrix<double>* pMats[] = {&m_opts.matU, &m_opts.matB, &m_opts.matUB, 
		&m_opts.matUinv, &m_opts.matBinv, &m_opts.matUBinv};

	for(ublas::matrix<double> *pMat : pMats)
	{
		pMat->resize(4,4,1);

		for(int i0=0; i0<3; ++i0)
			(*pMat)(i0,3) = (*pMat)(3,i0) = 0.;
		(*pMat)(3,3) = 1.;
	}

	return true;
}

bool TASReso::SetHKLE(t_real h, t_real k, t_real l, t_real E)
{
	//std::cout << "UB = " << m_opts.matUB << std::endl;
	//std::cout << h << " " << k << " " << l << ", " << E << std::endl;

	t_vec vecHKLE;
	if(m_opts.matUB.size1() == 3)
		vecHKLE = tl::make_vec({h, k, l});
	else
		vecHKLE = tl::make_vec({h, k, l, E});

	t_vec vecQ = ublas::prod(m_opts.matUB, vecHKLE);
	if(vecQ.size() > 3)
		vecQ.resize(3, true);

	m_reso.Q = ublas::norm_2(vecQ) / tl::angstrom;
	m_reso.E = E * tl::meV;

	//tl::log_info("kfix = ", m_dKFix);
	tl::wavenumber kother = tl::get_other_k(m_reso.E, m_dKFix/tl::angstrom, m_bKiFix);
	if(m_bKiFix)
	{
		m_reso.ki = m_dKFix / tl::angstrom;
		m_reso.kf = kother;
	}
	else
	{
		m_reso.ki = kother;
		m_reso.kf = m_dKFix / tl::angstrom;
	}
	
	//tl::log_info("ki = ", m_reso.ki, ", kf = ", m_reso.kf);
	//tl::log_info("Q = ", m_reso.Q, ", E = ", m_reso.E/tl::meV, " meV");

	m_reso.thetam = units::abs(tl::get_mono_twotheta(m_reso.ki, m_reso.mono_d, /*m_reso.dmono_sense>=0.*/1)*0.5);
	m_reso.thetaa = units::abs(tl::get_mono_twotheta(m_reso.kf, m_reso.ana_d, /*m_reso.dana_sense>=0.*/1)*0.5);
	m_reso.twotheta = units::abs(tl::get_sample_twotheta(m_reso.ki, m_reso.kf, m_reso.Q, 1));

	//tl::log_info("thetam = ", tl::r2d(m_reso.thetam/tl::radians));
	//tl::log_info("thetaa = ", tl::r2d(m_reso.thetaa/tl::radians));
	//tl::log_info("twothetas = ", tl::r2d(m_reso.twotheta/tl::radians));

	m_reso.angle_ki_Q = tl::get_angle_ki_Q(m_reso.ki, m_reso.kf, m_reso.Q, /*m_reso.dsample_sense>=0.*/1);
	m_reso.angle_kf_Q = tl::get_angle_kf_Q(m_reso.ki, m_reso.kf, m_reso.Q, /*m_reso.dsample_sense>=0.*/1);
	
	//tl::log_info("kiQ = ", tl::r2d(m_reso.angle_ki_Q/tl::radians));
	//m_reso.angle_ki_Q = units::abs(m_reso.angle_ki_Q);
	//m_reso.angle_kf_Q = units::abs(m_reso.angle_kf_Q);


	if(m_foc == ResoFocus::FOC_NONE)
	{
		m_reso.bMonoIsCurvedH = m_reso.bMonoIsCurvedV = 0;
		m_reso.bAnaIsCurvedH = m_reso.bAnaIsCurvedV = 0;
		
		//tl::log_info("No focus.");
	}
	else
	{
		m_reso.bMonoIsCurvedH = m_reso.bMonoIsOptimallyCurvedH = (unsigned(m_foc) & unsigned(ResoFocus::FOC_MONO_H));
		m_reso.bMonoIsCurvedV = m_reso.bMonoIsOptimallyCurvedV = (unsigned(m_foc) & unsigned(ResoFocus::FOC_MONO_V));
		m_reso.bAnaIsCurvedH = m_reso.bAnaIsOptimallyCurvedH = (unsigned(m_foc) & unsigned(ResoFocus::FOC_ANA_H));
		m_reso.bAnaIsCurvedV = m_reso.bAnaIsOptimallyCurvedV = (unsigned(m_foc) & unsigned(ResoFocus::FOC_ANA_V));
		
		//tl::log_info("Mono focus (h,v): ", m_reso.bMonoIsOptimallyCurvedH, ", ", m_reso.bMonoIsOptimallyCurvedV);
		//tl::log_info("Ana focus (h,v): ", m_reso.bAnaIsOptimallyCurvedH, ", ", m_reso.bAnaIsOptimallyCurvedV);

		// remove collimators
		/*if(m_reso.bMonoIsCurvedH)
		{
			m_reso.coll_h_pre_mono = 99999. * tl::radians;
			m_reso.coll_h_pre_sample = 99999. * tl::radians;
		}
		if(m_reso.bMonoIsCurvedV)
		{
			m_reso.coll_v_pre_mono = 99999. * tl::radians;
			m_reso.coll_v_pre_sample = 99999. * tl::radians;
		}
		if(m_reso.bAnaIsCurvedH)
		{
			m_reso.coll_h_post_sample = 99999. * tl::radians;
			m_reso.coll_h_post_ana = 99999. * tl::radians;
		}
		if(m_reso.bAnaIsCurvedV)
		{
			m_reso.coll_v_post_sample = 99999. * tl::radians;
			m_reso.coll_v_post_ana = 99999. * tl::radians;
		}*/
	}


	/*tl::log_info("thetam = ", m_reso.thetam);
	tl::log_info("thetaa = ", m_reso.thetaa);
	tl::log_info("2theta = ", m_reso.twotheta);*/

	if(std::fabs(vecQ[2]) > tl::get_plane_dist_tolerance<t_real>())
	{
		tl::log_err("Position Q = (", h, " ", k, " ", l, "), E = ", E, " meV not in scattering plane.");
		return false;
	}

	vecQ.resize(2, true);
	m_opts.dAngleQVec0 = -tl::vec_angle(vecQ);
	//tl::log_info("angle Q vec0 = ", m_opts.dAngleQVec0);


	// calculate resolution at (hkl) and E
	if(m_algo == ResoAlgo::CN)
	{
		//tl::log_info("Algorithm: Cooper-Nathans");
		m_res = calc_cn(m_reso);
	}
	else if(m_algo == ResoAlgo::POP)
	{
		//tl::log_info("Algorithm: Popovici");
		m_res = calc_pop(m_reso);
	}
	else if(m_algo == ResoAlgo::ECK)
	{
		//tl::log_info("Algorithm: Eckold-Sobolev");
		m_res = calc_eck(m_reso);
	}
	else
	{
		tl::log_err("Unknown algorithm selected.");
		return false;
	}

	//tl::log_info("Resolution matrix: ", m_res.reso);
	return m_res.bOk;
}

Ellipsoid4d TASReso::GenerateMC(std::size_t iNum, std::vector<t_vec>& vecNeutrons) const
{
	Ellipsoid4d ell4d = calc_res_ellipsoid4d(m_res.reso, m_res.Q_avg);;
	mc_neutrons(ell4d, iNum, m_opts, vecNeutrons);
	return ell4d;
}
