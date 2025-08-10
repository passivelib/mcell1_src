#include <fstream>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <iostream>
#include "mcell_command_line.h"
#include "mlic_license.h"
#include <stdlib.h>

using namespace std;

std::string licEnvVar = "MCELL_LICENSE";
mlic::FeatureType feature = mlic::FeatureType::MCELL;

int main(int argc, char **argv)
{

    // software version
    double version = 1.00;

    Command command;
    command.version = version;

    try
    {
        command.processArgs(argc, argv);

        // print version
        if (command.printVersion > 0)
        {
            std::cout << "version: " << version << std::endl;
            return 1;
        }

        // -----------------------------------------------------------------------------------
        // Check license
        // -----------------------------------------------------------------------------------
        mlic::License license(licEnvVar, version, feature);
        bool lv = license.getVL();
        command.lv = &lv;

        // -----------------------------------------------------------------------------------
        // Process
        // -----------------------------------------------------------------------------------
        command.process();
    }
    catch (HardError exception)
    {
        // ...code that handles HardError...
        cout << exception.name << endl;
        // clear and exit
        // exit(1);
    }
    catch (...)
    {
        cout << "Unknown exception." << endl;
        // exit(1);
    }

    return 0;
}
