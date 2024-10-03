#ifndef CONTINUOUS_TCP_SERVER_TEST_MANYCLIENTS_H_
#define CONTINUOUS_TCP_SERVER_TEST_MANYCLIENTS_H_

#include "TcpServerApi.h"
#include "TcpClientApi.h"

namespace Test
{
    class Continuous_TcpServer_Test_ManyClients : public testing::Test
    {
    public:
        Continuous_TcpServer_Test_ManyClients();
        ~Continuous_TcpServer_Test_ManyClients();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TCP server and collection of clients
        TestApi::TcpServerApi_continuous tcpServer;
        ::std::map<int, ::std::unique_ptr<TestApi::TcpClientApi_continuous>> tcpClients;

        // Port to use
        int port;
    };
}

#endif // CONTINUOUS_TCP_SERVER_TEST_MANYCLIENTS_H_
