/*
 * S(Q,w) python interface
 * @author tweber
 * @date aug-2015
 * @license GPLv2
 */

#ifndef __SQW_PY_H__
#define __SQW_PY_H__

#include "sqw.h"
//#include <mutex>
#include <boost/python.hpp>
namespace py = boost::python;


class SqwPy : public SqwBase
{
protected:
	//std::mutex m_mtx;
	py::object m_sys, m_mod;
	py::object m_Sqw;

public:
	SqwPy(const char* pcFile);
	virtual ~SqwPy();

	virtual double operator()(double dh, double dk, double dl, double dE) const override;
};

#endif
