/*
 * random numbers
 * @author tweber
 * @date 16-aug-2013
 */

#include "rand.h"
#include "log.h"

#include <cstdlib>
#include <exception>
#include <time.h>
#include <sys/time.h>


std::mt19937/*_64*/ g_randeng;

void init_rand()
{
	// seed 0: random device
	unsigned int uiSeed0 = 0;
	try
	{
		std::random_device rnd;
		if(rnd.entropy() == 0)
			log_debug("Random seed entropy is zero!");
		uiSeed0 = rnd();
	}
	catch(const std::exception& ex)
	{
		log_debug(ex.what());
		uiSeed0 = 0;
	}


	// seed 1: time
	struct timeval timev;
	gettimeofday(&timev, 0);
	unsigned int uiSeed1 = timev.tv_usec;


	// total seed
	unsigned int uiSeed = uiSeed0 ^ uiSeed1;

	log_debug("Random seed: ", uiSeed0, ", time seed: ", uiSeed1, ", total seed: ", uiSeed, ".");
	init_rand_seed(uiSeed);
}

void init_rand_seed(unsigned int uiSeed)
{
	srand(uiSeed);
	g_randeng = std::mt19937/*_64*/(uiSeed);
}

unsigned int simple_rand(unsigned int iMax)
{
	return rand() % iMax;
}
