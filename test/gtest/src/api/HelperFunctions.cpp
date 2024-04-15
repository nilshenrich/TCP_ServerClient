#include "HelperFunctions.h"

using namespace std;

bool HelperFunctions::pipeError{false};

int HelperFunctions::getFreePort()
{
    for (int port{1024}; port < 65536; port += 1)
    {
        int sock{socket(AF_INET, SOCK_STREAM, 0)};
        if (-1 == sock)
            return -1;

        struct sockaddr_in sin;
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons(port);

        if (!bind(sock, (struct sockaddr *)&sin, sizeof(sin)))
        {
            close(sock);
            return port;
        }
    }

    // If we get here, no free port was found. Return -1.
    return -1;
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
