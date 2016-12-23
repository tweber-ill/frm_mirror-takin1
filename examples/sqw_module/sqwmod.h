/**
 * S(q,w) module example
 * @author tweber
 * @date 2016
 * @license GPLv2
 */

#ifndef __MCONV_SQW_MOD_H__
#define __MCONV_SQW_MOD_H__

#include "tools/monteconvo/sqwbase.h"
#include <tuple>

class SqwMod : public SqwBase
{
	public:
		using SqwBase::t_var;

	protected:
		t_real_reso m_dT = t_real_reso(100);
		t_real_reso m_dSigma = t_real_reso(0.05);

	public:
		SqwMod();
		SqwMod(const std::string& strCfgFile);
		virtual ~SqwMod();

		std::tuple<t_real_reso, t_real_reso>
		dispersion(t_real_reso dh, t_real_reso dk, t_real_reso dl) const;

		virtual t_real_reso operator()(t_real_reso dh, t_real_reso dk, t_real_reso dl, t_real_reso dE) const override;

		virtual std::vector<t_var> GetVars() const override;
		virtual void SetVars(const std::vector<t_var>&) override;
		virtual bool SetVarIfAvail(const std::string& strKey, const std::string& strNewVal) override;

		virtual SqwBase* shallow_copy() const override;
};

#endif
