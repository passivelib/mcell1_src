#include "mcell_command_line.h"
#include "mcell_error.h"

#include "mcell_inductor_spiral.h"
#include "mcell_inductor_symmetric.h"
#include "mcell_transformer1o1.h"
#include "mcell_transformer_spiral.h"
#include "mcell_transformer2o2.h"
#include "mcell_transformer1o2.h"
#include "mcell_transformer2o1.h"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <iostream>
#include <fstream>

#include <vector>
#include <sstream>

#include <getopt.h>

#include <stdlib.h>

using namespace std;

int Command::split(vector<string> &out, string str, const char delim)
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

bool Command::loadTechFromEnVariable(std::string envVarable)
{
    const char *val = std::getenv(envVarable.c_str());

    if (val == nullptr)
    { // invalid to assign nullptr to std::string

        return false;
    }
    else
    {

        this->techFile = std::string(val);
        return true;
    }
}

bool Command::loadEmPathFromEnVariable(std::string envVarable)
{
    const char *val = std::getenv(envVarable.c_str());

    if (val == nullptr)
    { // invalid to assign nullptr to std::string

        return false;
    }
    else
    {

        this->emxPath = std::string(val);
        return true;
    }
}

bool Command::loadPassiveLibPathFromEnVariable(std::string envVarable)
{
    const char *val = std::getenv(envVarable.c_str());

    if (val == nullptr)
    { // invalid to assign nullptr to std::string

        return false;
    }
    else
    {

        this->passiveLibPath = std::string(val);
        return true;
    }
}

bool Command::loadEmOptFromEnVariable(std::string envVarable)
{
    const char *val = std::getenv(envVarable.c_str());

    if (val == nullptr)
    { // invalid to assign nullptr to std::string

        return false;
    }
    else
    {

        this->emxOptions = std::string(val);
        return true;
    }
}

void Command::printHelp()
{
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--help <-h>:" << endl;
    std::cout << "       Display this file. Short option is -h." << endl;
    std::cout << "--help=component:" << endl;
    std::cout << "       Print options for given component." << endl;
    std::cout << "       Valid components are:" << endl;
    std::cout << "       inductor-symmetric inductor-spiral transformer-spiral transformer1o1" << endl;
    std::cout << "       transformer1o2 transformer2o1 transformer2o2" << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--host-id:" << endl;
    std::cout << "       Creates file hostid.txt with host id information." << endl;
    std::cout << "       Please send this file to us to get valid license." << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--version <-v>:" << endl;
    std::cout << "       Print software version. Short option is -v." << endl;
    std::cout << "********************************************************************************************" << endl;

    exit(1);
}

