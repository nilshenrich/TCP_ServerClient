// DEV: Debugging file to be deleted

#include <thread>
#include <sstream>
#include <iostream>

#include "FtpServer.hpp"

using namespace ::std;
using namespace ::tcp;
using namespace ::ftp;
using namespace ::std::chrono_literals;

int main()
{
    FtpServer server;
    istringstream iFile{"My file content"};
    server.setWork_checkUserCredentials([](const string, const string) -> bool
                                        { return true; });
    server.setWork_checkAccessible([](const string, const string) -> bool
                                   { return true; });
    server.setWork_listDirectory([](const string) -> valarray<Item>
                                 { return valarray<Item>{Item{ItemType::directory, "MyDir", {6, 4, 4}, 0, 10, 11, 4096, 1722164144},
                                                         Item{ItemType::directory, "MyDir2", {6, 4, 4}, 0, 10, 11, 4096000, 1722164144},
                                                         Item{ItemType::file, "MyFile", {6, 4, 4}, 0, 10, 11, 4096, 1722164144}}; });
    server.setWork_createDirectory([](const string) -> bool
                                   { return true; });
    server.setWork_readFile([&iFile](const string) -> istringstream *
                            { return &iFile; });
    server.setWork_writeFile([](const string) -> ostream *
                             { return new ostringstream; });
    if (server.start())
        return -1;
    this_thread::sleep_for(5min);
}
