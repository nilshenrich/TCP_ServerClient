#ifndef FRAGMENTATION_TLS_CONNECTION_TEST_MSGTRANSFER_H_
#define FRAGMENTATION_TLS_CONNECTION_TEST_MSGTRANSFER_H_

#include "TlsServerApi.h"
#include "TlsClientApi.h"

namespace Test
{
    class Fragmentation_TlsConnection_Test_MsgTransfer : public testing::Test
    {
    public:
        Fragmentation_TlsConnection_Test_MsgTransfer();
        virtual ~Fragmentation_TlsConnection_Test_MsgTransfer();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TLS server and Client
        TestApi::TlsServerApi_fragmentation tlsServer_selfLong_frgnLong{};
        TestApi::TlsClientApi_fragmentation tlsClient_selfLong_frgnLong{};
        TestApi::TlsServerApi_fragmentation tlsServer_selfLong_frgnShort{};
        TestApi::TlsClientApi_fragmentation tlsClient_selfLong_frgnShort{};
        TestApi::TlsServerApi_fragmentation tlsServer_selfShort_frgnLong{TestConstants::MAXLEN_MSG_SHORT_B};
        TestApi::TlsClientApi_fragmentation tlsClient_selfShort_frgnLong{TestConstants::MAXLEN_MSG_SHORT_B};
        TestApi::TlsServerApi_fragmentation tlsServer_selfShort_frgnShort{TestConstants::MAXLEN_MSG_SHORT_B};
        TestApi::TlsClientApi_fragmentation tlsClient_selfShort_frgnShort{TestConstants::MAXLEN_MSG_SHORT_B};

        // Port to use
        int port_serverLong_clientLong;
        int port_serverLong_clientShort;
        int port_serverShort_clientLong;
        int port_serverShort_clientShort;

        // Client ID
        int clientId_serverLong_clientLong;
        int clientId_serverLong_clientShort;
        int clientId_serverShort_clientLong;
        int clientId_serverShort_clientShort;
    };
}

#endif // FRAGMENTATION_TLS_CONNECTION_TEST_MSGTRANSFER_H_
