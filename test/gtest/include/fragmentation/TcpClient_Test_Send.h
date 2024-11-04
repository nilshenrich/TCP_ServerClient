#ifndef FRAGMENTATION_TCP_CLIENT_TEST_SEND_H_
#define FRAGMENTATION_TCP_CLIENT_TEST_SEND_H_

#include "TcpServerApi.h"
#include "TcpClientApi.h"

namespace Test
{
    class Fragmentation_TcpClient_Test_Send : public testing::Test
    {
    public:
        Fragmentation_TcpClient_Test_Send();
        virtual ~Fragmentation_TcpClient_Test_Send();

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

#endif // FRAGMENTATION_TCP_CLIENT_TEST_SEND_H_
