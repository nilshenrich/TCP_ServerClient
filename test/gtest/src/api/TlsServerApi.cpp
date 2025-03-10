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

TlsServerApi_fragmentation::TlsServerApi_fragmentation() : tlsServer{'\x00', "", TestConstants::MAXLEN_MSG_B}
{
    tlsServer.setWorkOnMessage(bind(&TlsServerApi_fragmentation::workOnMessage, this, placeholders::_1, placeholders::_2));
    tlsServer.setWorkOnEstablished(bind(&TlsServerApi_fragmentation::workOnEstablished, this, placeholders::_1));
    tlsServer.setWorkOnClosed(bind(&TlsServerApi_fragmentation::workOnClosed, this, placeholders::_1));
}
TlsServerApi_fragmentation::TlsServerApi_fragmentation(const string &messageAppend) : tlsServer{'\x00', messageAppend, TestConstants::MAXLEN_MSG_B}
{
    tlsServer.setWorkOnMessage(bind(&TlsServerApi_fragmentation::workOnMessage, this, placeholders::_1, placeholders::_2));
    tlsServer.setWorkOnEstablished(bind(&TlsServerApi_fragmentation::workOnEstablished, this, placeholders::_1));
    tlsServer.setWorkOnClosed(bind(&TlsServerApi_fragmentation::workOnClosed, this, placeholders::_1));
}
TlsServerApi_fragmentation::TlsServerApi_fragmentation(size_t messageMaxLen) : tlsServer{'\x00', "", messageMaxLen}
{
    tlsServer.setWorkOnMessage(bind(&TlsServerApi_fragmentation::workOnMessage, this, placeholders::_1, placeholders::_2));
    tlsServer.setWorkOnEstablished(bind(&TlsServerApi_fragmentation::workOnEstablished, this, placeholders::_1));
    tlsServer.setWorkOnClosed(bind(&TlsServerApi_fragmentation::workOnClosed, this, placeholders::_1));
}
TlsServerApi_fragmentation::TlsServerApi_fragmentation(const string &messageAppend, size_t messageMaxLen) : tlsServer{'\x00', messageAppend, messageMaxLen}
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

int TlsServerApi_fragmentation::start(const int port, const string pathToCaCert, const string pathToServerCert, const string pathToServerKey, const bool clientAuth)
{
    tlsServer.requireClientAuthentication(clientAuth);
    tlsServer.setCertificates(pathToCaCert, pathToServerCert, pathToServerKey);
    return tlsServer.start(port);
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

string TlsServerApi_fragmentation::getClientIp(const int clientId) const
{
    return tlsServer.getClientIp(clientId);
}

string TlsServerApi_fragmentation::getSubjPartFromClientCert(const int clientId, const int subjPart)
{
    return tlsServer.getSubjPartFromClientCert(clientId, subjPart);
}

void TlsServerApi_fragmentation::workOnMessage(const int tlsClientId, const string tlsMsgFromClient)
{
    lock_guard<mutex> lck{bufferedMsg_m};
    bufferedMsg.push_back({tlsClientId, move(tlsMsgFromClient)});
}

void TlsServerApi_fragmentation::workOnEstablished(const int) {}

void TlsServerApi_fragmentation::workOnClosed(const int) {}

int TlsServerApi_continuous::start(const int port, const string pathToCaCert, const string pathToServerCert, const string pathToServerKey, const bool clientAuth)
{
    tlsServer.requireClientAuthentication(clientAuth);
    tlsServer.setCertificates(pathToCaCert, pathToServerCert, pathToServerKey);
    return tlsServer.start(port);
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

string TlsServerApi_continuous::getClientIp(const int clientId) const
{
    return tlsServer.getClientIp(clientId);
}

string TlsServerApi_continuous::getSubjPartFromClientCert(const int clientId, const int subjPart)
{
    return tlsServer.getSubjPartFromClientCert(clientId, subjPart);
}

void TlsServerApi_continuous::workOnEstablished(const int) {}

void TlsServerApi_continuous::workOnClosed(const int) {}

ostringstream *TlsServerApi_continuous::generateContinuousStream(int clientId)
{
    bufferedMsg[clientId] = new ostringstream;
    return bufferedMsg[clientId];
}
