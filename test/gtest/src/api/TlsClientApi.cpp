#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <mutex>
#include <functional>

#include "TlsClientApi.h"
#include "TestDefines.h"

using namespace std;
using namespace TestApi;
using namespace tcp;

TlsClientApi_fragmentation::TlsClientApi_fragmentation() : tlsClient{'\x00', "", TestConstants::MAXLEN_MSG_B}
{
    tlsClient.setWorkOnMessage(bind(&TlsClientApi_fragmentation::workOnMessage, this, placeholders::_1));
}
TlsClientApi_fragmentation::TlsClientApi_fragmentation(const string &messageAppend) : tlsClient{'\x00', messageAppend, TestConstants::MAXLEN_MSG_B}
{
    tlsClient.setWorkOnMessage(bind(&TlsClientApi_fragmentation::workOnMessage, this, placeholders::_1));
}
TlsClientApi_fragmentation::TlsClientApi_fragmentation(size_t messageMaxLen) : tlsClient{'\x00', "", messageMaxLen}
{
    tlsClient.setWorkOnMessage(bind(&TlsClientApi_fragmentation::workOnMessage, this, placeholders::_1));
}
TlsClientApi_fragmentation::TlsClientApi_fragmentation(const string &messageAppend, size_t messageMaxLen) : tlsClient{'\x00', messageAppend, messageMaxLen}
{
    tlsClient.setWorkOnMessage(bind(&TlsClientApi_fragmentation::workOnMessage, this, placeholders::_1));
}
TlsClientApi_fragmentation::~TlsClientApi_fragmentation() {}
TlsClientApi_continuous::TlsClientApi_continuous() : tlsClient{bufferedMsg_os} {}
TlsClientApi_continuous::~TlsClientApi_continuous() {}

int TlsClientApi_fragmentation::start(const string &ip, const int port, string pathToCaCert, string pathToClientCert, string pathToClientKey, const bool serverAuth)
{
    tlsClient.requireServerAuthentication(serverAuth);
    tlsClient.setCertificates(pathToCaCert, pathToClientCert, pathToClientKey);
    int start{tlsClient.start(ip, port)};
    this_thread::sleep_for(TestConstants::WAITFOR_CONNECT_TLS);
    return start;
}

void TlsClientApi_fragmentation::stop()
{
    tlsClient.stop();
    this_thread::sleep_for(TestConstants::WAITFOR_DISCONNECT_TLS);
    return;
}

bool TlsClientApi_fragmentation::sendMsg(const string &tlsMsg)
{
    return tlsClient.sendMsg(tlsMsg);
}

vector<string> TlsClientApi_fragmentation::getBufferedMsg()
{
    lock_guard<mutex> lck{bufferedMsg_m};
    return move(bufferedMsg);
}

void TlsClientApi_fragmentation::workOnMessage(const string tlsMsgFromServer)
{
    lock_guard<mutex> lck{bufferedMsg_m};
    bufferedMsg.push_back(move(tlsMsgFromServer));
    return;
}

string TlsClientApi_fragmentation::getSubjPartFromServerCert(const int subjPart)
{
    return tlsClient.getSubjPartFromServerCert(subjPart);
}

int TlsClientApi_continuous::start(const string &ip, const int port, string pathToCaCert, string pathToClientCert, string pathToClientKey, const bool serverAuth)
{
    tlsClient.requireServerAuthentication(serverAuth);
    tlsClient.setCertificates(pathToCaCert, pathToClientCert, pathToClientKey);
    int start{tlsClient.start(ip, port)};
    this_thread::sleep_for(TestConstants::WAITFOR_CONNECT_TLS);
    return start;
}

void TlsClientApi_continuous::stop()
{
    tlsClient.stop();
    this_thread::sleep_for(TestConstants::WAITFOR_DISCONNECT_TLS);
    return;
}

bool TlsClientApi_continuous::sendMsg(const string &tlsMsg)
{
    return tlsClient.sendMsg(tlsMsg);
}

string TlsClientApi_continuous::getBufferedMsg()
{
    return bufferedMsg_os.str();
}

string TlsClientApi_continuous::getSubjPartFromServerCert(const int subjPart)
{
    return tlsClient.getSubjPartFromServerCert(subjPart);
}
