#ifndef FRAGMENTATION_TLS_SERVER_TEST_SEND_H_
#define FRAGMENTATION_TLS_SERVER_TEST_SEND_H_

#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <vector>
#include <string>

#include "TlsServerApi.h"
#include "TlsClientApi.h"
#include "HelperFunctions.h"

namespace Test
{
    class Fragmentation_TlsServer_Test_Send : public testing::Test
    {
    public:
        Fragmentation_TlsServer_Test_Send();
        virtual ~Fragmentation_TlsServer_Test_Send();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TLS server and Client
        TestApi::TlsServerApi_fragmentation tlsServer{};
        TestApi::TlsClientApi_fragmentation tlsClient{};

        // Port to use
        int port;

        // Client ID
        int clientId;
    };
}

#endif // FRAGMENTATION_TLS_SERVER_TEST_SEND_H_
