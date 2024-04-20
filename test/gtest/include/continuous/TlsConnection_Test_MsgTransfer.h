#ifndef CONTINUOUS_TLS_CONNECTION_TEST_MSGTRANSFER_H_
#define CONTINUOUS_TLS_CONNECTION_TEST_MSGTRANSFER_H_

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
    class Continuous_TlsConnection_Test_MsgTransfer : public testing::Test
    {
    public:
        Continuous_TlsConnection_Test_MsgTransfer();
        virtual ~Continuous_TlsConnection_Test_MsgTransfer();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TLS server and Client
        TestApi::TlsServerApi_continuous tlsServer{};
        TestApi::TlsClientApi_continuous tlsClient{};

        // Port to use
        int port;

        // Client ID
        int clientId;
    };
}

#endif // CONTINUOUS_TLS_CONNECTION_TEST_MSGTRANSFER_H_
