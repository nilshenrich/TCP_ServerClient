#include "TlsClientApi.h"

using namespace std;
using namespace TestApi;
using namespace networking;

TlsClientApi_fragmentation::TlsClientApi_fragmentation(size_t messageMaxLen) : tlsClient{'\x00', messageMaxLen}
{
    tlsClient.setWorkOnMessage(bind(&TlsClientApi_fragmentation::workOnMessage, this, placeholders::_1));
}
TlsClientApi_fragmentation::~TlsClientApi_fragmentation() {}
TlsClientApi_forwarding::TlsClientApi_forwarding() : tlsClient{bufferedMsg_os} {}
TlsClientApi_forwarding::~TlsClientApi_forwarding() {}

int TlsClientApi_fragmentation::start(const std::string &ip, const int port, string pathToCaCert, string pathToClientCert, string pathToClientKey)
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

bool TlsClientApi_fragmentation::sendMsg(const std::string &tlsMsg)
{
    return tlsClient.sendMsg(tlsMsg);
}

vector<string> TlsClientApi_fragmentation::getBufferedMsg()
{
    lock_guard<mutex> lck{bufferedMsg_m};
    return move(bufferedMsg);
}

void TlsClientApi_fragmentation::workOnMessage(const std::string tlsMsgFromServer)
{
    lock_guard<mutex> lck{bufferedMsg_m};
    bufferedMsg.push_back(move(tlsMsgFromServer));
    return;
}

int TlsClientApi_forwarding::start(const std::string &ip, const int port, string pathToCaCert, string pathToClientCert, string pathToClientKey)
{
    int start{tlsClient.start(ip, port, pathToCaCert.c_str(), pathToClientCert.c_str(), pathToClientKey.c_str())};
    this_thread::sleep_for(TestConstants::WAITFOR_CONNECT_TLS);
    return start;
}

void TlsClientApi_forwarding::stop()
{
    tlsClient.stop();
    return;
}

bool TlsClientApi_forwarding::sendMsg(const std::string &tlsMsg)
{
    return tlsClient.sendMsg(tlsMsg);
}

string TlsClientApi_forwarding::getBufferedMsg()
{
    return bufferedMsg_os.str();
}
