// gdsTech.h

#ifndef _GDSTECH_H_
#define _GDSTECH_H_

#include <string>
#include <vector>

using namespace std;

struct LayerMet
{

	// Metal elements
	string name = "";
	int gdsNum = 0;
	int gdsType = 0;
	double minS = 0;
	double minW = 0;
	// distance from substrate
	double dSub = 0;
	// metal thickness
	double metT = 0;
	// metal number: first metal after substrate has number 1
	// this option is not specified in file but it is
	// calculated from dSub
	int metNum = 0;
	// conductivity S/m
	double cond = 0;
};

struct LayerVia
{

	string name = "";
	int gdsNum = 0;
	int gdsType = 0;
	string topMet = "";
	string botMet = "";
	double viaEnc = 0;
	double viaSize = 0;
	double viaSpace = 0;
	// conductivity MS/m
	double cond = 0;
};

struct LayerDielectric
{

	string name = "";
	// distance from substrate
	double dSub = 0;
	double er = 0;
	// conductivity
	double ei = 0;

	// identification number
	uint64_t id;
};

struct LayerSub
{

	std::string name = "";
	double dSub;
	double er;
	// conductivity
	double cond;
	double freq;
	double ei;
};

class Tech
{

public:
	vector<LayerMet> metal;
	vector<LayerVia> via;
	vector<LayerDielectric> dielectric;
	LayerSub substrate;
	double grid;

	Tech() {}

	Tech(const char *fileName)
	{

		// this->readFile(fileName);
		this->readYamlFile(fileName);
		this->sortMet();
		// this->sortDielectric();
		this->checkTech();
	}
	~Tech()
	{
		clear();
	}

	int clear()
	{

		if (metal.size() > 0)
		{
			metal.clear();
		}
		if (via.size() > 0)
		{
			via.clear();
		}
		if (dielectric.size() > 0)
		{
			dielectric.clear();
		}
		return 0;
	}

	vector<char *> splitStr(char *str, char *delimiter);

	int readFile(const char *fileName);
	void readYamlFile(const std::string &fileName);

	LayerVia getVia(string ttopMet, string bbotMet);

	LayerMet getMet(string nname);
	LayerMet getMet(int metNum);
	int getMet(LayerMet &mmet, int ggdsNum, int ggdsType);
	// return 0 if metal is found and 1 if not
	// LayerMet : information about metal layer
	int getVia(LayerVia &vvia, int ggdsNum, int ggdsType);
	// return 0 if via is found and 1 if not
	// LayerVia : information about via layer

	string getMetName(int metNum);
	string getMetNamePP(string nname); // get metal name ++, get metal one level above
	string getMetNameMM(string nname); // get metal name --, get metal one level above

	double getGrid();

	int checkTech(); // checking if technology file is valid

	int sortMet(); // sort metal leyers from bottom to top

	int sortDielectric(); // sort dielectric leyers from bottom to top

	bool isEqual(double x, double y);
};

#endif