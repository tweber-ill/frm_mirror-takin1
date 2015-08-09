//gcc -std=c++11 -I /usr/include/python2.7 -o pytst pytst.cpp -lstdc++ -lboost_python -lpython2.7

#include <iostream>

#include <boost/python.hpp>
namespace py = boost::python;


void call_py_fkt()
{
	try
	{
		py::object sys = py::import("sys");
		py::object sysdict = sys.attr("__dict__");
		py::list path = py::extract<py::list>(sysdict["path"]);
		path.append(".");

		py::object mod = py::import("tstsqw");
		py::object moddict = mod.attr("__dict__");
		py::object Sqw = moddict["Sqw"];

		for(double dE=0.; dE<1.; dE+=0.1)
		{
			double dS = py::extract<double>(Sqw(1., 1., 1., dE));
			std::cout << dS << std::endl;
		}
	}
	catch(const py::error_already_set& ex)
	{
		PyErr_Print();
		PyErr_Clear();
	}
}

int main(int argc, char** argv)
{
	Py_Initialize();
	call_py_fkt();
	//Py_Finalize();

	return 0;
}
