/*
 * TAS tool (crystal stuff)
 * @author tweber
 * @date feb-2014
 * @copyright GPLv2
 */

#include "taz.h"
#include "tlibs/string/spec_char.h"
#include <boost/algorithm/string.hpp>


static QString dtoqstr(double dVal, unsigned int iPrec=8)
{
	std::ostringstream ostr;
	ostr.precision(iPrec);
	ostr << dVal;
	return QString(ostr.str().c_str());
}

std::ostream& operator<<(std::ostream& ostr, const tl::Lattice<double>& lat)
{
	ostr << "a = " << lat.GetA();
	ostr << ", b = " << lat.GetB();
	ostr << ", c = " << lat.GetC();
	ostr << ", alpha = " << lat.GetAlpha();
	ostr << ", beta = " << lat.GetBeta();
	ostr << ", gamma = " << lat.GetGamma();
	return ostr;
}

void TazDlg::SetCrystalType()
{
	m_crystalsys = CrystalSystem::CRYS_NOT_SET;

	SpaceGroup *pSpaceGroup = 0;
	int iSpaceGroupIdx = comboSpaceGroups->currentIndex();
	if(iSpaceGroupIdx != 0)
		pSpaceGroup = (SpaceGroup*)comboSpaceGroups->itemData(iSpaceGroupIdx).value<void*>();
	if(pSpaceGroup)
		m_crystalsys = pSpaceGroup->GetCrystalSystem();

	CheckCrystalType();
}

// TODO
void TazDlg::CheckCrystalType()
{
	switch(m_crystalsys)
	{
		case CRYS_CUBIC:
			editA->setEnabled(1);
			editB->setEnabled(0);
			editC->setEnabled(0);
			editAlpha->setEnabled(0);
			editBeta->setEnabled(0);
			editGamma->setEnabled(0);

			editARecip->setEnabled(1);
			editBRecip->setEnabled(0);
			editCRecip->setEnabled(0);
			editAlphaRecip->setEnabled(0);
			editBetaRecip->setEnabled(0);
			editGammaRecip->setEnabled(0);

			editB->setText(editA->text());
			editC->setText(editA->text());
			editBRecip->setText(editARecip->text());
			editCRecip->setText(editARecip->text());
			editAlpha->setText("90");
			editBeta->setText("90");
			editGamma->setText("90");
			editAlphaRecip->setText("90");
			editBetaRecip->setText("90");
			editGammaRecip->setText("90");
			break;

		case CRYS_HEXAGONAL:
			editA->setEnabled(1);
			editB->setEnabled(0);
			editC->setEnabled(1);
			editAlpha->setEnabled(0);
			editBeta->setEnabled(0);
			editGamma->setEnabled(0);

			editARecip->setEnabled(1);
			editBRecip->setEnabled(0);
			editCRecip->setEnabled(1);
			editAlphaRecip->setEnabled(0);
			editBetaRecip->setEnabled(0);
			editGammaRecip->setEnabled(0);

			editB->setText(editA->text());
			editBRecip->setText(editARecip->text());
			editAlpha->setText("90");
			editBeta->setText("90");
			editGamma->setText("120");
			editAlphaRecip->setText("90");
			editBetaRecip->setText("90");
			editGammaRecip->setText("60");
			break;

		case CRYS_MONOCLINIC:
			editA->setEnabled(1);
			editB->setEnabled(1);
			editC->setEnabled(1);
			editAlpha->setEnabled(1);
			editBeta->setEnabled(0);
			editGamma->setEnabled(0);

			editARecip->setEnabled(1);
			editBRecip->setEnabled(1);
			editCRecip->setEnabled(1);
			editAlphaRecip->setEnabled(1);
			editBetaRecip->setEnabled(0);
			editGammaRecip->setEnabled(0);

			editBeta->setText("90");
			editGamma->setText("90");
			editBetaRecip->setText("90");
			editGammaRecip->setText("90");
			break;

		case CRYS_ORTHORHOMBIC:
			editA->setEnabled(1);
			editB->setEnabled(1);
			editC->setEnabled(1);
			editAlpha->setEnabled(0);
			editBeta->setEnabled(0);
			editGamma->setEnabled(0);

			editARecip->setEnabled(1);
			editBRecip->setEnabled(1);
			editCRecip->setEnabled(1);
			editAlphaRecip->setEnabled(0);
			editBetaRecip->setEnabled(0);
			editGammaRecip->setEnabled(0);

			editAlpha->setText("90");
			editBeta->setText("90");
			editGamma->setText("90");
			editAlphaRecip->setText("90");
			editBetaRecip->setText("90");
			editGammaRecip->setText("90");
			break;

		case CRYS_TETRAGONAL:
			editA->setEnabled(1);
			editB->setEnabled(0);
			editC->setEnabled(1);
			editAlpha->setEnabled(0);
			editBeta->setEnabled(0);
			editGamma->setEnabled(0);

			editARecip->setEnabled(1);
			editBRecip->setEnabled(0);
			editCRecip->setEnabled(1);
			editAlphaRecip->setEnabled(0);
			editBetaRecip->setEnabled(0);
			editGammaRecip->setEnabled(0);

			editB->setText(editA->text());
			editBRecip->setText(editARecip->text());
			editAlpha->setText("90");
			editBeta->setText("90");
			editGamma->setText("90");
			editAlphaRecip->setText("90");
			editBetaRecip->setText("90");
			editGammaRecip->setText("90");
			break;

		case CRYS_TRIGONAL:
			editA->setEnabled(1);
			editB->setEnabled(0);
			editC->setEnabled(0);
			editAlpha->setEnabled(1);
			editBeta->setEnabled(0);
			editGamma->setEnabled(0);

			editARecip->setEnabled(1);
			editBRecip->setEnabled(0);
			editCRecip->setEnabled(0);
			editAlphaRecip->setEnabled(1);
			editBetaRecip->setEnabled(0);
			editGammaRecip->setEnabled(0);

			editB->setText(editA->text());
			editC->setText(editA->text());
			editBRecip->setText(editARecip->text());
			editCRecip->setText(editARecip->text());
			editBeta->setText(editAlpha->text());
			editGamma->setText(editAlpha->text());
			editBetaRecip->setText(editAlphaRecip->text());
			editGammaRecip->setText(editAlphaRecip->text());
			break;

		case CRYS_TRICLINIC:
		case CRYS_NOT_SET:
		default:
			editA->setEnabled(1);
			editB->setEnabled(1);
			editC->setEnabled(1);
			editAlpha->setEnabled(1);
			editBeta->setEnabled(1);
			editGamma->setEnabled(1);

			editARecip->setEnabled(1);
			editBRecip->setEnabled(1);
			editCRecip->setEnabled(1);
			editAlphaRecip->setEnabled(1);
			editBetaRecip->setEnabled(1);
			editGammaRecip->setEnabled(1);
			break;
	}
}

