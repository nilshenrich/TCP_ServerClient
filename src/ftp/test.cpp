// DEV: Debugging file to be deleted

#include <thread>
#include <sstream>
#include <iostream>

#include "FtpServer.hpp"

using namespace ::std;
using namespace ::tcp;
using namespace ::ftp;
using namespace ::std::chrono_literals;

class MyOstream : private streambuf, public ostream
{
public:
    MyOstream() : ostream(this) {}

private:
    int overflow(int c) override
    {
        cout << "MyOstream says '";
        cout.put(c);
        cout << "'" << endl;
        return c;
    }
};

int main()
{
    FtpServer server;
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
    server.setWork_readFile([](const string path) -> istringstream *
                            { return new istringstream{"My file content for file '"s + path + "'"s}; });
    server.setWork_writeFile([](const string path) -> MyOstream *
                             { return new MyOstream; });
    if (server.start())
        return -1;
    this_thread::sleep_for(5min);
}
