#ifndef FRAGMENTATION_TLS_CLIENT_TEST_SEND_H_
#define FRAGMENTATION_TLS_CLIENT_TEST_SEND_H_

#include <gtest/gtest.h>

#include "TlsServerApi.h"
#include "TlsClientApi.h"

namespace Test
{
    class Fragmentation_TlsClient_Test_Send : public testing::Test
    {
    public:
        Fragmentation_TlsClient_Test_Send();
        virtual ~Fragmentation_TlsClient_Test_Send();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TLS server and Client
        TestApi::TlsServerApi_fragmentation tlsServer{};
        TestApi::TlsClientApi_fragmentation tlsClient{};

        // Port to use
        int port;
    };
}

#endif // FRAGMENTATION_TLS_CLIENT_TEST_SEND_H_
