#include "TcpClientApi.h"

using namespace std;
using namespace TestApi;
using namespace tcp;

TcpClientApi_fragmentation::TcpClientApi_fragmentation(size_t messageMaxLen) : tcpClient{'\x00', messageMaxLen}
{
    tcpClient.setWorkOnMessage(bind(&TcpClientApi_fragmentation::workOnMessage, this, placeholders::_1));
}
TcpClientApi_fragmentation::~TcpClientApi_fragmentation() {}
TcpClientApi_continuous::TcpClientApi_continuous() : tcpClient{bufferedMsg_os} {}
TcpClientApi_continuous::~TcpClientApi_continuous() {}

int TcpClientApi_fragmentation::start(const string &ip, const int port)
{
    int start{tcpClient.start(ip, port)};
    this_thread::sleep_for(TestConstants::WAITFOR_CONNECT_TCP);
    return start;
}

void TcpClientApi_fragmentation::stop()
{
    tcpClient.stop();
    return;
}

bool TcpClientApi_fragmentation::sendMsg(const string &tcpMsg)
{
    return tcpClient.sendMsg(tcpMsg);
}

vector<string> TcpClientApi_fragmentation::getBufferedMsg()
{
    lock_guard<mutex> lck{bufferedMsg_m};
    return move(bufferedMsg);
}

void TcpClientApi_fragmentation::workOnMessage(const string tcpMsgFromServer)
{
    lock_guard<mutex> lck{bufferedMsg_m};
    bufferedMsg.push_back(move(tcpMsgFromServer));
    return;
}

int TcpClientApi_continuous::start(const string &ip, const int port)
{
    int start{tcpClient.start(ip, port)};
    this_thread::sleep_for(TestConstants::WAITFOR_CONNECT_TCP);
    return start;
}

void TcpClientApi_continuous::stop()
{
    tcpClient.stop();
    return;
}

bool TcpClientApi_continuous::sendMsg(const string &tcpMsg)
{
    return tcpClient.sendMsg(tcpMsg);
}

string TcpClientApi_continuous::getBufferedMsg()
{
    return bufferedMsg_os.str();
}
