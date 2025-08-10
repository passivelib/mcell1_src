// gdsTech.h

#ifndef _INDUCTORSYMMETRIC_H_
#define _INDUCTORSYMMETRIC_H_

#include <string>
#include <vector>
#include <string>
#include "mcell_library.h"
#include "mcell_fast_henry.h"

#include "mcell_command_line.h"
#include <math.h>
#include <mutex>
#include "BS_thread_pool.hpp"

// using namespace std;

class InductorSymmetric
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

	~InductorSymmetric()
	{
		this->clear();
	}

	InductorSymmetric(const char *fileName) : l(NULL), fhl(NULL)
	{
		this->l = new Gdslib(fileName);
		this->fhl = new FastHenryLib(fileName);

		this->numTasksPerThread = 1028;
	}

	int drc(double d, double w, double s, double sp, double lp, int n, double wtp, int geometry, string topM);
	// ###############################################################################################
	// # symmetrical inductor
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

	int genCell(Cell &result, double d, double w, double s, double sp, double lp, int n, double wtp, int geometry, string topM, bool fasterCap = false);
	// ###############################################################################################
	// # symmetrical inductor
	// #		: generate cell for gds file
	// ###############################################################################################
	// # d 				: outer diameter
	// # w				: line width
	// # s 				: space between lines
	// # sp 			: space between pins
	// # lp           	: pin length (default should be 2w)
	// # n 				: number of turns
	// # wtp          : width of line for tapped inductor if wtp<=0 no tapping
	// # geometry		: 0-> octagonal, 1-> rectangular
	// # topM			: top metal layer (metal name)
	// # fasterCap		: false-> standard gds is generated
	// #				: true -> line of symmetry is removed from gds
	// ###############################################################################################

	static void printHelp();
	// ###############################################################################################
	// # symmetrical inductor
	// #		: print command line options for symind
	// #			this function is static so it can be called without any object SymInd::printHelp
	// ###############################################################################################

	int process(Command *cm);
	// ###############################################################################################
	// # symmetrical inductor
	// #		: process command line options
	// ###############################################################################################

	int FastHenryGenCell(FastHenry *fh, double d, double w, double s, double sp, double lp, int n, double wtp, int geometry, string topM, bool FastHenry2);
	// ###############################################################################################
	// # symmetrical inductor
	// #		: generate cell for gds file
	// ###############################################################################################
	// # d 				: outer diameter
	// # w				: line width
	// # s 				: space between lines
	// # sp 			: space between pins
	// # lp           	: pin length (default should be 2w)
	// # n 				: number of turns
	// # wtp          : width of line for tapped inductor if wtp<=0 no tapping
	// # geometry		: 0-> octagonal, 1-> rectangular
	// # topM			: top metal layer (metal name)
	// # fastHenry2		: false: one port, true: two ports (one more at symmetry point)
	// ###############################################################################################

	int FastHenryProcess(Command *cm);
	// ###############################################################################################
	// # symmetrical inductor
	// #		: process command line options
	// ###############################################################################################

	int FastHenry2Process(Command *cm);
	// ###############################################################################################
	// # symmetrical inductor
	// #		: process command line options
	// ###############################################################################################

	int threadNumber(Command *cm);
	// ###############################################################################################
	// # calculate number of threads to be used in thread pool
	// ###############################################################################################
};

#endif