void Command::printCommonHelp()
{
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--type=name <-t>:" << endl;
    std::cout << "       Set component type. Short option is -t." << endl;
    std::cout << "       This option is mandatory." << endl;
    std::cout << "       Possible values: transformer-spiral, transformer1o1, inductor-spiral," << endl;
    std::cout << "                      : inductor-symmetric, transformer1o2, transformer2o1," << endl;
    std::cout << "                      : transformer2o2." << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--cell-name=name:" << endl;
    std::cout << "       Set top cell name in gds file." << endl;
    std::cout << "       This option is optional. If not set default value will be used." << endl;
    std::cout << "       --cell-name=inductor" << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--gds-file-name=name:" << endl;
    std::cout << "       Set gds file name." << endl;
    std::cout << "       This option is optional. If not set default value will be used." << endl;
    std::cout << "       --gds-file-name=ind.gds" << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--tech-file-name=name:" << endl;
    std::cout << "       Set technology file name." << endl;
    std::cout << "       In this file technology is described." << endl;
    std::cout << "       This option is mandatory." << endl;
    std::cout << "       It can be set with environment variable: MCELL_TECHNOLOGY." << endl;
    std::cout << "       --tech-file-name=tech.txt" << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--license-file-name=name <-l>:" << endl;
    std::cout << "       Set license file name. Short option is -l." << endl;
    std::cout << "       It can be set with environment variable: MCELL_LICENSE." << endl;
    std::cout << "       --license-file-name=license.lic or -l license.lic" << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--generate-spice-model:" << endl;
    std::cout << "       Prepare files for spice model generation." << endl;
    std::cout << "       Two folders are created gdsFiles and yFile." << endl;
    std::cout << "       All created gds files are stored in gdsFiles folder." << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--generate-em:" << endl;
    std::cout << "       Prepare files for em simulation." << endl;
    std::cout << "       Two folders are created gdsFiles and yFile." << endl;
    std::cout << "       All created gds files are stored in gdsFiles folder." << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--fast-henry:" << endl;
    std::cout << "       Prepare files for fastHenry simulation." << endl;
    std::cout << "       Two folders are created fastHenryFile and zFile." << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--project-path=name:" << endl;
    std::cout << "       Set path for gdsFile and yFile directories." << endl;
    std::cout << "       This option is optional. Default value is ./" << endl;
    std::cout << "       --project-path=/proj/inductor" << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--frequency=number <-f>:" << endl;
    std::cout << "       Set simulation frequency for FastHenry. Short option is -f." << endl;
    std::cout << "       This option is optional. Default is 1 Hz" << endl;
    std::cout << "       --frequency=100 or -f 100" << endl;
    std::cout << "--frequency=fmin:fmax:ndec <-f>:" << endl;
    std::cout << "       Frequency is swept from fmin to fmax with ndec frequencies per decade." << endl;
    std::cout << "       Short option is -f." << endl;
    std::cout << "       --frequency=1:1e6:10 or -f 1:1e6:10" << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--mesh-width=number:" << endl;
    std::cout << "       Set mesh size for fastHenry." << endl;
    std::cout << "       FastHenry:" << endl;
    std::cout << "       Mesh size should be in range of the skin depth." << endl;
    std::cout << "       nhinc=round(hight/mesh-width), nwinc=round(width/mesh-width)." << endl;
    std::cout << "       Rounding will be done to the odd number." << endl;
    std::cout << "       Definition of nwinc and nhinc can be found in the FastHenry documentation." << endl;
    std::cout << "       This option is optional. If not set default value will be used." << endl;
    std::cout << "       Default value is nhinc=1, nwinc=1." << endl;
    std::cout << "       --mesh-width=0.2" << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--parallel=number <-j>:" << endl;
    std::cout << "       Set number of threads to be used for the computation. Short option is -j" << endl;
    std::cout << "       This option is optional. Default value is 1." << endl;
    std::cout << "       --parallel=0, tool will choose optimal number of threads for the given hardware." << endl;
    std::cout << "       --parallel=4 or -j 4" << endl;
}

