// gdsTech.h

#ifndef _TRANSFORMER_2O2_H_
#define _TRANSFORMER_2O2_H_

#include <string>
#include <vector>
#include <string>

#include "mcell_fast_henry.h"

#include "mcell_library.h"
#include "gdstk.h"
#include "mcell_command_line.h"
#include <math.h>
#include <mutex>
#include "BS_thread_pool.hpp"

class Transformer2o2
{

public:
	Gdslib *l;
	FastHenryLib *fhl;
	size_t numTasksPerThread; // max number of tasks per one thread

	void clear()
	{
		if (l != NULL)
		{
			l->clear();
			delete l;
			l = NULL;
		}
		if (fhl != NULL)
		{
			fhl->clear();
			delete fhl;
			fhl = NULL;
		}
	}

	~Transformer2o2()
	{
		this->clear();
	}

	Transformer2o2(const char *fileName) : l(NULL), fhl(NULL)
	{
		this->l = new Gdslib(fileName);
		this->fhl = new FastHenryLib(fileName);
		this->numTasksPerThread = 1028;
	}

	int drc(double dp, double ds, double wp, double ws, double sp, double ss, double spin, double lpin, double wtp, double wts, int geometry, string topM);
	// ###############################################################################################
	// # transformer2o2
	// # 		: check if geometry is feasible
	// ###############################################################################################
	// # dp 			: outer diameter of primary
	// # ds 			: outer diameter of secondary
	// # wp				: primary line width
	// # ws				: secondary line width
	// # sp 			: space between turns primary
	// # ss 			: space between turns secondary
	// # spin 			: space between pins
	// # lpin           : pin length (default should be 2w)
	// # wtp          	: width of line for tapped primary if wtp<=0 no tapping
	// # wts          	: width of line for tapped secondary if wts<=0 no tapping
	// # geometry		: 0-> octagonal, else rectangular
	// # topM			: top metal layer (metal name)
	// ###############################################################################################

	int genCell(Cell &result, double dp, double ds, double wp, double ws, double sp, double ss, double spin, double lpin, double wtp, double wts, int geometry, string topM, bool fasterCap = false);
	// ###############################################################################################
	// # transformer2o2
	// # 		: generate cell
	// ###############################################################################################
	// # dp 			: outer diameter of primary
	// # ds 			: outer diameter of secondary
	// # wp				: primary line width
	// # ws				: secondary line width
	// # sp 			: space between turns primary
	// # ss 			: space between turns secondary
	// # spin 			: space between pins
	// # lpin           : pin length (default should be 2w)
	// # wtp          	: width of line for tapped primary if wtp<=0 no tapping
	// # wts          	: width of line for tapped secondary if wts<=0 no tapping
	// # geometry		: 0-> octagonal, else rectangular
	// # topM			: top metal layer (metal name)
	// ###############################################################################################

	static void printHelp();
	// ###############################################################################################
	// # transformer2o2
	// #	: print command line options for transformer1o1
	// #	this function is static so it can be called without any object Transformer1o1::printHelp
	// ###############################################################################################

	int process(Command *cm);
	// ###############################################################################################
	// # transformer2o2
	// #		: process command line options
	// ###############################################################################################

	int FastHenryGenCell(FastHenry *fh, double dp, double ds, double wp, double ws, double sp, double ss, double spin, double lpin, double wtp, double wts, int geometry, string topM, bool fasterCap);
	// ###############################################################################################
	// # transformer1o1
	// # 		: generate cell
	// ###############################################################################################
	// # dp 			: outer diameter of primary
	// # ds 			: outer diameter of secondary
	// # wp				: primary line width
	// # ws				: secondary line width
	// # sp 			: space between turns primary
	// # ss 			: space between turns secondary
	// # spin 			: space between pins
	// # lpin           	: pin length (default should be 2w)
	// # wtp          	: width of line for tapped primary if wtp<=0 no tapping
	// # wts          	: width of line for tapped secondary if wts<=0 no tapping
	// # geometry		: 0-> octagonal, else rectangular
	// # topM			: top metal layer (metal name)
	// ###############################################################################################

	int FastHenryProcess(Command *cm);
	// ###############################################################################################
	// # transformer1o2
	// #		: process command line options
	// ###############################################################################################

	int FastHenry2Process(Command *cm);
	// ###############################################################################################
	// # transformer1o2
	// #		: process command line options
	// ###############################################################################################

	int threadNumber(Command *cm);
	// ###############################################################################################
	// # calculate number of threads to be used in thread pool
	// ###############################################################################################
};

#endif