#ifndef GENERAL_TCP_SERVER_TEST_START_H_
#define GENERAL_TCP_SERVER_TEST_START_H_

#include <gtest/gtest.h>

#include "TcpServerApi.h"
#include "TcpClientApi.h"

namespace Test
{
    class General_TcpServer_Test_Start : public testing::Test
    {
    public:
        General_TcpServer_Test_Start();
        virtual ~General_TcpServer_Test_Start();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TCP Server
        TestApi::TcpServerApi_fragmentation tcpServer{};

        // Port to use
        int port;
    };
} // namespace Test

#endif // GENERAL_TCP_SERVER_TEST_START_H_