void Command::printCommonIndTrHelp()
{

    this->printCommonHelp();

    std::cout << "********************************************************************************************" << endl;
    std::cout << "--top-metal=name:" << endl;
    std::cout << "       Set metal in which inductor/transformer is built." << endl;
    std::cout << "       This option is mandatory." << endl;
    std::cout << "       --top-metal=M8" << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--rect-geometry:" << endl;
    std::cout << "       Set rectangular geometry for inductor." << endl;
    std::cout << "       This option is optional. If not set default value will be used." << endl;
    std::cout << "       Default is octagonal geometry." << endl;
    std::cout << "       --rect-geometry" << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--oct-geometry:" << endl;
    std::cout << "       Set octagonal geometry for inductor." << endl;
    std::cout << "       This option is optional. If not set default value will be used." << endl;
    std::cout << "       Default is octagonal geometry." << endl;
    std::cout << "       --oct-geometry" << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--gnd-shield-metal-name=name:" << endl;
    std::cout << "       Set metal in which ground shield is built." << endl;
    std::cout << "       This option is optional. If not set ground shield is not used." << endl;
    std::cout << "       --gnd-shield-metal-name=M1" << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--gnd-shield-diameter=number:" << endl;
    std::cout << "       Set diameter for ground shield." << endl;
    std::cout << "       This diameter is relative." << endl;
    std::cout << "       Absolute diameter is relative diameter + diameter of inductor/transformer." << endl;
    std::cout << "       This option is optional. If not set default value will be used." << endl;
    std::cout << "       Default value is 0." << endl;
    std::cout << "       --gnd-shield-diameter=20" << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--gnd-shield-finger-width=number:" << endl;
    std::cout << "       Set metal width for ground shield." << endl;
    std::cout << "       This option is optional. If not set default value will be used." << endl;
    std::cout << "       Default value is two times minimal metal width for used metal layer." << endl;
    std::cout << "       --gnd-shield-finger-width=1" << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--gnd-shield-finger-space=number:" << endl;
    std::cout << "       Set hole width for ground shield." << endl;
    std::cout << "       This option is optional. If not set default value will be used." << endl;
    std::cout << "       Default value is minimal metal space for used metal layer." << endl;
    std::cout << "       --gnd-shield-finger-space=0.2" << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--gnd-shield-rect-geometry:" << endl;
    std::cout << "       Set rectangular geometry for shield." << endl;
    std::cout << "       This option is optional. If not set default value will be used." << endl;
    std::cout << "       Default is rectangular geometry." << endl;
    std::cout << "       --gnd-shield-rect-geometry" << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--gnd-shield-oct-geometry:" << endl;
    std::cout << "       Set octagonal geometry for shield." << endl;
    std::cout << "       This option is optional. If not set default value will be used." << endl;
    std::cout << "       Default is rectangular geometry." << endl;
    std::cout << "       --gnd-shield-oct-geometry" << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--pin-space=number:" << endl;
    std::cout << "--ps=number:" << endl;
    std::cout << "       Set space between pins." << endl;
    std::cout << "       This option is optional. If not set default value will be used." << endl;
    std::cout << "       Default is 2*w." << endl;
    std::cout << "       --pin-space=10 or --ps=10" << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--pin-length=number:" << endl;
    std::cout << "--pl=number:" << endl;
    std::cout << "       Set length of pins." << endl;
    std::cout << "       This option is optional. If not set default value will be used." << endl;
    std::cout << "       Default is 2*w." << endl;
    std::cout << "       --pin-length=10 or --pl=10" << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--diameter=number <-d>:" << endl;
    std::cout << "       Set outer diameter of inductor/transformer. Short option is -d." << endl;
    std::cout << "       This option is mandatory." << endl;
    std::cout << "       --diameter=100 or -d 100" << endl;
    std::cout << "--diameter=dmin:dmax:dstep <-d>:" << endl;
    std::cout << "       Outer diameter of inductor/transformer is swept from dmin to dmax with step dstep." << endl;
    std::cout << "       Short option is -d." << endl;
    std::cout << "       --diameter=50:100:10 or -d 50:100:10" << endl;
    std::cout << "********************************************************************************************" << endl;
    std::cout << "--width=number <-w>:" << endl;
    std::cout << "       Set width of turns of inductor/transformer. Short option is -w." << endl;
    std::cout << "       This option is mandatory." << endl;
    std::cout << "       --width=5 or -w 5" << endl;
    std::cout << "--width=wmin:wmax:wstep <-w>:" << endl;
    std::cout << "       Width of turns of inductor/transformer is swept from wmin to wmax with step wstep." << endl;
    std::cout << "       Short option is -w." << endl;
    std::cout << "       --width=2:7:1 or -w 2:7:1" << endl;
}

