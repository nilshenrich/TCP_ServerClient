#ifndef CONTINUOUS_TCP_CONNECTION_TEST_MSGTRANSFER_H_
#define CONTINUOUS_TCP_CONNECTION_TEST_MSGTRANSFER_H_

#include "TcpServerApi.h"
#include "TcpClientApi.h"

namespace Test
{
    class Continuous_TcpConnection_Test_MsgTransfer : public testing::Test
    {
    public:
        Continuous_TcpConnection_Test_MsgTransfer();
        virtual ~Continuous_TcpConnection_Test_MsgTransfer();

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

#endif // CONTINUOUS_TCP_CONNECTION_TEST_MSGTRANSFER_H_
