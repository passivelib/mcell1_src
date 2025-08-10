// gdsTech.h

#ifndef _TRANSFORMER_1O1_H_
#define _TRANSFORMER_1O1_H_

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

class Transformer1o1
{

public:
	Gdslib *l;
	FastHenryLib *fhl;

	size_t numTasksPerThread; // max number of tasks per one thread

	void clear()
	{
		// this->l->clear();
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

	~Transformer1o1()
	{
		this->clear();
	}

	Transformer1o1(const char *fileName) : l(NULL), fhl(NULL)
	{
		this->l = new Gdslib(fileName);
		this->fhl = new FastHenryLib(fileName);
		this->numTasksPerThread = 1028;
	}

	int drc(double dp, double ds, double wp, double ws, double sp, double lp, double wtp, double wts, int geometry, string topM);
	// ###############################################################################################
	// # transformer1o1
	// # 		: check if geometry is feasible
	// ###############################################################################################
	// # dp 			: outer diameter of primary
	// # ds 			: outer diameter of secondary
	// # wp				: line width of primary
	// # ws				: line width of secondary
	// # sp 			: space between pins
	// # lp           	: pin length (default should be 2w)
	// # wtp          	: width of line for tapped primary if wtp<=0 no tapping
	// # wts          	: width of line for tapped secondary if wts<=0 no tapping
	// # geometry		: 0-> octagonal, else rectangular
	// # topM			: top metal layer (metal name)
	// ###############################################################################################

	int genCell(Cell &result, double dp, double ds, double wp, double ws, double sp, double lp, double wtp, double wts, int geometry, string topM, bool fasterCap = false);
	// ###############################################################################################
	// # transformer1o1
	// # 		: generate cell
	// ###############################################################################################
	// # dp 			: outer diameter of primary
	// # ds 			: outer diameter of secondary
	// # wp				: line width of primary
	// # ws				: line width of secondary
	// # sp 			: space between pins
	// # lp           	: pin length (default should be 2w)
	// # wtp          	: width of line for tapped primary if wtp<=0 no tapping
	// # wts          	: width of line for tapped secondary if wts<=0 no tapping
	// # geometry		: 0-> octagonal, else rectangular
	// # topM			: top metal layer (metal name)
	// ###############################################################################################

	static void printHelp();
	// ###############################################################################################
	// # transformer1o1
	// #	: print command line options for transformer1o1
	// #	this function is static so it can be called without any object Transformer1o1::printHelp
	// ###############################################################################################

	int process(Command *cm);
	// ###############################################################################################
	// # transformer1o1
	// #		: process command line options
	// ###############################################################################################

	int FastHenryGenCell(FastHenry *fh, double dp, double ds, double wp, double ws, double sp, double lp, double wtp, double wts, int geometry, string topM, bool fastHenry2);
	// ###############################################################################################
	// # transformer1o1
	// # 		: generate cell
	// ###############################################################################################
	// # dp 			: outer diameter of primary
	// # ds 			: outer diameter of secondary
	// # wp				: line width of primary
	// # ws				: line width of secondary
	// # sp 			: space between pins
	// # lp           	: pin length (default should be 2w)
	// # wtp          	: width of line for tapped primary if wtp<=0 no tapping
	// # wts          	: width of line for tapped secondary if wts<=0 no tapping
	// # geometry		: 0-> octagonal, else rectangular
	// # topM			: top metal layer (metal name)
	// # fastHenry2		: true: separate on symmetry point
	// ###############################################################################################

	int FastHenryProcess(Command *cm);
	// ###############################################################################################
	// # transformer1o1
	// #		: process command line options
	// ###############################################################################################

	int FastHenry2Process(Command *cm);
	// ###############################################################################################
	// # transformer1o1
	// #		: process command line options
	// ###############################################################################################

	int threadNumber(Command *cm);
	// ###############################################################################################
	// # calculate number of threads to be used in thread pool
	// ###############################################################################################
};

#endif