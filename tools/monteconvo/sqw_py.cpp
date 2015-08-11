/*
 * S(Q,w) python interface
 * @author tweber
 * @date aug-2015
 * @license GPLv2
 */

#include "sqw_py.h"
#include "tlibs/string/string.h"

SqwPy::SqwPy(const char* pcFile)
{
	std::string strFile = pcFile;
	std::string strDir = tl::get_dir(strFile);
	std::string strMod = tl::get_file_noext(tl::get_file(strFile));

	try
	{
		::Py_Initialize();

		m_sys = py::import("sys");
		py::dict sysdict = py::extract<py::dict>(m_sys.attr("__dict__"));
		py::list path = py::extract<py::list>(sysdict["path"]);
		path.append(strDir.c_str());
		path.append(".");

		m_mod = py::import(strMod.c_str());
		py::dict moddict = py::extract<py::dict>(m_mod.attr("__dict__"));
		m_Sqw = moddict["Sqw"];

		m_bOk = 1;
	}
	catch(const py::error_already_set& ex)
	{
		PyErr_Print();
		PyErr_Clear();

		m_bOk = 0;
	}
}

SqwPy::~SqwPy()
{	
	//::Py_Finalize();
}


double SqwPy::operator()(double dh, double dk, double dl, double dE) const
{
	try
	{
		//std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_mtx));
		return py::extract<double>(m_Sqw(dh, dk, dl, dE));
	}
	catch(const py::error_already_set& ex)
	{
		PyErr_Print();
		PyErr_Clear();
	}

	return 0.;
}
