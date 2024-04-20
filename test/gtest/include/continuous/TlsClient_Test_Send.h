#ifndef CONTINUOUS_TLS_CLIENT_TEST_SEND_H_
#define CONTINUOUS_TLS_CLIENT_TEST_SEND_H_

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
    class Continuous_TlsClient_Test_Send : public testing::Test
    {
    public:
        Continuous_TlsClient_Test_Send();
        virtual ~Continuous_TlsClient_Test_Send();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TLS server and Client
        TestApi::TlsServerApi_continuous tlsServer{};
        TestApi::TlsClientApi_continuous tlsClient{};

        // Port to use
        int port;
    };
}

#endif // CONTINUOUS_TLS_CLIENT_TEST_SEND_H_
