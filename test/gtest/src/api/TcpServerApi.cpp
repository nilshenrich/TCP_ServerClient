#include "TcpServerApi.h"

using namespace std;
using namespace TestApi;
using namespace tcp;

TcpServerApi_fragmentation::TcpServerApi_fragmentation(size_t messageMaxLen) : tcpServer{'\x00', messageMaxLen}
{
    tcpServer.setWorkOnMessage(bind(&TcpServerApi_fragmentation::workOnMessage, this, placeholders::_1, placeholders::_2));
    tcpServer.setWorkOnEstablished(bind(&TcpServerApi_fragmentation::workOnEstablished, this, placeholders::_1));
    tcpServer.setWorkOnClosed(bind(&TcpServerApi_fragmentation::workOnClosed, this, placeholders::_1));
}
TcpServerApi_fragmentation::~TcpServerApi_fragmentation() {}
TcpServerApi_continuous::TcpServerApi_continuous() : tcpServer{}
{
    tcpServer.setCreateForwardStream(bind(&TcpServerApi_continuous::generateContinuousStream, this, placeholders::_1));
    tcpServer.setWorkOnEstablished(bind(&TcpServerApi_continuous::workOnEstablished, this, placeholders::_1));
    tcpServer.setWorkOnClosed(bind(&TcpServerApi_continuous::workOnClosed, this, placeholders::_1));
}
TcpServerApi_continuous::~TcpServerApi_continuous() {}

int TcpServerApi_fragmentation::start(const int port)
{
    return tcpServer.start(port);
}

void TcpServerApi_fragmentation::stop()
{
    tcpServer.stop();
}

bool TcpServerApi_fragmentation::sendMsg(const int tcpClientId, const string &tcpMsg)
{
    return tcpServer.sendMsg(tcpClientId, tcpMsg);
}

vector<MessageFromClient> TcpServerApi_fragmentation::getBufferedMsg()
{
    lock_guard<mutex> lck{bufferedMsg_m};
    return move(bufferedMsg);
}

vector<int> TcpServerApi_fragmentation::getClientIds()
{
    return tcpServer.getAllClientIds();
}

void TcpServerApi_fragmentation::workOnMessage(const int tcpClientId, const string tcpMsgFromClient)
{
    lock_guard<mutex> lck{bufferedMsg_m};
    bufferedMsg.push_back({tcpClientId, move(tcpMsgFromClient)});
}

void TcpServerApi_fragmentation::workOnEstablished(const int) {}

void TcpServerApi_fragmentation::workOnClosed(const int) {}

int TcpServerApi_continuous::start(const int port)
{
    return tcpServer.start(port);
}

void TcpServerApi_continuous::stop()
{
    tcpServer.stop();
}

bool TcpServerApi_continuous::sendMsg(const int tcpClientId, const string &tcpMsg)
{
    return tcpServer.sendMsg(tcpClientId, tcpMsg);
}

map<int, string> TcpServerApi_continuous::getBufferedMsg()
{
    map<int, string> messages;
    for (auto &v : bufferedMsg)
    {
        string msg{v.second->str()};
        if (msg.size())
            messages[v.first] = msg;
    }
    bufferedMsg.clear();
    return messages;
}

vector<int> TcpServerApi_continuous::getClientIds()
{
    return tcpServer.getAllClientIds();
}

void TcpServerApi_continuous::workOnEstablished(const int) {}

void TcpServerApi_continuous::workOnClosed(const int) {}

ostringstream *TcpServerApi_continuous::generateContinuousStream(int clientId)
{
    bufferedMsg[clientId] = new ostringstream;
    return bufferedMsg[clientId];
}
