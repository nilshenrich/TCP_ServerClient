#ifndef GENERAL_TLS_CONNECTION_TEST_INCOMPLETECERT_H_
#define GENERAL_TLS_CONNECTION_TEST_INCOMPLETECERT_H_

#include <gtest/gtest.h>

#include "TlsServerApi.h"
#include "TlsClientApi.h"

namespace Test
{
    class General_TlsConnection_Test_IncompleteCert : public testing::Test
    {
    public:
        General_TlsConnection_Test_IncompleteCert();
        virtual ~General_TlsConnection_Test_IncompleteCert();

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

#endif // GENERAL_TLS_CONNECTION_TEST_INCOMPLETECERT_H_
