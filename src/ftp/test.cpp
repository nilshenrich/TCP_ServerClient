// DEV: Debugging file to be deleted

#include <thread>

#include "FtpServer.hpp"

using namespace ::std;
using namespace ::tcp;
using namespace ::ftp;
using namespace ::std::chrono_literals;

int main()
{
    FtpServer server;
    server.start();
    this_thread::sleep_for(5min);
}
