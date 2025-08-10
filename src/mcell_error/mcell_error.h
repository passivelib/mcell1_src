// gdsTech.h

#ifndef _GDSERROR_H_
#define _GDSERROR_H_

#include <string>

using namespace std;

class SoftError {

public:
	// Massage
	string name;
	
	SoftError(string nname)
	{
		this->name=nname;
	}

};


class HardError {

public:
	// Massage
	string name;

	HardError(string nname)
	{
		this->name=nname;
	}

};




#endif