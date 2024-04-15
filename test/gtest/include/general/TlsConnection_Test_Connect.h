#ifndef GENERAL_TLS_CONNECTION_TEST_CONNECT_H_
#define GENERAL_TLS_CONNECTION_TEST_CONNECT_H_

#include <gtest/gtest.h>
#include <chrono>
#include <thread>

#include "TlsServerApi.h"
#include "TlsClientApi.h"
#include "HelperFunctions.h"

namespace Test
{
    class General_TlsConnection_Test_Connect : public testing::Test
    {
    public:
        General_TlsConnection_Test_Connect();
        virtual ~General_TlsConnection_Test_Connect();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TCP server and Client
        TestApi::TlsServerApi_fragmentation tlsServer{};
        TestApi::TlsClientApi_fragmentation tlsClient{};

        // Port to use
        int port;
    };
}

#endif // GENERAL_TLS_CONNECTION_TEST_CONNECT_H_
