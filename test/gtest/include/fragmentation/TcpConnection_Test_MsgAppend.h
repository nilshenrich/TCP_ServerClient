#ifndef FRAGMENTATION_TCP_CONNECTION_TEST_MSGAPPEND_H_
#define FRAGMENTATION_TCP_CONNECTION_TEST_MSGAPPEND_H_

#include "TcpServerApi.h"
#include "TcpClientApi.h"

namespace Test
{

    class Fragmentation_TcpConnection_Test_MsgAppend : public testing::Test
    {
    public:
        Fragmentation_TcpConnection_Test_MsgAppend();
        ~Fragmentation_TcpConnection_Test_MsgAppend();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TCP server and Client
        ::std::string messageAppendServer{"Appended message from server"};
        ::std::string messageAppendClient{"Appended message from client"};
        TestApi::TcpServerApi_fragmentation tcpServer{messageAppendServer};
        TestApi::TcpClientApi_fragmentation tcpClient{messageAppendClient};

        // Port to use
        int port;

        // Client ID
        int clientId;
    };
}

#endif // FRAGMENTATION_TCP_CONNECTION_TEST_MSGAPPEND_H_