void TazDlg::CalcPeaksRecip()
{
	double a = editARecip->text().toDouble();
	double b = editBRecip->text().toDouble();
	double c = editCRecip->text().toDouble();

	double alpha = editAlphaRecip->text().toDouble()/180.*M_PI;
	double beta = editBetaRecip->text().toDouble()/180.*M_PI;
	double gamma = editGammaRecip->text().toDouble()/180.*M_PI;

	tl::Lattice<double> lattice(a,b,c, alpha,beta,gamma);
	tl::Lattice<double> recip = lattice.GetRecip();

	editA->setText(dtoqstr(recip.GetA()));
	editB->setText(dtoqstr(recip.GetB()));
	editC->setText(dtoqstr(recip.GetC()));
	editAlpha->setText(dtoqstr(recip.GetAlpha()/M_PI*180.));
	editBeta->setText(dtoqstr(recip.GetBeta()/M_PI*180.));
	editGamma->setText(dtoqstr(recip.GetGamma()/M_PI*180.));

	m_bUpdateRecipEdits = 0;
	CalcPeaks();
	m_bUpdateRecipEdits = 1;
}

void TazDlg::CalcPeaks()
{
	if(!m_sceneRecip.GetTriangle())
		return;

	try
	{
		// lattice
		double a = editA->text().toDouble();
		double b = editB->text().toDouble();
		double c = editC->text().toDouble();

		double alpha = editAlpha->text().toDouble()/180.*M_PI;
		double beta = editBeta->text().toDouble()/180.*M_PI;
		double gamma = editGamma->text().toDouble()/180.*M_PI;

		tl::Lattice<double> lattice(a,b,c, alpha,beta,gamma);
		tl::Lattice<double> recip_unrot = lattice.GetRecip();




		// scattering plane
		double dX0 = editScatX0->text().toDouble();
		double dX1 = editScatX1->text().toDouble();
		double dX2 = editScatX2->text().toDouble();
		ublas::vector<double> vecPlaneX = dX0*recip_unrot.GetVec(0) +
										dX1*recip_unrot.GetVec(1) +
										dX2*recip_unrot.GetVec(2);

		double dY0 = editScatY0->text().toDouble();
		double dY1 = editScatY1->text().toDouble();
		double dY2 = editScatY2->text().toDouble();
		ublas::vector<double> vecPlaneY = dY0*recip_unrot.GetVec(0) +
										dY1*recip_unrot.GetVec(1) +
										dY2*recip_unrot.GetVec(2);

		//----------------------------------------------------------------------
		// show integer up vector
		unsigned int iMaxDec = 4;	// TODO: determine max. # of entered decimals
		ublas::vector<int> ivecUp = tl::cross_3(
			tl::make_vec<ublas::vector<int>>({int(dX0*std::pow(10, iMaxDec)),
											int(dX1*std::pow(10, iMaxDec)),
											int(dX2*std::pow(10, iMaxDec))}),
			tl::make_vec<ublas::vector<int>>({int(dY0*std::pow(10, iMaxDec)),
											int(dY1*std::pow(10, iMaxDec)),
											int(dY2*std::pow(10, iMaxDec))}));
		ivecUp = tl::get_gcd_vec(ivecUp);
		editScatZ0->setText(std::to_string(ivecUp[0]).c_str());
		editScatZ1->setText(std::to_string(ivecUp[1]).c_str());
		editScatZ2->setText(std::to_string(ivecUp[2]).c_str());
		//----------------------------------------------------------------------

		ublas::vector<double> vecX0 = ublas::zero_vector<double>(3);
		tl::Plane<double> plane(vecX0, vecPlaneX, vecPlaneY);
		if(!plane.IsValid())
		{
			tl::log_err("Invalid scattering plane.");
			return;
		}


		if(m_pGotoDlg)
		{
			m_pGotoDlg->SetLattice(lattice);
			m_pGotoDlg->SetScatteringPlane(tl::make_vec({dX0, dX1, dX2}), tl::make_vec({dY0, dY1, dY2}));
			m_pGotoDlg->CalcSample();
		}


		/*// rotated lattice
		double dPhi = spinRotPhi->value() / 180. * M_PI;
		double dTheta = spinRotTheta->value() / 180. * M_PI;
		double dPsi = spinRotPsi->value() / 180. * M_PI;
		//lattice.RotateEuler(dPhi, dTheta, dPsi);*/

		ublas::vector<double> dir0 = plane.GetDir0();
		ublas::vector<double> dirup = plane.GetNorm();
		ublas::vector<double> dir1 = tl::cross_3(dirup, dir0);

		double dDir0Len = ublas::norm_2(dir0);
		double dDir1Len = ublas::norm_2(dir1);
		double dDirUpLen = ublas::norm_2(dirup);

		if(tl::float_equal(dDir0Len, 0.) || tl::float_equal(dDir1Len, 0.) || tl::float_equal(dDirUpLen, 0.)
			|| tl::is_nan_or_inf<double>(dDir0Len) || tl::is_nan_or_inf<double>(dDir1Len) || tl::is_nan_or_inf<double>(dDirUpLen))
		{
			tl::log_err("Invalid scattering plane.");
			return;
		}

		dir0 /= dDir0Len;
		dir1 /= dDir1Len;
		//dirup /= dDirUpLen;

		//lattice.RotateEulerRecip(dir0, dir1, dirup, dPhi, dTheta, dPsi);
		tl::Lattice<double> recip = lattice.GetRecip();



		if(m_bUpdateRecipEdits)
		{
			editARecip->setText(dtoqstr(recip.GetA()));
			editBRecip->setText(dtoqstr(recip.GetB()));
			editCRecip->setText(dtoqstr(recip.GetC()));
			editAlphaRecip->setText(dtoqstr(recip.GetAlpha()/M_PI*180.));
			editBetaRecip->setText(dtoqstr(recip.GetBeta()/M_PI*180.));
			editGammaRecip->setText(dtoqstr(recip.GetGamma()/M_PI*180.));
		}

		const std::wstring& strAA = tl::get_spec_char_utf16("AA");
		const std::wstring& strMinus = tl::get_spec_char_utf16("sup-");
		const std::wstring& strThree = tl::get_spec_char_utf16("sup3");

		double dVol = lattice.GetVol();
		double dVol_recip = recip.GetVol() /*/ (2.*M_PI*2.*M_PI*2.*M_PI)*/;

		std::wostringstream ostrSample;
		ostrSample.precision(8);
		ostrSample << "Sample";
		ostrSample << " - ";
		ostrSample << "Unit Cell Vol.: ";
		ostrSample << "Real: " << dVol << " " << strAA << strThree;
		ostrSample << ", Recip.: " << dVol_recip << " " << strAA << strMinus << strThree;
		groupSample->setTitle(QString::fromWCharArray(ostrSample.str().c_str()));


		const char* pcCryTy = "<not set>";
		SpaceGroup *pSpaceGroup = 0;
		int iSpaceGroupIdx = comboSpaceGroups->currentIndex();
		if(iSpaceGroupIdx != 0)
			pSpaceGroup = (SpaceGroup*)comboSpaceGroups->itemData(iSpaceGroupIdx).value<void*>();

		if(pSpaceGroup)
			pcCryTy = pSpaceGroup->GetCrystalSystemName();

		editCrystalSystem->setText(pcCryTy);

		m_sceneRecip.GetTriangle()->CalcPeaks(lattice, recip, recip_unrot, plane, pSpaceGroup);
		if(m_sceneRecip.getSnapq())
			m_sceneRecip.SnapToNearestPeak(m_sceneRecip.GetTriangle()->GetNodeGq());
		m_sceneRecip.emitUpdate();

		if(m_pRecip3d)
			m_pRecip3d->CalcPeaks(lattice, recip, recip_unrot, plane, pSpaceGroup);
	}
	catch(const std::exception& ex)
	{
		m_sceneRecip.GetTriangle()->ClearPeaks();
		tl::log_err(ex.what());
	}
}

