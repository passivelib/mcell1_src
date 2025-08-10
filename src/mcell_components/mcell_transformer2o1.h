#ifndef _TRANSFORMER_2O1_H_
#define _TRANSFORMER_2O1_H_

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

class Transformer2o1
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

	~Transformer2o1()
	{
		this->clear();
	}

	Transformer2o1(const char *fileName) : l(NULL), fhl(NULL)
	{
		this->l = new Gdslib(fileName);
		this->fhl = new FastHenryLib(fileName);

		this->numTasksPerThread = 1028;
	}

	int drc(double dp, double ds, double wp, double ws, double s, double sp, double lp, double wtp, double wts, int geometry, string topM);
	// ###############################################################################################
	// # transformer1o2
	// # 		: check if geometry is feasible
	// ###############################################################################################
	// # dp 			: outer diameter of primary
	// # ds 			: outer diameter of secondary
	// # wp				: line width of primary
	// # ws				: secondary line width
	// # ss 			: shift between primary and secondary
	// # sp 			: space between pins
	// # lp           	: pin length (default should be 2w)
	// # wtp          	: width of line for tapped primary if wtp<=0 no tapping
	// # wts          	: width of line for tapped secondary if wts<=0 no tapping
	// # geometry		: 0-> octagonal, 1 -> rectangular
	// # topM			: top metal layer (metal name)
	// ###############################################################################################

	int genCell(Cell &result, double dp, double ds, double wp, double ws, double ss, double sp, double lp, double wtp, double wts, int geometry, string topM, bool fasterCap = false);
	// ###############################################################################################
	// # transformer1o2
	// # 		: generate cell
	// ###############################################################################################
	// # dp 			: outer diameter of primary
	// # ds 			: outer diameter of secondary
	// # wp				: line width of primary
	// # ws				: secondary line width
	// # ss 			: shift between primary and secondary
	// # sp 			: space between pins
	// # lp           	: pin length (default should be 2w)
	// # wtp          	: width of line for tapped primary if wtp<=0 no tapping
	// # wts          	: width of line for tapped secondary if wts<=0 no tapping
	// # geometry		: 0-> octagonal, 1 -> rectangular
	// # topM			: top metal layer (metal name)
	// ###############################################################################################

	static void printHelp();
	// ###############################################################################################
	// # transformer1o12
	// #	: print command line options for transformer1o2
	// #	this function is static so it can be called without any object Transformer1o2::printHelp
	// ###############################################################################################

	int process(Command *cm);
	// ###############################################################################################
	// # transformer1o2
	// #		: process command line options
	// ###############################################################################################

	int FastHenryGenCell(FastHenry *fh, double dp, double ds, double wp, double ws, double ss, double sp, double lp, double wtp, double wts, int geometry, string topM, bool fasterCap);
	// ###############################################################################################
	// # transformer1o2
	// # 		: generate cell
	// ###############################################################################################
	// # dp 			: outer diameter of primary
	// # ds 			: outer diameter of secondary
	// # wp				: line width of primary
	// # ws				: secondary line width
	// # ss 			: shift between primary and secondary
	// # sp 			: space between pins
	// # lp           	: pin length (default should be 2w)
	// # wtp          	: width of line for tapped primary if wtp<=0 no tapping
	// # wts          	: width of line for tapped secondary if wts<=0 no tapping
	// # geometry		: 0-> octagonal, 1 -> rectangular
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