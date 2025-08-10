// gdsTech.h

#ifndef _INDUCTORSPIRAL_H_
#define _INDUCTORSPIRAL_H_

#include <string>
#include <vector>
#include <string>

#include "mcell_library.h"
#include "gdstk.h"
#include "mcell_command_line.h"
#include "mcell_fast_henry.h"

#include <math.h>
#include <mutex> // std::mutex
#include "BS_thread_pool.hpp"
#include "mcell_component.h"

class InductorSpiral : public Component
{

public:
	~InductorSpiral()
	{
		this->Component::clear();
	}

	InductorSpiral(const char *fileName) : Component(fileName)
	{
		this->name = "plIndSpir";
	}

	int drc(double d, double w, double wu, double s, double sp, double lp, double n, int geometry, string topM);
	// ###############################################################################################
	// # common mode inductor
	// # 		: check if geometry is feasible
	// ###############################################################################################
	// # d 				: outer diameter
	// # w				: line width
	// # wu      		: line width for underpass layer
	// # s 				: space between lines
	// # sp 			: space between pins
	// # lp           	: pin length (default should be 2w)
	// # n 				: number of turns
	// # wtp          : width of line for tapped inductor if wtp<=0 no tapping
	// # geometry		: 0-> octagonal, else rectangular
	// # topM			: top metal layer (metal name)
	// ###############################################################################################

	int genCell(Cell &result, double d, double w, double wu, double s, double sp, double lp, double n, int geometry, string topM);
	// ###############################################################################################
	// # common mode inductor
	// #		: generate cell for gds file
	// ###############################################################################################
	// # d 				: outer diameter
	// # w				: line width
	// # wu      		: line width for underpass layer
	// # s 				: space between lines
	// # sp 			: space between pins
	// # lp           	: pin length (default should be 2w)
	// # n 				: number of turns
	// # geometry		: 0-> octagonal, else rectangular
	// # topM			: top metal layer (metal name)
	// ###############################################################################################

	static void printHelp();
	// ###############################################################################################
	// # spiral inductor
	// #		: print command line options for spiral
	// #			this function is static so it can be called without any object InductorSpiral::printHelp
	// ###############################################################################################

	int process(Command *cm);
	// ###############################################################################################
	// # spiral inductor
	// #		: process command line options
	// ###############################################################################################

	int FastHenryGenCell(FastHenry *fh, double d, double w, double wu, double s, double sp, double lp, double n, int geometry, string topM);
	// ###############################################################################################
	// # common mode inductor
	// #		: generate cell for fast henry file
	// ###############################################################################################
	// # d 				: outer diameter
	// # w				: line width
	// # wu      		: line width for underpass layer
	// # s 				: space between lines
	// # sp 			: space between pins
	// # lp           	: pin length (default should be 2w)
	// # n 				: number of turns
	// # geometry		: 0-> octagonal, else rectangular
	// # topM			: top metal layer (metal name)
	// ###############################################################################################

	int FastHenryProcess(Command *cm);
	// ###############################################################################################
	// # spiral inductor
	// #		: fast henry process command line options
	// ###############################################################################################

	std::string getEmxOptions(Command *cm);
	// ###############################################################################################
	// # get all emx options including ports name
	// ###############################################################################################

	void initMaestroFile(ofstream &maestroFile, Command *cm);
	// ###############################################################################################
	// # open and write to maestro file for initialization
	// ###############################################################################################

	void ifSweep(Command *cm);
	// ###############################################################################################
	// # check if any variable is swept, if yes set flag cm->generateEm = 1
	// ###############################################################################################

	void roundParameters(Command *cm);
	// ###############################################################################################
	// # round all parameters and set default values if not set
	// ###############################################################################################

	void verbose(Command *cm, const std::string &message, double parameters[]);
	// ###############################################################################################
	// # write message to std::cout
	// ###############################################################################################

	std::string getGdsFileName(Command *cm, double parameters[]);
	// ###############################################################################################
	// # gds file name for modelgen, emx sweep or no sweep
	// ###############################################################################################
};

#endif