void TazDlg::RepopulateSpaceGroups()
{
	if(!m_pmapSpaceGroups)
		return;

	for(int iCnt=comboSpaceGroups->count()-1; iCnt>0; --iCnt)
		comboSpaceGroups->removeItem(iCnt);

	std::string strFilter = editSpaceGroupsFilter->text().toStdString();

	for(const t_mapSpaceGroups::value_type& pair : *m_pmapSpaceGroups)
	{
		const std::string& strName = pair.second.GetName();

		typedef const boost::iterator_range<std::string::const_iterator> t_striterrange;
		if(strFilter!="" &&
				!boost::ifind_first(t_striterrange(strName.begin(), strName.end()),
									t_striterrange(strFilter.begin(), strFilter.end())))
			continue;

		comboSpaceGroups->insertItem(comboSpaceGroups->count(),
									strName.c_str(),
									QVariant::fromValue((void*)&pair.second));
	}
}


//--------------------------------------------------------------------------------
// spurion stuff

void TazDlg::ShowSpurions()
{
	if(!m_pSpuri)
	{
		m_pSpuri = new SpurionDlg(this, &m_settings);

		QObject::connect(&m_sceneRecip, SIGNAL(paramsChanged(const RecipParams&)),
						m_pSpuri, SLOT(paramsChanged(const RecipParams&)));

		m_sceneRecip.emitAllParams();
	}

	m_pSpuri->show();
	m_pSpuri->activateWindow();
}

