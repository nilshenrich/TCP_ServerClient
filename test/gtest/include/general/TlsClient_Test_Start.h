#ifndef GENERAL_TLS_CLIENT_TEST_START_H_
#define GENERAL_TLS_CLIENT_TEST_START_H_

#include <gtest/gtest.h>
#include <chrono>
#include <thread>

#include "TlsServerApi.h"
#include "TlsClientApi.h"
#include "HelperFunctions.h"

namespace Test
{
    class General_TlsClient_Test_Start : public testing::Test
    {
    public:
        General_TlsClient_Test_Start();
        virtual ~General_TlsClient_Test_Start();

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

#endif // GENERAL_TLS_CLIENT_TEST_START_H_
