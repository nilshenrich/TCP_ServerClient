#ifndef TCPSERVER_TEST_READCLIENTDATA_H_
#define TCPSERVER_TEST_READCLIENTDATA_H_

#include <gtest/gtest.h>

#include "TcpServerApi.h"
#include "TcpClientApi.h"

namespace Test
{
    class General_TcpServer_Test_ReadClientData : public testing::Test
    {
    public:
        General_TcpServer_Test_ReadClientData();
        virtual ~General_TcpServer_Test_ReadClientData();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TCP Server and Client
        TestApi::TcpServerApi_fragmentation tcpServer{};
        TestApi::TcpClientApi_fragmentation tcpClient{};

        // Port to use
        int port;

        // Client ID
        int clientId;
    };
}

#endif // TCPSERVER_TEST_READCLIENTDATA_H_
