#ifndef TLSSERVER_TEST_READCLIENTDATA_H_
#define TLSSERVER_TEST_READCLIENTDATA_H_

#include <gtest/gtest.h>

#include "TlsServerApi.h"
#include "TlsClientApi.h"

namespace Test
{
    class General_TlsServer_Test_ReadClientData : public testing::Test
    {
    public:
        General_TlsServer_Test_ReadClientData();
        virtual ~General_TlsServer_Test_ReadClientData();

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

#endif // TLSSERVER_TEST_READCLIENTDATA_H_