int Command::processArgs(int argc, char **argv)
{
    int index = -1;
    // vector<string> tmpStrArr;
    // const char delim=':';
    //"vf::o:" refers to three options: an argumentless v, an optional-argument f, and a mandatory-argument o.
    const char *const short_opts = "h::d:w:s:n:t:r:f:l:vj:";
    const option long_opts[] = {
        {"help", optional_argument, nullptr, 'h'},
        {"diameter", required_argument, nullptr, 'd'},
        {"radius", required_argument, nullptr, 'r'},
        {"width", required_argument, nullptr, 'w'},
        {"space", required_argument, nullptr, 's'},
        {"number-of-turns", required_argument, nullptr, 'n'},
        {"type", required_argument, nullptr, 't'},
        {"license-file-name", required_argument, nullptr, 'l'},
        {"version", no_argument, nullptr, 'v'},
        {"parallel", required_argument, nullptr, 'j'},
        {"pin-space", required_argument, &index, 0},
        {"pin-length", required_argument, &index, 1},
        {"rect-geometry", no_argument, &index, 2},
        {"oct-geometry", no_argument, &index, 3},
        {"top-metal", required_argument, &index, 4},
        {"cell-name", required_argument, &index, 6},
        {"gds-file-name", required_argument, &index, 7},
        {"tech-file-name", required_argument, &index, 8},
        {"gnd-shield-metal-name", required_argument, &index, 9},
        {"gnd-shield-finger-width", required_argument, &index, 10},
        {"gnd-shield-finger-space", required_argument, &index, 11},
        {"gnd-shield-diameter", required_argument, &index, 12},
        {"gnd-shield-rect-geometry", no_argument, &index, 13},
        {"gnd-shield-oct-geometry", no_argument, &index, 14},
        {"ps", required_argument, &index, 15},
        {"pl", required_argument, &index, 16},
        {"report-file-name", required_argument, &index, 18},
        {"generate-spice-model", no_argument, &index, 19},
        {"tapped", optional_argument, &index, 20},
        {"underpass-metal-width", required_argument, &index, 21},
        {"tapped-primary", optional_argument, &index, 22},
        {"tapped-secondary", optional_argument, &index, 23},
        {"number-of-primary-turns", required_argument, &index, 24},
        {"np", required_argument, &index, 25},
        {"number-of-secondary-turns", required_argument, &index, 26},
        {"ns", required_argument, &index, 27},
        {"shift-secondary", required_argument, &index, 28},
        {"sh", required_argument, &index, 29},
        {"host-id", no_argument, &index, 30},
        {"width-secondary", required_argument, &index, 31},
        {"ws", required_argument, &index, 32},
        {"ss", required_argument, &index, 33},
        {"space-secondary", required_argument, &index, 34},
        {"generate-em", no_argument, &index, 35},
        {"project-path", required_argument, &index, 36},
        {"fast-henry", no_argument, &index, 37},
        {"frequency", required_argument, &index, 38},
        {"mesh-width", required_argument, &index, 39},
        {"fast-henry2", no_argument, &index, 42},
        {"wu", required_argument, &index, 43},
        {nullptr, no_argument, nullptr, 0}};

    while (true)
    {

        const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

        if (-1 == opt)
            break;

        // string optargStr(optarg); //argument that is passed to command
        string optargStr;
        // cout<<"processArgs"<<endl;

        switch (opt)
        {

        case 0: // only long options

            switch (index)
            {
            case 0: // pin space sp
                this->ps = std::stod(optarg);
                break;
            case 1: // pin length
                this->pl = std::stod(optarg);
                break;
            case 2: // rect geometry
                this->geometry = 1;
                break;
            case 3:
                this->geometry = 0;
                break;
            case 4:
                this->topM = optarg;
                break;
            case 6:
                this->cellName = optarg;
                break;
            case 7:
                optargStr = optarg;
                if (optargStr.find(".gds") != std::string::npos)
                {
                    this->gdsFile = optargStr.substr(0, optargStr.find(".gds"));
                }
                else
                {
                    this->gdsFile = optargStr;
                }

                break;
            case 8:
                this->techFile = optarg;
                break;
            case 9:
                this->gndM = optarg;
                break;
            case 10:
                this->gndW = std::stod(optarg);
                break;
            case 11:
                this->gndS = std::stod(optarg);
                break;
            case 12:
                this->gndD = std::stod(optarg);
                break;
            case 13: // rect geometry
                this->gndGeometry = 1;
                break;
            case 14: // oct geometry
                this->gndGeometry = 0;
                break;
            case 15: // pin space sp
                this->ps = std::stod(optarg);
                break;
            case 16: // pin length
                this->pl = std::stod(optarg);
                break;
            case 18:
                this->reportFile = optarg;
                break;
            case 19: // rect geometry
                this->generateSpiceModel = 1;
                break;
            case 20: // tapped
                if (!optarg)
                {
                    this->tappedWidth = -3;
                }
                else
                {
                    this->tappedWidth = std::stod(optarg);
                }
                break;

            case 21: // underpass width
                this->wUnd = std::stod(optarg);
                break;

            case 22: // tapped-primary
                if (!optarg)
                {
                    this->tappedPrimaryWidth = -3;
                }
                else
                {
                    this->tappedPrimaryWidth = std::stod(optarg);
                }
                break;

            case 23: // tapped-secondary
                if (!optarg)
                {
                    this->tappedSecondaryWidth = -3;
                }
                else
                {
                    this->tappedSecondaryWidth = std::stod(optarg);
                }
                break;

            case 24: //--number-of-primary-turns
                this->np.errorMessage = "Error: Command line error. Invalid --np or --number-of-primary-turns command!";
                this->np.read(string(optarg));

                break; //--number-of-primary-turns

            case 25: //--np
                this->np.errorMessage = "Error: Command line error. Invalid --np or --number-of-primary-turns command!";
                this->np.read(string(optarg));

                break; //-np

            case 26: //--number-of-secondary-turns
                this->ns.errorMessage = "Error: Command line error. Invalid --ns or --number-of-secondary-turns command!";
                this->ns.read(string(optarg));

                break; //--number-of-secondary-turns
            case 27:   //--ns
                this->ns.errorMessage = "Error: Command line error. Invalid --ns or --number-of-secondary-turns command!";
                this->ns.read(string(optarg));

                break; //--ns

            case 28: //--shift-secondary
                this->sh.errorMessage = "Error: Command line error. Invalid --sh or --shift-secondary command!";
                this->sh.read(string(optarg));

                break; //--shift-secondary

            case 29: //--sh
                this->sh.errorMessage = "Error: Command line error. Invalid --sh or --shift-secondary command!";
                this->sh.read(string(optarg));

                break; //--sh

            case 30: // host-id
                this->hostId = true;
                break;

            case 31: //--width-secondary
                this->ws.errorMessage = "Error: Command line error. Invalid --ws or --width-secondary!";
                this->ws.read(string(optarg));

                break; //--ws

            case 32: //--ws
                this->ws.errorMessage = "Error: Command line error. Invalid --ws or --width-secondary!";
                this->ws.read(string(optarg));

                break; //--ws

            case 33: //--ss
                this->ss.errorMessage = "Error: Command line error. Invalid --ss or --space-secondary!";
                this->ss.read(string(optarg));

                break; //--ss

            case 34: //--space-secondary
                this->ss.errorMessage = "Error: Command line error. Invalid --ss or --space-secondary!";
                this->ss.read(string(optarg));

                break; //--space-secondary

            case 35: // prepare for em simulation
                this->generateEm = 1;
                break;

            case 36: // prepare for project-path
                this->projectPath = std::string(optarg);
                break;

            case 37: // prepare fast henry files
                this->fastHenry = 1;
                break;

            case 38: // frequency
                this->freq.errorMessage = "Error: Command line error. Invalid -f or --frequency!";
                this->freq.read(string(optarg));
                break; // frequency

            case 39: // mesh-width
                this->meshWidth = std::stod(optarg);
                break; // frequency

            case 42: // prepare fast henry2 files
                this->fastHenry = 2;
                break;
            case 43: // underpass width
                this->wUnd = std::stod(optarg);
                break;

            default:
                break;

            } // switch

            break; // case 0:

        case 'd': // diameter, can have --diameter=start:stop:step
            this->d.errorMessage = "Error: Command line error. Invalid -d or --diameter command!";
            this->d.read(string(optarg));

            break; // d

        case 'r': // radius, can have --radius=start:stop:step

            this->d.errorMessage = "Error: Command line error. Invalid -r or --radius command!";
            this->d.read(string(optarg));

            this->d.max = 2 * this->d.max;
            this->d.min = 2 * this->d.min;
            this->d.step = 2 * this->d.step;

            if (this->d.max == this->d.min)
            {
                this->d.step = 1;
            }

            break; // d

        case 'w': // width, can have --width=start:stop:step
            this->w.errorMessage = "Error: Command line error. Invalid -w or --width command!";
            this->w.read(string(optarg));

            break; // w

        case 's': // space, can have --space=start:stop:step
            this->s.errorMessage = "Error: Command line error. Invalid -s or --space command!";
            this->s.read(string(optarg));

            break; // s

        case 'f': // frequency for FastHenry, can have -f start:stop:step
            this->freq.errorMessage = "Error: Command line error. Invalid -f or --frequency!";
            this->freq.read(string(optarg));
            break; // f

        case 'n': // number of turns, can have --number-of-turns=start:stop:step
            this->n.errorMessage = "Error: Command line error. Invalid -n or --number-of-turns command!";
            this->n.read(string(optarg));
            if (this->n.min < 1)
                this->n.min = 1;
            if (this->n.max < 1)
                this->n.max = 1;
            break; // n

        case 't': // type
            this->type = string(optarg);
            break;

        case 'l': // license file
            this->licenseFile = string(optarg);
            break;

        case 'v': // print version
            this->printVersion = 1;
            break;

        case 'j': // number of threads
            this->parallel = std::stoi(optarg) > -1 ? std::stoi(optarg) : 0;
            break;

        case 'h': // -h or --help
            if (!optarg)
            {
                this->printHelp();
            }
            else if (string(optarg) == "inductor-symmetric")
            {
                this->printCommonIndTrHelp();
                InductorSymmetric::printHelp();
                exit(1);
            }
            else if (string(optarg) == "inductor-spiral")
            {
                this->printCommonIndTrHelp();
                InductorSpiral::printHelp();
                exit(1);
            }
            else if (string(optarg) == "transformer-spiral")
            {
                this->printCommonIndTrHelp();
                TransformerSpiral::printHelp();
                exit(1);
            }
            else if (string(optarg) == "transformer1o1")
            {
                this->printCommonIndTrHelp();
                Transformer1o1::printHelp();
                exit(1);
            }
            else if (string(optarg) == "transformer2o2")
            {
                this->printCommonIndTrHelp();
                Transformer2o2::printHelp();
                exit(1);
            }
            else if (string(optarg) == "transformer1o2")
            {
                this->printCommonIndTrHelp();
                Transformer1o2::printHelp();
                exit(1);
            }
            else if (string(optarg) == "transformer2o1")
            {
                this->printCommonIndTrHelp();
                Transformer2o1::printHelp();
                exit(1);
            }
            else
            {
                this->printHelp();
            }
            break;
        case '?': // Unrecognized option
            cout << "Unrecognized option!" << endl;
            cout << "Type -h or --help to see available options." << endl;
            exit(1);
            break;
        default:
            cout << "Unrecognized option!" << endl;
            cout << "Type -h or --help to see available options." << endl;
            exit(1);
            break;

        } // switch

    } // while

    return 0;

} // ProcessArgs

