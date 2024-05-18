#ifndef FRAGMENTATION_TCP_SERVER_TEST_MANYCLIENTS_H_
#define FRAGMENTATION_TCP_SERVER_TEST_MANYCLIENTS_H_

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
    class Fragmentation_TcpServer_Test_ManyClients : public testing::Test
    {
    public:
        Fragmentation_TcpServer_Test_ManyClients();
        ~Fragmentation_TcpServer_Test_ManyClients();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TCP server and collection of clients
        TestApi::TcpServerApi_fragmentation tcpServer;
        ::std::map<int, ::std::unique_ptr<TestApi::TcpClientApi_fragmentation>> tcpClients;

        // Port to use
        int port;
    };
}

#endif // FRAGMENTATION_TCP_SERVER_TEST_MANYCLIENTS_H_
