// gcc -o tst_clipper tst_clipper.cpp -lstdc++ -lclipper-core
#include <clipper/clipper.h>

void list_sgs()
{
	for(int iSg=1; iSg<=230; ++iSg)
	{
		clipper::Spgr_descr dsc(iSg);
		std::cout << "Spacegroup " << iSg << ": " << dsc.symbol_hm() << std::endl;
	}
}

void test_hkls()
{
	std::string strSg;

	std::cout << "Enter spacegroup: ";
	std::getline(std::cin, strSg);
	clipper::Spgr_descr dsc(strSg);

	std::cout << "Nr.: " << dsc.spacegroup_number() << std::endl;
	std::cout << "Hall: " << dsc.symbol_hall() << std::endl;
	std::cout << "HM: " << dsc.symbol_hm() << std::endl;

	clipper::Spacegroup sg(dsc);
	std::cout << "Laue group: " << sg.symbol_laue() << std::endl;

	int iNumSymOps = sg.num_symops();
	std::cout << "Number of symmetry operations: " << iNumSymOps << std::endl;
	for(int iSymOp=0; iSymOp<iNumSymOps; ++iSymOp)
	{
		const clipper::Symop& symop = sg.symop(iSymOp);
		std::cout << "Symmetry operation " << (iSymOp+1) << ": " << symop.format() << std::endl;
	}

	std::cout << std::endl;
	while(1)
	{
		int h,k,l;
		std::cout << "Enter hkl: ";
		std::cin >> h >> k >> l;
		clipper::HKL_class hkl = sg.hkl_class(clipper::HKL(h,k,l));

		std::cout << "allowed: " << (!hkl.sys_abs()) << std::endl;
		std::cout << "centric: " << hkl.centric() << std::endl;
		std::cout << "allowed phase: " << hkl.allowed() << std::endl;
		std::cout << "epsilon: " << hkl.epsilon() << std::endl;
		std::cout << std::endl;
	}
}

int main()
{
	try
	{
		//list_sgs();
		test_hkls();
	}
	catch(const clipper::Message_fatal& ex)
	{
		std::cerr << "Fatal error." << std::endl;
	}
	catch(const std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;
	}

	return 0;
}
