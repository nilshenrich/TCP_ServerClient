#ifndef FORWARDING_TLS_CONNECTION_TEST_MSGTRANSFER_H_
#define FORWARDING_TLS_CONNECTION_TEST_MSGTRANSFER_H_

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
    class Forwarding_TlsConnection_Test_MsgTransfer : public testing::Test
    {
    public:
        Forwarding_TlsConnection_Test_MsgTransfer();
        virtual ~Forwarding_TlsConnection_Test_MsgTransfer();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TLS server and Client
        TestApi::TlsServerApi_forwarding tlsServer{};
        TestApi::TlsClientApi_forwarding tlsClient{};

        // Port to use
        int port;

        // Client ID
        int clientId;
    };
}

#endif // FORWARDING_TLS_CONNECTION_TEST_MSGTRANSFER_H_
