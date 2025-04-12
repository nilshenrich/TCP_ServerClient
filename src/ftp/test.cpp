// DEV: Debugging file to be deleted

#include <thread>
#include <sstream>
#include <iostream>

#include "FtpServer.hpp"

using namespace ::std;
using namespace ::tcp;
using namespace ::ftp;
using namespace ::std::chrono_literals;

class MyTempOstream : private streambuf, public ostream
{
public:
    MyTempOstream() : ostream(this) {}

private:
    int overflow(int c) override
    {
        cout << "MyTempOstream says '";
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
    server.setWork_readFile([](const string path, const ios::openmode mode) -> istringstream *
                            {
                                cout << "[Test] Start reading file '"s + path + "' in mode '"s + to_string(mode) + "'"s; 
                                return new istringstream{"My file content for file '"s + path + "'"s, mode}; });
    server.setWork_writeTempFile([](const ios::openmode mode) -> MyTempOstream *
                                 {
                                    cout << "[Test] Start writing to temporary file in mode '"s + to_string(mode) + "'"s;
                                    return new MyTempOstream(); });
    server.setWork_moveTempFile([](const string path)
                                { cout << "Move temp file to " << path << endl; });

    if (server.start())
        return -1;
    this_thread::sleep_for(5min);
}
