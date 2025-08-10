// gdsTech.h

#ifndef _TRANSFORMERSPIRAL_H_
#define _TRANSFORMERSPIRAL_H_

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

class TransformerSpiral
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

	~TransformerSpiral()
	{
		this->clear();
	}

	TransformerSpiral(const char *fileName) : l(NULL), fhl(NULL)
	{
		this->l = new Gdslib(fileName);
		this->fhl = new FastHenryLib(fileName);
		this->numTasksPerThread = 1028;
	}

	int drc(double d, double w, double s, double sp, double lp, int np, int ns, double wtp, double wts, int geometry, string topM);
	// ###############################################################################################
	// # transformer spiral
	// # 		: check if geometry is feasible
	// ###############################################################################################
	// # d 				: outer diameter
	// # w				: line width
	// # s 				: space between lines
	// # sp 			: space between pins
	// # lp           	: pin length (default should be 2w)
	// # n 				: number of turns
	// # wtp          : width of line for tapped inductor if wtp<=0 no tapping
	// # geometry		: 0-> octagonal, else rectangular
	// # topM			: top metal layer (metal name)
	// ###############################################################################################

	int genCell(Cell &result, double d, double w, double s, double sp, double lp, int np, int ns, double wtp, double wts, int geometry, string topM, bool fasterCap = false);
	// ###############################################################################################
	// # transformer spiral
	// #		: generate cell for gds file
	// ###############################################################################################
	// # d 				: outer diameter
	// # w				: line width
	// # s 				: space between lines
	// # sp 			: space between pins
	// # lp           	: pin length (default should be 2w)
	// # n 				: number of turns
	// # wtp          : width of line for tapped inductor if wtp<=0 no tapping
	// # geometry		: 0-> octagonal, else rectangular
	// # topM			: top metal layer (metal name)
	// # typ			: 0-> standard pins on left side below bottom metal layer might be used for tapping
	// #				: 1-> pins are at bottom side, only bottom layer is used for tapping
	// #					symetry is poor tapped pin is not in the midle.
	// #					Distance from Plut to tapped and from Minus to tapped is not identical
	// ###############################################################################################

	static void printHelp();
	// ###############################################################################################
	// # transformer spiral
	// #		: print command line options
	// #			this function is static so it can be called without any object SymInd::printHelp
	// ###############################################################################################

	int process(Command *cm);
	// ###############################################################################################
	// # transformer spiral
	// #		: process command line options
	// ###############################################################################################

	int FastHenryGenCell(FastHenry *fh, double d, double w, double s, double sp, double lp, int np, int ns, double wtp, double wts, int geometry, string topM, bool fasterCap);
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