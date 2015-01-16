// tests all available cl devices
// gcc -o tst_cl tst_cl.cpp -lstdc++ -lOpenCL -std=c++11
// @author tw

#include <CL/cl.hpp>
#include <iostream>

//#define USE_DOUBLE

#ifdef USE_DOUBLE
	typedef double t_real;
#else
	typedef float t_real;
#endif

static const std::string strAux =
R"RAW(
#ifdef USE_DOUBLE
	#pragma OPENCL EXTENSION cl_khr_fp64: enable
	typedef double t_real;
#else
	typedef float t_real;
#endif

	static void copy(int iSize, __global t_real *pdOut, const __global t_real *pdIn)
	{
		for(int i=0; i<iSize; ++i)
			pdOut[i] = pdIn[i];
	}

	static void scale(int iSize, __global t_real *pd0, t_real d1)
	{
		for(int i=0; i<iSize; ++i)
			pd0[i] *= d1;
	}
)RAW";

static const std::string strProg =
R"RAW(
	__kernel void tst_cl_kern_loop(const __global t_real* pIn, __global t_real* pOut)
	{
		copy(4, pOut, pIn);
		scale(4, pOut, 2.);
	}

	__kernel void tst_cl_kern(const __global t_real* pIn, __global t_real* pOut)
	{
		const int i = get_global_id(0);
		pOut[i] = pIn[i] * 2.;
	}
)RAW";

int main()
{
	const bool bUseLoop = 0;

	std::vector<cl::Platform> vecPlat;
	cl::Platform::get(&vecPlat);
	std::cout << "Found " << vecPlat.size() << " platforms." << std::endl;
	std::cout << "------------------------------------------------------------" << std::endl;

	for(cl::Platform& plat : vecPlat)
	{
		std::string strVendor, strName, strVer, strProf, strExt;
		plat.getInfo(CL_PLATFORM_VENDOR, &strVendor);
		plat.getInfo(CL_PLATFORM_NAME, &strName);
		plat.getInfo(CL_PLATFORM_VERSION, &strVer);
		plat.getInfo(CL_PLATFORM_PROFILE, &strProf);
		plat.getInfo(CL_PLATFORM_EXTENSIONS, &strExt);

		std::cout << "Platform: " << strVendor << ", " << strName << ", " 
					<< strVer << ", " << strProf << std::endl;
		std::cout << "Extensions: " << strExt << std::endl;

		std::vector<cl::Device> vecDevCPU, vecDevGPU;
		plat.getDevices(CL_DEVICE_TYPE_CPU, &vecDevCPU);
		plat.getDevices(CL_DEVICE_TYPE_GPU, &vecDevGPU);

		std::cout << "Found " << vecDevCPU.size() << " CPUs and "
					<< vecDevGPU.size() << " GPUs." << std::endl;

		std::vector<cl::Device> vecDevs = vecDevCPU;
		vecDevs.insert(vecDevCPU.end(), vecDevGPU.begin(), vecDevGPU.end());

		for(cl::Device& dev : vecDevs)
		{
			std::string strDevName;
			cl_uint iUnits;
			dev.getInfo(CL_DEVICE_NAME, &strDevName);
			dev.getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &iUnits);
			std::cout << "Device: " << strDevName << std::endl;
			std::cout << "Number of computation units: " << iUnits << std::endl;


			std::cout << "Executing test kernel." << std::endl;

			std::vector<cl::Device> vecDevsCont = {dev};
			cl::Context cont(vecDevsCont);


			cl::Program::Sources vecSrc =
			{
				std::pair<const char*, std::size_t>(strAux.c_str(), strAux.length()),
				std::pair<const char*, std::size_t>(strProg.c_str(), strProg.length())
			};

			cl_int iErr, iErr2;
			cl::Program prog(cont, vecSrc, &iErr);
			if(iErr < 0)
			{
				std::cerr << "Error: Cannot create program." << std::endl;
				continue;
			}

			std::string strOpts = "";
#ifdef USE_DOUBLE
			strOpts += "-DUSE_DOUBLE";
#endif

			iErr = prog.build(strOpts.c_str());
			if(iErr < 0)
			{
				std::string strErrLog;
				prog.getBuildInfo(dev, CL_PROGRAM_BUILD_LOG, &strErrLog);

				std::cerr << "Error: Cannot build program." << std::endl;
				std::cerr << "Log: " << strErrLog << std::endl;
				continue;
			}


			cl::Kernel kern(prog, bUseLoop ? "tst_cl_kern_loop" : "tst_cl_kern", &iErr);
			if(iErr < 0)
			{
				std::cerr << "Error: Cannot create kernel." << std::endl;
				continue;
			}

			cl::CommandQueue qu(cont, dev, 0, &iErr);
			if(iErr < 0)
			{
				std::cerr << "Error: Cannot create command queue." << std::endl;
				continue;
			}

			t_real dIn[] = {1., 2., 3., 4.};
			t_real dOut[4];

			cl::Buffer bufVecIn(cont, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, sizeof dIn, dIn, &iErr);
			cl::Buffer bufVecOut(cont, CL_MEM_WRITE_ONLY, sizeof dIn, nullptr, &iErr2);
			if(iErr<0 || iErr2<0)
			{
				std::cerr << "Error: Cannot create input/output buffers." << std::endl;
				continue;
			}

			kern.setArg(0, bufVecIn);
			kern.setArg(1, bufVecOut);

			cl::Event evtKern, evtOut;
			cl::NDRange ranOffs, ranGlob(4), ranLoc;
			if(bUseLoop)
				qu.enqueueTask(kern, 0, &evtKern);
			else
				qu.enqueueNDRangeKernel(kern, ranOffs, ranGlob, ranLoc, 0, &evtKern);

			std::vector<cl::Event> vecEvts = {evtKern};
			qu.enqueueReadBuffer(bufVecOut, 0, 0, sizeof dOut, dOut, &vecEvts, &evtOut);
			evtKern.wait(); evtOut.wait();

			std::cout << "Result: 2 * (1 2 3 4) = ";
			for(int i=0; i<4; ++i)
				std::cout << dOut[i] << " ";
			std::cout << std::endl;
		}

		std::cout << "------------------------------------------------------------" << std::endl;
	}

	return 0;
}
