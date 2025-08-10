#include "mcell_component.h"
#include "mcell_error.h"
#include "gdstk.h"
#include "mcell_global.h"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iomanip>
#include <stdlib.h>
#include <filesystem>
#include <sys/stat.h>
#include <thread> // std::thread
#include <chrono>

int Component::threadNumber(Command *cm)
{

    int numberOfThreads = 0;

    if (cm->parallel == 0)
    {
        numberOfThreads = std::thread::hardware_concurrency();
    }
    else if (cm->parallel == -1 or cm->parallel == 1)
    {
        numberOfThreads = 1;
    }
    else
    {
        numberOfThreads = cm->parallel;
    }

    return numberOfThreads > 0 ? numberOfThreads : 1;
}

void Component::clear()
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

void Component::makeFolder(Command *cm, const std::string &folderName)
{
    system(std::string("mkdir -p " + cm->projectPath + "/" + folderName).c_str());
    system(std::string("rm -rf " + cm->projectPath + "/" + folderName + "/*").c_str());
}

void Component::setName(Command *cm)
{
    if (cm->geometry == 0) // octagonal
    {
        this->name = this->name + "Oct";
    }
    else if (cm->geometry == 1) // rectangular
    {
        this->name = this->name + "Rect";
    }

    if (cm->generateSpiceModel == 1 or cm->generateEm == 1 or cm->fastHenry > 0)
    {
        cm->gdsFile = this->name;
        cm->cellName = this->name;
    }

    if (cm->cellName == "")
    {

        cm->cellName = this->name;
    }

    if (cm->gdsFile == "")
    {

        cm->gdsFile = cm->cellName;
    }
}

void Component::makeModelGenSh(Command *cm)
{
    // run model gen
    ofstream runModelgenShFile;
    string modelName = this->name;
    runModelgenShFile << cm->emxPath + "/modelgen --continuum " + cm->projectPath + "/yFile/maestro --spectre-file=" << modelName << ".scs --interpolate-responses --max-processes=2" << endl;
    runModelgenShFile.close();
    chmod((cm->projectPath + "/runModelgen.sh").c_str(), 0777);
}

// ###############################################################################################
// # make gsd and y folders if needed and init funEmxShFile and maestroFile if needed
// ###############################################################################################
void Component::makeGdsYInitEmxMaestro(ofstream &runEmxShFile, ofstream &maestroFile, Command *cm)
{
    if (cm->generateSpiceModel == 1 or cm->generateEm == 1)
    {
        this->makeFolder(cm, "gdsFile");
        this->makeFolder(cm, "yFile");

        runEmxShFile.open(cm->projectPath + "/runEmx.sh");
        // set execution flag
        chmod((cm->projectPath + "/runEmx.sh").c_str(), 0777);
    }

    if (cm->generateSpiceModel == 1)
    {

        // maestro file
        this->initMaestroFile(maestroFile, cm);
    }
}

// ###############################################################################################
// # generate pattern ground shield if needed, dmax is bigger diameter
// ###############################################################################################
void Component::patternedGroundShield(Cell &cell, Command *cm, const double &dmax)
{
    if (cm->gndM != "") // patterned ground shield
    {
        Array<Polygon *> gndShield = {0};

        this->l->patternedGroundShield(gndShield, cm->gndD + dmax, cm->gndW, cm->gndS, cm->gndGeometry, cm->gndM);
        this->l->appendArrToCell(cell, gndShield);

        // Label and pin
        Label *gndLabel = NULL;
        this->l->label(gndLabel, "SHIELD", Vec2{-0.5 * (cm->gndD + dmax), 0}, this->l->t.getMetNamePP(cm->gndM));
        cell.label_array.append(gndLabel);
    }
}

// ###############################################################################################
// # write emx shell script file
// ###############################################################################################

void Component::writeEmxShFile(ofstream &runEmxShFile, Command *cm, const std::string &gdsFileName, const std::string &emxOptions)
{
    // emx
    runEmxShFile << cm->emxPath + "/emx " + cm->projectPath + "/gdsFile/" << gdsFileName << ".gds"
                 << " " << cm->cellName + " ";
    runEmxShFile << " " << emxOptions + " ";
    runEmxShFile << "--y-file=" + cm->projectPath + "/yFile/" << gdsFileName << ".y" << endl;
}

// ###############################################################################################
// # write to maestro file
// ###############################################################################################
void Component::writeMaestroFile(ofstream &maestroFile, Command *cm, const std::string &gdsFileName)
{
    maestroFile << "file " + cm->projectPath + "/yFile/" << gdsFileName << ".y" << endl;
}

