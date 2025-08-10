#include <fstream>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <iostream>
#include "mcell_command_line.h"
#include <stdlib.h>

using namespace std;

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
        bool lv = true;
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
