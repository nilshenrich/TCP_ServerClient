#include "TlsClientApi.h"

using namespace std;
using namespace TestApi;
using namespace tcp_serverclient;

TlsClientApi_fragmentation::TlsClientApi_fragmentation(size_t messageMaxLen) : tlsClient{'\x00', messageMaxLen}
{
    tlsClient.setWorkOnMessage(bind(&TlsClientApi_fragmentation::workOnMessage, this, placeholders::_1));
}
TlsClientApi_fragmentation::~TlsClientApi_fragmentation() {}
TlsClientApi_continuous::TlsClientApi_continuous() : tlsClient{bufferedMsg_os} {}
TlsClientApi_continuous::~TlsClientApi_continuous() {}

int TlsClientApi_fragmentation::start(const string &ip, const int port, string pathToCaCert, string pathToClientCert, string pathToClientKey)
{
    int start{tlsClient.start(ip, port, pathToCaCert.c_str(), pathToClientCert.c_str(), pathToClientKey.c_str())};
    this_thread::sleep_for(TestConstants::WAITFOR_CONNECT_TLS);
    return start;
}

void TlsClientApi_fragmentation::stop()
{
    tlsClient.stop();
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

int TlsClientApi_continuous::start(const string &ip, const int port, string pathToCaCert, string pathToClientCert, string pathToClientKey)
{
    int start{tlsClient.start(ip, port, pathToCaCert.c_str(), pathToClientCert.c_str(), pathToClientKey.c_str())};
    this_thread::sleep_for(TestConstants::WAITFOR_CONNECT_TLS);
    return start;
}

void TlsClientApi_continuous::stop()
{
    tlsClient.stop();
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
