#ifndef TLSCLIENT_TEST_READSERVERDATA_H_
#define TLSCLIENT_TEST_READSERVERDATA_H_

#include <gtest/gtest.h>

#include "TlsServerApi.h"
#include "TlsClientApi.h"

namespace Test
{
    class General_TlsClient_Test_ReadServerData : public testing::Test
    {
    public:
        General_TlsClient_Test_ReadServerData();
        virtual ~General_TlsClient_Test_ReadServerData();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TLS Server and Client
        TestApi::TlsServerApi_fragmentation tlsServer{};
        TestApi::TlsClientApi_fragmentation tlsClient{};

        // Port to use
        int port;

        // Client ID
        int clientId;
    };
}

#endif // TLSCLIENT_TEST_READSERVERDATA_H_
