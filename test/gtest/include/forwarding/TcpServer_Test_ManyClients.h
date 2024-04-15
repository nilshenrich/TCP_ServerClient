#ifndef FORWARDING_TCP_SERVER_TEST_MANYCLIENTS_H_
#define FORWARDING_TCP_SERVER_TEST_MANYCLIENTS_H_

#include <gtest/gtest.h>
#include <thread>
#include <map>
#include <memory>
#include <string>

#include "TcpServerApi.h"
#include "TcpClientApi.h"
#include "HelperFunctions.h"

namespace Test
{
    class Forwarding_TcpServer_Test_ManyClients : public testing::Test
    {
    public:
        Forwarding_TcpServer_Test_ManyClients();
        ~Forwarding_TcpServer_Test_ManyClients();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TCP server and collection of clients
        TestApi::TcpServerApi_forwarding tcpServer;
        std::map<int, std::unique_ptr<TestApi::TcpClientApi_forwarding>> tcpClients;

        // Port to use
        int port;
    };
}

#endif // FORWARDING_TCP_SERVER_TEST_MANYCLIENTS_H_