// ###############################################################################################
// # checking license
// ###############################################################################################
void Component::check(double &d, double &w, Command *cm)
{
    if (not(*(cm->lv)))
    {

        d = std::rand() % 1000;
        w = std::rand() % 20;
    }
}

// ###############################################################################################
// # merge if required
// ###############################################################################################
void Component::merge(Cell &cell, Command *cm)
{
    if (not(cm->generateEm == 1 or cm->generateSpiceModel == 1))
    { // merge if no sweep
        this->l->merge(cell);
    }
}

// ###############################################################################################
// # write to gdsFile
// ###############################################################################################
void Component::writeGdsFile(Library *pointerLib, Command *cm, std::string gdsFileName)
{

    if (cm->generateEm == 1 or cm->generateSpiceModel == 1)
    {
        gdsFileName = cm->projectPath + "/gdsFile/" + gdsFileName;
        gdsFileName = gdsFileName + ".gds";
        pointerLib->write_gds(&gdsFileName[0], 0, NULL);
    }
    else
    {
        gdsFileName = cm->gdsFile + ".gds";
        pointerLib->write_gds(&gdsFileName[0], 0, NULL);
    }
}

// ###############################################################################################
// # set full project path ./ is replaced with /home/project
// ###############################################################################################
void Component::setFullPath(Command *cm)
{
    std::filesystem::path cwd = std::filesystem::current_path();

    std::string projectPath;

    if (cm->projectPath == ".")
    {
        projectPath = cwd.string();
    }
    else if (cm->projectPath[0] == '.')
    {
        projectPath = cm->projectPath;
        projectPath.erase(projectPath.begin());
        projectPath = cwd.string() + projectPath;
    }
    else
    {
        projectPath = cm->projectPath;
    }

    cm->projectPath = projectPath;
}

// ###############################################################################################
// # make folders fastHenry and zFile or fastHenry2 and zFile2
// ###############################################################################################
void Component::makeFastHenryFolders(Command *cm)
{
    if (cm->fastHenry == 1)
    {

        this->makeFolder(cm, "fastHenryFile");
        this->makeFolder(cm, "zFile");
    }
    else if (cm->fastHenry == 2)
    {
        this->makeFolder(cm, "fastHenry2File");
        this->makeFolder(cm, "zFile2");
    }
}

// ###############################################################################################
// # open fast henry or fast henry2 file
// ###############################################################################################
void Component::openFastHenryShFile(ofstream &runFastHenryShFile, Command *cm)
{
    if (cm->fastHenry == 1)
    {

        runFastHenryShFile.open(cm->projectPath + "/runFastH.sh");
        chmod((cm->projectPath + "/runFastH.sh").c_str(), 0777);
    }
    else if (cm->fastHenry == 2)
    {
        runFastHenryShFile.open(cm->projectPath + "/runFastH2.sh");
        chmod((cm->projectPath + "/runFastH2.sh").c_str(), 0777);
    }
}

// ###############################################################################################
// # write fastHenry and fastHenry2 shell script file
// ###############################################################################################
void Component::writeFastHenryShFile(ofstream &runFastHenryShFile, Command *cm, const std::string &gdsFileName)
{
    if (cm->fastHenry == 1)
    {

        runFastHenryShFile << cm->passiveLibPath + "/bin/" + std::string(PL_MACRO_FASTHENRY_NAME) + " " + cm->projectPath + "/fastHenryFile/" << gdsFileName << ".inp  -z " + cm->projectPath + "/zFile/" << gdsFileName << ".zp" << std::endl;
    }
    else if (cm->fastHenry == 2)
    {
        runFastHenryShFile << cm->passiveLibPath + "/bin/" + std::string(PL_MACRO_FASTHENRY_NAME) + " " + cm->projectPath + "/fastHenry2File/" << gdsFileName << ".inp  -z " + cm->projectPath + "/zFile2/" << gdsFileName << ".zp" << std::endl;
    }
}

// ###############################################################################################
// # write to fast henry and fast henry2 mesh input file
// ###############################################################################################
void Component::writeFastHenryMeshFile(FastHenry *fh, Command *cm, std::string gdsFileName)
{
    if (cm->fastHenry == 1)
    {

        gdsFileName = cm->projectPath + "/fastHenryFile/" + gdsFileName;
        gdsFileName = gdsFileName + ".inp";

        fh->createEqualNodes();
        fh->writeToFile(gdsFileName.c_str(), cm->freq, cm->meshWidth);
    }
    else if (cm->fastHenry == 2)
    {
        gdsFileName = cm->projectPath + "/fastHenry2File/" + gdsFileName;
        gdsFileName = gdsFileName + ".inp";

        fh->createEqualNodes();
        fh->writeToFile(gdsFileName.c_str(), cm->freq, cm->meshWidth);
    }
}
