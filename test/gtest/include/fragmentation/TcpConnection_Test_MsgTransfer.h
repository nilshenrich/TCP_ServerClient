#ifndef FRAGMENTATION_TCP_CONNECTION_TEST_MSGTRANSFER_H_
#define FRAGMENTATION_TCP_CONNECTION_TEST_MSGTRANSFER_H_

#include <gtest/gtest.h>

#include "TcpServerApi.h"
#include "TcpClientApi.h"

namespace Test
{
    class Fragmentation_TcpConnection_Test_MsgTransfer : public testing::Test
    {
    public:
        Fragmentation_TcpConnection_Test_MsgTransfer();
        virtual ~Fragmentation_TcpConnection_Test_MsgTransfer();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TCP server and Client
        TestApi::TcpServerApi_fragmentation tcpServer_selfLong_frgnLong{};
        TestApi::TcpClientApi_fragmentation tcpClient_selfLong_frgnLong{};
        TestApi::TcpServerApi_fragmentation tcpServer_selfLong_frgnShort{};
        TestApi::TcpClientApi_fragmentation tcpClient_selfLong_frgnShort{};
        TestApi::TcpServerApi_fragmentation tcpServer_selfShort_frgnLong{TestConstants::MAXLEN_MSG_SHORT_B};
        TestApi::TcpClientApi_fragmentation tcpClient_selfShort_frgnLong{TestConstants::MAXLEN_MSG_SHORT_B};
        TestApi::TcpServerApi_fragmentation tcpServer_selfShort_frgnShort{TestConstants::MAXLEN_MSG_SHORT_B};
        TestApi::TcpClientApi_fragmentation tcpClient_selfShort_frgnShort{TestConstants::MAXLEN_MSG_SHORT_B};

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

#endif // FRAGMENTATION_TCP_CONNECTION_TEST_MSGTRANSFER_H_
