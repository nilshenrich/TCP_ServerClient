// DEV: Debugging file to be deleted

#include <iostream>
#include <sstream>
#include <thread>

#include "FtpServer.hpp"

using namespace ::std;
using namespace ::tcp;
using namespace ::ftp;
using namespace ::std::chrono_literals;

class MyOstreambuf : public streambuf
{
public:
    MyOstreambuf(streambuf *buf = cout.rdbuf()) : myStreambuf_p{buf},
                                                  myBuffer{valarray<char>(256)},
                                                  prepend{"MyOstream says: "},
                                                  append{}
    {
        setp(begin(myBuffer) + prepend.size(), end(myBuffer) - 1 - append.size());
    }
    virtual ~MyOstreambuf() {}

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
        cout << "MyOstreambuf::output() - Sending " << pptr() - pbase() << " bytes to stream: \"";
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

class MyOstream : public ostream
{
public:
    MyOstream() : ostream{new MyOstreambuf()} {}
    ~MyOstream() {}
};

void test1()
{
    cout << endl
         << "==================================" << endl
         << "Test 1: MyOstreambuf via MyOstream" << endl
         << "==================================" << endl;

    MyOstream ms;
    string msg{"Hello, world! - MyOstreambuf"};
    ms << msg << endl;
    ms.rdbuf()->sputn(msg.c_str(), msg.size()); // DEV: Not sent out yet (Just buffered)
    ms << endl;                                 // DEV: msg sent out when called because sync-ed
}

void test2()
{
    cout << endl
         << "==================================" << endl
         << "Test 2: DynamicOstream with MyOstreambuf" << endl
         << "==================================" << endl;

    // DynamicOstream<16> myStream{}; // Buffer too small for first message -> Error
    DynamicOstream<32> myStream{}; // Buffer large enough for first message, but not for complete message -> First message buffered and complete message sent out in chunks
    // DynamicOstream<64> myStream{}; // Buffer large enough for complete message -> Complete message sent out in one go
    MyOstream myOstream;
    cout << "1: Status = " << myStream.rdbuf()->status() << endl;
    myStream << "Hello, world! - before" << endl;
    cout << "2: Status = " << myStream.rdbuf()->status() << endl;
    myStream.rdbuf()->setStreambuf(myOstream.rdbuf()); // FIXME: Nothing sent out to cout, why? (cout.rdbuf works)
    cout << "3: Status = " << myStream.rdbuf()->status() << endl;
    myStream << "Hello, world! - after" << endl;
    cout << "4: Status = " << myStream.rdbuf()->status() << endl;
}

void test3()
{
    cout << endl
         << "==================================" << endl
         << "Test 3: DynamicOstream with MyOstreambuf (via temp pointer)" << endl
         << "==================================" << endl;

    // DynamicOstream<16> myStream{}; // Buffer too small for first message -> Error
    DynamicOstream<32> myStream{}; // Buffer large enough for first message, but not for complete message -> First message buffered and complete message sent out in chunks
    // DynamicOstream<64> myStream{}; // Buffer large enough for complete message -> Complete message sent out in one go
    // ofstream myOstream{"test.txt"};
    MyOstream myOstream;
    auto temp_buf{static_cast<DynamicStreambuf<32> *>(myStream.rdbuf())};
    myOstream << "Hello, world! - direct to myOstream - before all" << endl;
    cout << "1: Status = " << temp_buf->status() << endl;
    myStream << "Hello, world! - before" << endl;
    cout << "2: Status = " << temp_buf->status() << endl;
    temp_buf->setStreambuf(myOstream.rdbuf()); // DEV: Works fine, just if used via temp_buf pointer that is determined before (even if DynamicOstream overrides rdbuf() method)
    cout << "3: Status = " << temp_buf->status() << endl;
    myStream << "Hello, world! - after" << endl;
    cout << "4: Status = " << temp_buf->status() << endl;
    myOstream << "Hello, world! - direct to myOstream - after all" << endl;
}

int main()
{
    test1();
    test2();
    test3();
    return 0;

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
