#include <unistd.h>
#include <netinet/in.h>
#include <iostream>

#include "HelperFunctions.h"
#include "algorithms.hpp"

using namespace std;

bool HelperFunctions::pipeError{false};

int HelperFunctions::getFreePort()
{
    return ::tcp::algorithms::getFreePort(1024, 65535);
}

void HelperFunctions::setPipeError()
{
    pipeError = true;
    cout << "SIGPIPE detected" << endl;
    return;
}

bool HelperFunctions::getAndResetPipeError()
{
    bool ret{pipeError};
    pipeError = false;
    return ret;
}
