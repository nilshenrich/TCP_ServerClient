#ifndef FORWARDING_TCP_CLIENT_TEST_SEND_H_
#define FORWARDING_TCP_CLIENT_TEST_SEND_H_

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
    class Forwarding_TcpClient_Test_Send : public testing::Test
    {
    public:
        Forwarding_TcpClient_Test_Send();
        virtual ~Forwarding_TcpClient_Test_Send();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TCP server and Client
        TestApi::TcpServerApi_forwarding tcpServer{};
        TestApi::TcpClientApi_forwarding tcpClient{};

        // Port to use
        int port;
    };
}

#endif // FORWARDING_TCP_CLIENT_TEST_SEND_H_
