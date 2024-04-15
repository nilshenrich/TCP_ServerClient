#ifndef FORWARDING_TCP_CONNECTION_TEST_MSGTRANSFER_H_
#define FORWARDING_TCP_CONNECTION_TEST_MSGTRANSFER_H_

#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <vector>
#include <string>

#include "TcpServerApi.h"
#include "TcpClientApi.h"
#include "HelperFunctions.h"

namespace Test
{
    class Forwarding_TcpConnection_Test_MsgTransfer : public testing::Test
    {
    public:
        Forwarding_TcpConnection_Test_MsgTransfer();
        virtual ~Forwarding_TcpConnection_Test_MsgTransfer();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TCP server and Client
        TestApi::TcpServerApi_forwarding tcpServer{};
        TestApi::TcpClientApi_forwarding tcpClient{};

        // Port to use
        int port;

        // Client ID
        int clientId;
    };
}

#endif // FORWARDING_TCP_CONNECTION_TEST_MSGTRANSFER_H_