int Command::process()
{
    try
    {
        // load tech file from environment variable
        if (this->techFile == "")
        {
            this->loadTechFromEnVariable("MCELL_TECHNOLOGY");
        }

        // load em path from environment variable

        this->loadEmPathFromEnVariable("MCELL_EMX_PATH");

        // load em option from environment variable

        this->loadEmOptFromEnVariable("MCELL_EMX_OPTIONS");

        // load PassiveLib path from environment variable

        this->loadPassiveLibPathFromEnVariable("MCELL_PATH");

        if (this->type == "inductor-symmetric") // symmetrical inductor
        {

            InductorSymmetric *ind = new InductorSymmetric(&this->techFile[0]);

            if (this->generateEm == 1 or this->generateSpiceModel == 1 or this->fastHenry == 0)
            {

                ind->process(this);
            }
            else if (this->fastHenry == 1)
            {
                ind->FastHenryProcess(this);
            }
            else if (this->fastHenry == 2)
            {
                ind->FastHenry2Process(this);
            }

            delete ind;
            ind = NULL;
        }
        else if (this->type == "inductor-spiral") // spiral inductor
        {

            InductorSpiral *ind = new InductorSpiral(&this->techFile[0]);

            if (this->generateEm == 1 or this->generateSpiceModel == 1 or this->fastHenry == 0)
            {
                ind->process(this);
            }
            else if (this->fastHenry == 1 or this->fastHenry == 2)
            {
                ind->FastHenryProcess(this);
            }

            delete ind;
            ind = NULL;
        }

        else if (this->type == "transformer1o1") // transformer1o1
        {

            Transformer1o1 *tr = new Transformer1o1(&this->techFile[0]);

            if (this->generateEm == 1 or this->generateSpiceModel == 1 or this->fastHenry == 0)
            {

                tr->process(this);
            }
            else if (this->fastHenry == 1)
            {
                tr->FastHenryProcess(this);
            }
            else if (this->fastHenry == 2)
            {
                tr->FastHenry2Process(this);
            }

            delete tr;
            tr = NULL;
        }
        else if (this->type == "transformer-spiral") // transformer1o1
        {
            TransformerSpiral *tr = new TransformerSpiral(&this->techFile[0]);

            if (this->generateEm == 1 or this->generateSpiceModel == 1 or this->fastHenry == 0)
            {

                tr->process(this);
            }
            else if (this->fastHenry == 1)
            {
                tr->FastHenryProcess(this);
            }
            else if (this->fastHenry == 2)
            {
                tr->FastHenry2Process(this);
            }

            delete tr;
            tr = NULL;
        }

        else if (this->type == "transformer2o2") // transformer2o2
        {
            Transformer2o2 *tr = new Transformer2o2(&this->techFile[0]);

            if (this->generateEm == 1 or this->generateSpiceModel == 1 or this->fastHenry == 0)
            {

                tr->process(this);
            }
            else if (this->fastHenry == 1)
            {
                tr->FastHenryProcess(this);
            }
            else if (this->fastHenry == 2)
            {
                tr->FastHenry2Process(this);
            }

            delete tr;
            tr = NULL;
        }
        else if (this->type == "transformer1o2") // transformer1o2
        {
            Transformer1o2 *tr = new Transformer1o2(&this->techFile[0]);

            if (this->generateEm == 1 or this->generateSpiceModel == 1 or this->fastHenry == 0)
            {

                tr->process(this);
            }
            else if (this->fastHenry == 1)
            {
                tr->FastHenryProcess(this);
            }
            else if (this->fastHenry == 2)
            {
                tr->FastHenry2Process(this);
            }

            delete tr;
            tr = NULL;
        }

        else if (this->type == "transformer2o1") // transformer2o1
        {
            Transformer2o1 *tr = new Transformer2o1(&this->techFile[0]);

            if (this->generateEm == 1 or this->generateSpiceModel == 1 or this->fastHenry == 0)
            {

                tr->process(this);
            }
            else if (this->fastHenry == 1)
            {
                tr->FastHenryProcess(this);
            }
            else if (this->fastHenry == 2)
            {
                tr->FastHenry2Process(this);
            }

            delete tr;
            tr = NULL;
        }

    } // try

    catch (HardError exception)
    {
        // ...code that handles HardError...
        cout << exception.name << endl;
        // clear and exit
        exit(1);
    }
    catch (...)
    {
        cout << "Unknown exception." << endl;
        exit(1);
    }

    return 0;
}
