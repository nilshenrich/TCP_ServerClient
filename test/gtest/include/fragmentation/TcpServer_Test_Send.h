#ifndef FRAGMENTATION_TCP_SERVER_TEST_SEND_H_
#define FRAGMENTATION_TCP_SERVER_TEST_SEND_H_

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
    class Fragmentation_TcpServer_Test_Send : public testing::Test
    {
    public:
        Fragmentation_TcpServer_Test_Send();
        virtual ~Fragmentation_TcpServer_Test_Send();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TCP server and Client
        TestApi::TcpServerApi_fragmentation tcpServer{};
        TestApi::TcpClientApi_fragmentation tcpClient{};

        // Port to use
        int port;

        // Client ID
        int clientId;
    };
}

#endif // FRAGMENTATION_TCP_SERVER_TEST_SEND_H_
