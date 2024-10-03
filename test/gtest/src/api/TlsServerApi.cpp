#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <mutex>
#include <functional>

#include "TlsServerApi.h"
#include "TestDefines.h"

using namespace std;
using namespace TestApi;
using namespace tcp;

TlsServerApi_fragmentation::TlsServerApi_fragmentation(size_t messageMaxLen) : tlsServer{'\x00', messageMaxLen}
{
    tlsServer.setWorkOnMessage(bind(&TlsServerApi_fragmentation::workOnMessage, this, placeholders::_1, placeholders::_2));
    tlsServer.setWorkOnEstablished(bind(&TlsServerApi_fragmentation::workOnEstablished, this, placeholders::_1));
    tlsServer.setWorkOnClosed(bind(&TlsServerApi_fragmentation::workOnClosed, this, placeholders::_1));
}
TlsServerApi_fragmentation::~TlsServerApi_fragmentation() {}
TlsServerApi_continuous::TlsServerApi_continuous() : tlsServer{}
{
    tlsServer.setCreateForwardStream(bind(&TlsServerApi_continuous::generateContinuousStream, this, placeholders::_1));
    tlsServer.setWorkOnEstablished(bind(&TlsServerApi_continuous::workOnEstablished, this, placeholders::_1));
    tlsServer.setWorkOnClosed(bind(&TlsServerApi_continuous::workOnClosed, this, placeholders::_1));
}
TlsServerApi_continuous::~TlsServerApi_continuous() {}

int TlsServerApi_fragmentation::start(const int port, const string pathToCaCert, const string pathToServerCert, const string pathToServerKey)
{
    return tlsServer.start(port, pathToCaCert.c_str(), pathToServerCert.c_str(), pathToServerKey.c_str());
}

void TlsServerApi_fragmentation::stop()
{
    tlsServer.stop();
}

bool TlsServerApi_fragmentation::sendMsg(const int tlsClientId, const string &tlsMsg)
{
    return tlsServer.sendMsg(tlsClientId, tlsMsg);
}

vector<MessageFromClient> TlsServerApi_fragmentation::getBufferedMsg()
{
    lock_guard<mutex> lck{bufferedMsg_m};
    return move(bufferedMsg);
}

vector<int> TlsServerApi_fragmentation::getClientIds()
{
    return tlsServer.getAllClientIds();
}

void TlsServerApi_fragmentation::workOnMessage(const int tlsClientId, const string tlsMsgFromClient)
{
    lock_guard<mutex> lck{bufferedMsg_m};
    bufferedMsg.push_back({tlsClientId, move(tlsMsgFromClient)});
}

void TlsServerApi_fragmentation::workOnEstablished(const int) {}

void TlsServerApi_fragmentation::workOnClosed(const int) {}

int TlsServerApi_continuous::start(const int port, const string pathToCaCert, const string pathToServerCert, const string pathToServerKey)
{
    return tlsServer.start(port, pathToCaCert.c_str(), pathToServerCert.c_str(), pathToServerKey.c_str());
}

void TlsServerApi_continuous::stop()
{
    tlsServer.stop();
}

bool TlsServerApi_continuous::sendMsg(const int tlsClientId, const string &tlsMsg)
{
    return tlsServer.sendMsg(tlsClientId, tlsMsg);
}

map<int, string> TlsServerApi_continuous::getBufferedMsg()
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

vector<int> TlsServerApi_continuous::getClientIds()
{
    return tlsServer.getAllClientIds();
}

void TlsServerApi_continuous::workOnEstablished(const int) {}

void TlsServerApi_continuous::workOnClosed(const int) {}

ostringstream *TlsServerApi_continuous::generateContinuousStream(int clientId)
{
    bufferedMsg[clientId] = new ostringstream;
    return bufferedMsg[clientId];
}