void TazDlg::spurionInfo(const tl::ElasticSpurion& spuri,
					const std::vector<tl::InelasticSpurion<double>>& vecInelCKI,
					const std::vector<tl::InelasticSpurion<double>>& vecInelCKF)
{
	std::ostringstream ostrMsg;
	if(spuri.bAType || spuri.bMType || vecInelCKI.size() || vecInelCKF.size())
		ostrMsg << "Warning: ";

	if(spuri.bAType || spuri.bMType)
	{
		ostrMsg << "Spurious elastic scattering of type ";
		if(spuri.bAType && spuri.bMType)
		{
			ostrMsg << "A and M";
			ostrMsg << (spuri.bAKfSmallerKi ? " (kf<ki)" : " (kf>ki)");
		}
		else if(spuri.bAType)
		{
			ostrMsg << "A";
			ostrMsg << (spuri.bAKfSmallerKi ? " (kf<ki)" : " (kf>ki)");
		}
		else if(spuri.bMType)
		{
			ostrMsg << "M";
			ostrMsg << (spuri.bMKfSmallerKi ? " (kf<ki)" : " (kf>ki)");
		}
		ostrMsg << " detected.";

		if(vecInelCKI.size() || vecInelCKF.size())
			ostrMsg << " ";
	}

	const std::string& strDelta = tl::get_spec_char_utf8("Delta");

	if(vecInelCKI.size())
	{
		ostrMsg << "Spurious inelastic CKI scattering at "
				<< strDelta << "E = ";
		for(unsigned int iInel=0; iInel<vecInelCKI.size(); ++iInel)
		{
			ostrMsg << vecInelCKI[iInel].dE_meV << " meV";
			if(iInel != vecInelCKI.size()-1)
				ostrMsg << ", ";
		}
		ostrMsg << " detected.";

		if(vecInelCKF.size())
			ostrMsg << " ";
	}

	if(vecInelCKF.size())
	{
		ostrMsg << "Spurious inelastic CKF scattering at "
				<< strDelta << "E = ";
		for(unsigned int iInel=0; iInel<vecInelCKF.size(); ++iInel)
		{
			ostrMsg << vecInelCKF[iInel].dE_meV << " meV";
			if(iInel != vecInelCKF.size()-1)
				ostrMsg << ", ";
		}
		ostrMsg << " detected.";
	}

	m_pStatusMsg->setText(QString::fromUtf8(ostrMsg.str().c_str(), ostrMsg.str().size()));
}



