/**
 * S(Q,w) processes
 * @author tweber
 * @date sep-2016
 * @license GPLv2
 */

#ifndef __SQW_PROC_IMPL_H__
#define __SQW_PROC_IMPL_H__

#include "sqw_proc.h"
#include "tlibs/string/string.h"
#include "tlibs/log/log.h"
#include "tlibs/math/rand.h"

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
//#include <boost/interprocess/containers/vector.hpp>

namespace proc = boost::interprocess;
using t_real = t_real_reso;


// ----------------------------------------------------------------------------
// types and conversions

template<class t_ch=char>
using t_sh_str_alloc_gen = proc::allocator<t_ch, proc::managed_shared_memory::segment_manager>;
template<class t_ch=char>
using t_sh_str_gen = proc::basic_string<t_ch, std::char_traits<t_ch>, t_sh_str_alloc_gen<t_ch>>;

using t_sh_str_alloc = t_sh_str_alloc_gen<char>;
using t_sh_str = t_sh_str_gen<char>;

/*
using t_sh_vec_alloc = proc::allocator<std::tuple<t_sh_str, t_sh_str, t_sh_str>,
	proc::managed_shared_memory::segment_manager>;
using t_sh_vec = proc::vector<std::tuple<t_sh_str, t_sh_str, t_sh_str>, t_sh_vec_alloc>;

template<class t_str_from, class t_str_to>
static std::tuple<t_str_to, t_str_to, t_str_to>
var_to_procvar(const std::tuple<t_str_from, t_str_from, t_str_from>& var)
{
	return std::tuple<t_str_to, t_str_to, t_str_to>
		(std::get<0>(var).c_str(), std::get<1>(var).c_str(), std::get<2>(var).c_str());
}
*/

static void pars_to_str(t_sh_str& str, const std::vector<SqwBase::t_var>& vec)
{
	str.clear();

	for(const SqwBase::t_var& var : vec)
	{
		str.append(std::get<0>(var).c_str()); str.append("#,#");
		str.append(std::get<1>(var).c_str()); str.append("#,#");
		str.append(std::get<2>(var).c_str());
		str.append("#;#");
	}
}

static std::vector<SqwBase::t_var> str_to_pars(const t_sh_str& _str)
{
	std::string str;
	for(const t_sh_str::value_type& ch : _str) str.push_back(ch);

	std::vector<std::string> vecLines;
	tl::get_tokens_seq<std::string, std::string, std::vector>
		(str, "#;#", vecLines, 1);


	std::vector<SqwBase::t_var> vec;
	vec.reserve(vecLines.size());

	for(const std::string& strLine : vecLines)
	{
		if(strLine.length() == 0) continue;

		std::vector<std::string> vecVals;
		tl::get_tokens_seq<std::string, std::string, std::vector>
			(strLine, "#,#", vecVals, 1);

		if(vecVals.size() != 3)
		{
			tl::log_err("Wrong size of parameters: \"", strLine, "\".");
			continue;
		}

		SqwBase::t_var var;
		std::get<0>(var) = vecVals[0];
		std::get<1>(var) = vecVals[1];
		std::get<2>(var) = vecVals[2];
		vec.push_back(var);

		//tl::log_debug(std::get<0>(var), ", ", std::get<1>(var), ", ", std::get<2>(var));
	}
	return vec;
}
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// messages

#define MSG_QUEUE_SIZE 128
#define PARAM_MEM 1024*1024*1024

enum class ProcMsgTypes
{
	QUIT,
	NOP,

	SQW,
	GET_VARS,
	SET_VARS,
};

struct ProcMsg
{
	ProcMsgTypes ty = ProcMsgTypes::NOP;

	t_real dParam1, dParam2, dParam3, dParam4;
	t_real dRet;

	t_sh_str *pPars = nullptr;
};

static void msg_send(proc::message_queue& msgqueue, const ProcMsg& msg)
{
	try
	{
		msgqueue.send(&msg, sizeof(msg), 0);
	}
	catch(const std::exception& ex)
	{
		tl::log_err(ex.what());
	}
}

static ProcMsg msg_recv(proc::message_queue& msgqueue)
{
	ProcMsg msg;
	try
	{
		std::size_t iSize;
		unsigned int iPrio;
		msgqueue.receive(&msg, sizeof(msg), iSize, iPrio);

		if(iSize != sizeof(msg))
			tl::log_err("Message size mismatch.");
	}
	catch(const std::exception& ex)
	{
		tl::log_err(ex.what());
	}
	return msg;
}

// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// child process

// ----------------------------------------------------------------------------

template<class t_sqw>
static void child_proc(proc::message_queue& msgToParent, proc::message_queue& msgFromParent,
	const char* pcCfg)
{
	std::unique_ptr<t_sqw> pSqw(new t_sqw(pcCfg));

	while(1)
	{
		ProcMsg msg = msg_recv(msgFromParent);

		switch(msg.ty)
		{
			case ProcMsgTypes::SQW:
			{
				ProcMsg msgRet;
				msgRet.ty = ProcMsgTypes::SQW;
				msgRet.dRet = pSqw->operator()(msg.dParam1, msg.dParam2, msg.dParam3, msg.dParam4);
				msg_send(msgToParent, msgRet);
				break;
			}
			case ProcMsgTypes::GET_VARS:
			{
				ProcMsg msgRet;
				msgRet.ty = ProcMsgTypes::GET_VARS;
				msgRet.pPars = msg.pPars;	// use provided pointer to shared mem
				pars_to_str(*msgRet.pPars, pSqw->GetVars());
				msg_send(msgToParent, msgRet);
				break;
			}
			case ProcMsgTypes::SET_VARS:
			{
				pSqw->SetVars(str_to_pars(*msg.pPars));
				break;
			}
			case ProcMsgTypes::QUIT:
			{
				//tl::log_debug("Exiting child process");
				exit(0);
				break;
			}
		}
	}
}


