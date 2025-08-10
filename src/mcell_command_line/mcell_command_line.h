#ifndef __MCELL_COMMAND_LINE_H__
#define __MCELL_COMMAND_LINE_H__

#include "mcell_error.h"

#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <vector>
#include <sstream>

#include <getopt.h>

using namespace std;

class Range
{
public:
	double max;
	double min;
	double step;

	string errorMessage;

	void clear()
	{
		max = 0;
		min = 0;
		step = 1;
		errorMessage = "";
	}

	Range()
	{
		this->clear();
	}

	~Range()
	{
		this->clear();
	}

	int split(vector<string> &out, string str, const char delim)
	{
		// construct a stream from the string
		std::stringstream ss(str);

		std::string s;
		while (std::getline(ss, s, delim))
		{
			out.push_back(s);
		}

		return 0;
	}

	void read(string x)
	{
		vector<string> tmpStrArr;
		const char delim = ':';

		tmpStrArr.clear();
		this->split(tmpStrArr, x, delim);

		if (tmpStrArr.size() == 1)
		{
			this->max = std::stod(x);
			this->min = this->max;
			this->step = 1;
		}
		else if (tmpStrArr.size() == 3)
		{
			this->min = std::stod(tmpStrArr[0]);
			this->max = std::stod(tmpStrArr[1]);
			this->step = std::stod(tmpStrArr[2]);

			if (this->step <= 0)
			{
				this->step = 1;
			}
		}
		else
		{
			throw HardError(this->errorMessage);
		}

		tmpStrArr.clear();
	}

	size_t size()
	{
		return std::ceil(std::abs(this->max - this->min) / this->step);
	}
};

class Command
{

public:
	// # symmetrical inductor
	// type of components:
	//		symind
	string type;

	// diameter
	Range d;

	// line width
	Range w;

	// underpass Width
	double wUnd;

	// line space
	Range s;

	// number of turns
	Range n;

	// number of primary turns
	Range np;

	// number of secondary turns
	Range ns;

	// space between pins
	double ps;

	// pin length
	double pl;

	// geometry
	//	0-> octagonal, 1-> rectangular
	int geometry;

	// top metal
	string topM;

	// cell name
	string cellName;

	// gds file name
	string gdsFile;

	// technology file
	string techFile;

	// patterned Ground Shield metal name
	string gndM;

	// patterned Ground Shield diameter but relative
	// real diameter is gndD+r of inductor
	double gndD;

	// patterned Ground Shield metal width
	double gndW;

	// patterned Ground Shield hole width
	double gndS;

	// patterned Ground Shield geometry
	//	0-> octagonal, else rectangular
	//   default 1, rectangular
	int gndGeometry;

	// report file name
	string reportFile;

	// prepear files for spice model generation
	int generateSpiceModel;

	// prepear files for em simulation if 1
	int generateEm;

	// prepear files for Fast Henry if 1
	int fastHenry;

	// tapped width
	//  if specified without value, value is set to -1 and default value is 2*w
	//  if tapping is not specified, value is set to 0 and tapping will not be done
	//  if value is >0, that value is used for tapping
	double tappedWidth;

	// tapped primary/secondary width
	//  if specified without value, value is set to -1 and default value is 2*w
	//  if tapping is not specified, value is set to 0 and tapping will not be done
	//  if value is >0, that value is used for tapping
	double tappedPrimaryWidth;
	double tappedSecondaryWidth;

	// space secondary
	Range ss;

	// secondary shift
	Range sh;

	// secondary width
	Range ws;

	// license file
	std::string licenseFile;

	// generated host id for licensing
	//  if true generate file with host id information
	bool hostId;

	// path to emx
	// loaded from environment variable PASSIVE_LIB_EMPATH
	std::string emxPath;

	// path to PassiveLib
	// loaded from environment variable PASSIVE_LIB_PATH
	std::string passiveLibPath;

	// emx options
	// loaded from environment variable PASSIVE_LIB_EMOPTIONS
	std::string emxOptions;

	// path to be used for creation of gdsFile and yFile directories
	std::string projectPath;

	// license valid
	bool *lv;

	// frequency range for FastHenry
	Range freq;

	// mesh width
	double meshWidth;

	// software version
	double version;

	// print version
	int printVersion;

	// number of threads to be used
	int parallel;

	Command()
	{

		this->clear();
		// license valid
		this->lv = NULL;
	}

	~Command()
	{
		this->clear();
	}

	void clear()
	{
		this->type = "";

		this->d.clear();

		this->w.clear();

		this->wUnd = 0;

		this->s.clear();

		this->n.clear();

		this->ps = 0;
		this->pl = 0;

		this->geometry = 0;

		this->topM = "";

		this->cellName = "";

		this->gdsFile = "";

		this->techFile = "";

		this->gndM = "";

		this->gndW = 0;

		this->gndS = 0;

		this->gndD = 0;

		this->gndGeometry = 1;

		this->reportFile = "";

		this->generateSpiceModel = 0;

		this->tappedWidth = 0;

		this->tappedSecondaryWidth = 0;

		this->tappedPrimaryWidth = 0;

		this->np.clear();

		this->ns.clear();

		this->ss.clear();

		this->sh.clear();

		this->ws.clear();

		this->licenseFile = "";

		this->hostId = false;

		this->emxOptions = "";

		this->emxPath = "";

		this->passiveLibPath = "";

		this->generateEm = 0;

		this->fastHenry = 0;

		this->projectPath = ".";

		this->freq.clear();

		this->meshWidth = -1;

		this->version = -1;

		this->printVersion = -1;

		this->parallel = -1;
	}

	void print();
	int split(vector<string> &out, string str, const char delim);
	void printHelp();
	void printCommonHelp();
	void printCommonIndTrHelp();
	int processArgs(int argc, char **argv);
	// process particular cells

	// process all cells. Call in main
	int process();
	bool loadTechFromEnVariable(std::string envVarable);

	bool loadEmPathFromEnVariable(std::string envVarable);
	bool loadPassiveLibPathFromEnVariable(std::string envVarable);
	bool loadEmOptFromEnVariable(std::string envVarable);
};

#endif // __MCELL_COMMAND_LINE_H__