//--------------------------------------------------------------------------------
// reso stuff
void TazDlg::InitReso()
{
	if(!m_pReso)
	{
		m_pReso = new ResoDlg(this, &m_settings);

		QObject::connect(this, SIGNAL(ResoParamsChanged(const ResoParams&)),
						m_pReso, SLOT(ResoParamsChanged(const ResoParams&)));
		QObject::connect(&m_sceneRecip, SIGNAL(paramsChanged(const RecipParams&)),
						m_pReso, SLOT(RecipParamsChanged(const RecipParams&)));
		QObject::connect(&m_sceneReal, SIGNAL(paramsChanged(const RealParams&)),
						m_pReso, SLOT(RealParamsChanged(const RealParams&)));

		UpdateDs();
		UpdateMonoSense();
		UpdateSampleSense();
		UpdateAnaSense();

		m_sceneRecip.emitAllParams();
		m_sceneReal.emitAllParams();
	}
}

void TazDlg::ShowResoParams()
{
	InitReso();

	m_pReso->show();
	m_pReso->activateWindow();
}

void TazDlg::ShowResoEllipses()
{
	InitReso();

	if(!m_pEllipseDlg)
	{
		m_pEllipseDlg = new EllipseDlg(this, &m_settings);
		QObject::connect(m_pReso, SIGNAL(ResoResults(const ublas::matrix<double>&, const ublas::vector<double>&)),
						 m_pEllipseDlg, SLOT(SetParams(const ublas::matrix<double>&, const ublas::vector<double>&)));

		m_pReso->EmitResults();
	}

	m_pEllipseDlg->show();
	m_pEllipseDlg->activateWindow();
}

void TazDlg::ShowResoEllipses3D()
{
	InitReso();

	if(!m_pEllipseDlg3D)
	{
		m_pEllipseDlg3D = new EllipseDlg3D(this, &m_settings);
		QObject::connect(m_pReso, SIGNAL(ResoResults(const ublas::matrix<double>&, const ublas::vector<double>&)),
						 m_pEllipseDlg3D, SLOT(SetParams(const ublas::matrix<double>&, const ublas::vector<double>&)));

		m_pReso->EmitResults();
	}

	m_pEllipseDlg3D->show();
	m_pEllipseDlg3D->activateWindow();
}