// ----------------------------------------------------------------------------
// parent process

template<class t_sqw>
SqwProc<t_sqw>::SqwProc()
{
	++m_iRefCnt;
}

template<class t_sqw>
SqwProc<t_sqw>::SqwProc(const char* pcCfg)
	: m_pmtx(std::make_shared<std::mutex>()),
	m_strProcName(tl::rand_name<std::string>(8))
{
	++m_iRefCnt;

	try
	{
		tl::log_debug("Creating process memory \"", "takin_sqw_proc_*_", m_strProcName, "\".");

		m_pMem = std::make_shared<proc::managed_shared_memory>(proc::create_only,
			("takin_sqw_proc_mem_" + m_strProcName).c_str(), PARAM_MEM);
		m_pSharedPars = static_cast<void*>(m_pMem->construct<t_sh_str>
			(("takin_sqw_proc_params_" + m_strProcName).c_str())
			(t_sh_str_alloc(m_pMem->get_segment_manager())));

		m_pmsgIn = std::make_shared<proc::message_queue>(proc::create_only,
			("takin_sqw_proc_in_" + m_strProcName).c_str(), MSG_QUEUE_SIZE, sizeof(ProcMsg));
		m_pmsgOut = std::make_shared<proc::message_queue>(proc::create_only,
			("takin_sqw_proc_out_" + m_strProcName).c_str(), MSG_QUEUE_SIZE, sizeof(ProcMsg));

		m_pidChild = fork();
		if(m_pidChild < 0)
		{
			tl::log_err("Cannot fork process.");
			return;
		}
		else if(m_pidChild == 0)
		{
			child_proc<t_sqw>(*m_pmsgIn, *m_pmsgOut, pcCfg);
			exit(0);
		}

		m_bOk = 1;
	}
	catch(const std::exception& ex)
	{
		m_bOk = 0;
		tl::log_err(ex.what());
	}
}

template<class t_sqw>
SqwProc<t_sqw>::~SqwProc()
{
	try
	{
		if(m_pmsgOut)
		{
			ProcMsg msg;
			msg.ty = ProcMsgTypes::QUIT;
			msg_send(*m_pmsgOut, msg);
		}

		if(--m_iRefCnt == 0)
		{
			tl::log_debug("Removing process memory \"", "takin_sqw_proc_*_", m_strProcName, "\".");

			proc::shared_memory_object::remove(("takin_sqw_proc_mem_" + m_strProcName).c_str());

			proc::message_queue::remove(("takin_sqw_proc_in_" + m_strProcName).c_str());
			proc::message_queue::remove(("takin_sqw_proc_out_" + m_strProcName).c_str());
		}
	}
	catch(const std::exception&)
	{}
}


template<class t_sqw>
t_real SqwProc<t_sqw>::operator()(t_real dh, t_real dk, t_real dl, t_real dE) const
{
	std::lock_guard<std::mutex> lock(*m_pmtx);

	ProcMsg msg;
	msg.ty = ProcMsgTypes::SQW;
	msg.dParam1 = dh;
	msg.dParam2 = dk;
	msg.dParam3 = dl;
	msg.dParam4 = dE;
	msg_send(*m_pmsgOut, msg);

	ProcMsg msgS = msg_recv(*m_pmsgIn);
	return msgS.dRet;
}

template<class t_sqw>
std::vector<SqwBase::t_var> SqwProc<t_sqw>::GetVars() const
{
	std::lock_guard<std::mutex> lock(*m_pmtx);

	ProcMsg msg;
	msg.ty = ProcMsgTypes::GET_VARS;
	msg.pPars = static_cast<decltype(msg.pPars)>(m_pSharedPars);
	msg_send(*m_pmsgOut, msg);

	ProcMsg msgRet = msg_recv(*m_pmsgIn);
	return str_to_pars(*msg.pPars);
}

template<class t_sqw>
void SqwProc<t_sqw>::SetVars(const std::vector<SqwBase::t_var>& vecVars)
{
	std::lock_guard<std::mutex> lock(*m_pmtx);

	ProcMsg msg;
	msg.ty = ProcMsgTypes::SET_VARS;
	msg.pPars = static_cast<decltype(msg.pPars)>(m_pSharedPars);
	pars_to_str(*msg.pPars, vecVars);

	msg_send(*m_pmsgOut, msg);
}

template<class t_sqw>
SqwBase* SqwProc<t_sqw>::shallow_copy() const
{
	SqwProc* pSqw = new SqwProc();
	*static_cast<SqwBase*>(pSqw) = *static_cast<const SqwBase*>(this);

	pSqw->m_pmtx = this->m_pmtx;
	pSqw->m_pMem = this->m_pMem;
	pSqw->m_pmsgIn = this->m_pmsgIn;
	pSqw->m_pmsgOut = this->m_pmsgOut;
	pSqw->m_strProcName = this->m_strProcName;

	return pSqw;
}

// ----------------------------------------------------------------------------

#endif

