#ifndef GENERAL_TCP_CLIENT_TEST_START_H_
#define GENERAL_TCP_CLIENT_TEST_START_H_

#include <gtest/gtest.h>

#include "TcpServerApi.h"
#include "TcpClientApi.h"

namespace Test
{
    class General_TcpClient_Test_Start : public testing::Test
    {
    public:
        General_TcpClient_Test_Start();
        virtual ~General_TcpClient_Test_Start();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TCP server and Client
        TestApi::TcpServerApi_fragmentation tcpServer{};
        TestApi::TcpClientApi_fragmentation tcpClient{};

        // Port to use
        int port;
    };
}

#endif // GENERAL_TCP_CLIENT_TEST_START_H_
