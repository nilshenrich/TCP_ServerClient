// DEV: Debugging file to be deleted

#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

#include "FtpServer.hpp"

using namespace ::std;
using namespace ::tcp;
using namespace ::ftp;
using namespace ::std::chrono_literals;

class MyStreambuf : public streambuf
{
public:
    MyStreambuf(streambuf *buf = cout.rdbuf()) : myStreambuf_p{buf},
                                                 myBuffer{valarray<char>(256)},
                                                 prepend{"MyOstream says: "},
                                                 append{}
    {
        setp(begin(myBuffer) + prepend.size(), end(myBuffer) - 1 - append.size());
    }

    int_type overflow(int_type c) override
    {
        if (c != traits_type::eof())
        {
            *pptr() = traits_type::to_char_type(c);
            pbump(1);
            output();
        }
        return c;
    }
    int sync() override
    {
        output();
        return 0;
    }

private:
    streambuf *myStreambuf_p;
    valarray<char> myBuffer;
    const string prepend;
    const string append;

    void output()
    {
        cout << "MyStreambuf::output() - Sending " << pptr() - pbase() << " bytes to stream: \"";
        for (int i{0}; i < pptr() - pbase(); i += 1)
        {
            char c{*(pbase() + i)};

            switch (c)
            {
            case '\0':
                cout << "\\0";
                break;
            case '\n':
                cout << "\\n";
                break;
            case '\r':
                cout << "\\r";
                break;
            default:
                cout << c;
                break;
            }
        }
        cout << "\"" << endl;
        for (size_t i{0}; i < prepend.size(); i += 1)
            myBuffer[i] = prepend[i];
        for (size_t i{0}; i < append.size(); i += 1)
            myBuffer[myBuffer.size() - append.size() + i] = append[i];
        myStreambuf_p->sputn(begin(myBuffer), myBuffer.size());
        setp(begin(myBuffer) + prepend.size(), end(myBuffer) - 1 - append.size());
    }
};

void test1()
{
    cout << endl
         << "==================================" << endl
         << "Test 1: MyStreambuf via ostream directly" << endl
         << "==================================" << endl;

    MyStreambuf myBuf;
    ostream ms{&myBuf};
    string msg{"Hello, world! - MyStreambuf"};
    ms << msg << endl;
    ms.rdbuf()->sputn(msg.c_str(), msg.size()); // DEV: Not sent out yet (Just buffered)
    ms << endl;                                 // DEV: msg sent out when called because sync-ed
}

void test2(bool finalSend)
{
    cout << endl
         << "==================================" << endl
         << "Test 2: DynamicOstream with MyStreambuf. Final send: " << finalSend << endl
         << "==================================" << endl;

    // DynamicOstream<16> myStream{}; // Buffer too small for first message -> Error
    DynamicOstream<32> myStream{}; // Buffer large enough for first message, but not for complete message -> First message buffered and complete message sent out in chunks
    // DynamicOstream<64> myStream{}; // Buffer large enough for complete message -> Complete message sent out in one go
    MyStreambuf myBuf;
    ostream myOstream{&myBuf};
    cout << "1: Status = " << myStream.rdbuf()->status() << endl;
    myStream << "Hello, world! - before" << endl;
    cout << "2: Status = " << myStream.rdbuf()->status() << endl;
    myStream.rdbuf()->setStreambuf(myOstream.rdbuf()); // FIXME: Nothing sent out to cout, why? (cout.rdbuf works)
    cout << "3: Status = " << myStream.rdbuf()->status() << endl;
    myStream << "Hello, world! - after" << endl;
    cout << "4: Status = " << myStream.rdbuf()->status() << endl;

    if (finalSend)
        myOstream << "Send something to MyOstream directly." << endl; // INFO: Actually this makes the MyOstream work (worked for cout as debug messages were sent to cout before)
}

void test3()
{
    cout << endl
         << "==================================" << endl
         << "Test 3: Use raw numbers (including 0)" << endl
         << "==================================" << endl;

    string zero_no;
    string zero_yes;
    for (char i{-10}; i < 10; i += 1)
    {
        zero_no += (i + 75);
        zero_yes += i;
    }

    DynamicOstream<32> myStream{};
    MyStreambuf myBuf;
    ostream myOstream{&myBuf};
    cout << "1: Status = " << myStream.rdbuf()->status() << endl;
    myStream << "zero-no:  " << zero_no << endl;
    cout << "2: Status = " << myStream.rdbuf()->status() << endl;
    myStream.redirect(&myBuf);
    cout << "3: Status = " << myStream.rdbuf()->status() << endl;
    myStream << "zero-yes: " << zero_yes << endl;
    cout << "4: Status = " << myStream.rdbuf()->status() << endl;

    myOstream << "Final endline" << endl;
}

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
    server.setWork_writeFile([](const string path, const ios::openmode mode) -> ostream *
                             {
                                cout << "[Test] Start writing to file '"s + path + "' in mode '"s + to_string(mode) + "'"s;
                                return new ostream{cout.rdbuf()}; });

    if (server.start())
        return -1;
    this_thread::sleep_for(5min);
}
