#ifndef _COMPONENT_H_
#define _COMPONENT_H_

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

class Component
{
public:
    Gdslib *l;
    FastHenryLib *fhl;
    std::string name;

    size_t numTasksPerThread; // max number of tasks per one thread

    void clear();

    virtual ~Component()
    {
        this->clear();
    }

    Component(const char *fileName)
    {
        this->l = new Gdslib(fileName);
        this->fhl = new FastHenryLib(fileName);
        name = "component";
        this->numTasksPerThread = 1028;
    }

    virtual int process(Command *cm) = 0;
    // ###############################################################################################
    // # Component
    // #		: process command line options
    // ###############################################################################################

    virtual int FastHenryProcess(Command *cm) = 0;
    // ###############################################################################################
    // # Component
    // #		: fast henry process command line options
    // ###############################################################################################

    int threadNumber(Command *cm);
    // ###############################################################################################
    // # calculate number of threads to be used in thread pool
    // ###############################################################################################

    void makeFolder(Command *cm, const std::string &folderName);
    // ###############################################################################################
    // # make a folder folderName
    // ###############################################################################################

    void setName(Command *cm);
    // ###############################################################################################
    // # set gdsFileName, cellName, this->name
    // ###############################################################################################

    virtual std::string getEmxOptions(Command *cm) = 0;
    // ###############################################################################################
    // # get all emx options including ports name
    // ###############################################################################################

    virtual void initMaestroFile(ofstream &maestroFile, Command *cm) = 0;
    // ###############################################################################################
    // # open and write to maestro file for initialization
    // ###############################################################################################

    void makeModelGenSh(Command *cm);
    // ###############################################################################################
    // # make modelgen shell script
    // ###############################################################################################

    virtual void ifSweep(Command *cm) = 0;
    // ###############################################################################################
    // # check if any variable is swept, if yes set flag cm->generateEm = 1
    // ###############################################################################################

    virtual void roundParameters(Command *cm) = 0;
    // ###############################################################################################
    // # round all parameters and set default values if not set
    // ###############################################################################################

    void makeGdsYInitEmxMaestro(ofstream &runEmxShFile, ofstream &maestroFile, Command *cm);
    // ###############################################################################################
    // # make gsd and y folders if needed and init funEmxShFile and maestroFile if needed
    // ###############################################################################################

    virtual void verbose(Command *cm, const std::string &message, double parameters[]) = 0;
    // ###############################################################################################
    // # write message to std::cout
    // ###############################################################################################

    void patternedGroundShield(Cell &cell, Command *cm, const double &dmax);
    // ###############################################################################################
    // # generate pattern ground shield if needed, dmax is bigger diameter
    // ###############################################################################################

    virtual std::string getGdsFileName(Command *cm, double parameters[]) = 0;
    // ###############################################################################################
    // # gds file name for modelgen, emx sweep or no sweep
    // ###############################################################################################

    void writeEmxShFile(ofstream &runEmxShFile, Command *cm, const std::string &gdsFileName, const std::string &emxOptions);
    // ###############################################################################################
    // # write emx shell script file
    // ###############################################################################################

    void writeMaestroFile(ofstream &maestroFile, Command *cm, const std::string &gdsFileName);
    // ###############################################################################################
    // # write to maestro file
    // ###############################################################################################

    void check(double &d, double &w, Command *cm);
    // ###############################################################################################
    // # checking license
    // ###############################################################################################

    void merge(Cell &cell, Command *cm);
    // ###############################################################################################
    // # merge if required.
    // ###############################################################################################

    void writeGdsFile(Library *pointerLib, Command *cm, std::string gdsFileName);
    // ###############################################################################################
    // # write to gdsFile
    // ###############################################################################################

    void setFullPath(Command *cm);
    // ###############################################################################################
    // # set full path ./ is replaced with /home/project
    // ###############################################################################################

    void makeFastHenryFolders(Command *cm);
    // ###############################################################################################
    // # make folders fastHenry and zFile or fastHenry2 and zFile2
    // ###############################################################################################

    void openFastHenryShFile(ofstream &runFastHenryShFile, Command *cm);
    // ###############################################################################################
    // # open fast henry or fast henry2 file
    // ###############################################################################################

    void writeFastHenryShFile(ofstream &runFastHenryShFile, Command *cm, const std::string &gdsFileName);
    // ###############################################################################################
    // # write fastHenry and fastHenry2 shell script file
    // ###############################################################################################

    void writeFastHenryMeshFile(FastHenry *fh, Command *cm, std::string gdsFileName);
    // ###############################################################################################
    // # write to fast henry and fast henry2 mesh input file
    // ###############################################################################################
};

#endif