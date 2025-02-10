#ifndef CONTINUOUS_TCP_SERVER_TEST_SEND_H_
#define CONTINUOUS_TCP_SERVER_TEST_SEND_H_

#include <gtest/gtest.h>

#include "TcpServerApi.h"
#include "TcpClientApi.h"

namespace Test
{
    class Continuous_TcpServer_Test_Send : public testing::Test
    {
    public:
        Continuous_TcpServer_Test_Send();
        virtual ~Continuous_TcpServer_Test_Send();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TCP server and Client
        TestApi::TcpServerApi_continuous tcpServer{};
        TestApi::TcpClientApi_continuous tcpClient{};

        // Port to use
        int port;

        // Client ID
        int clientId;
    };
}

#endif // CONTINUOUS_TCP_SERVER_TEST_SEND_H_
