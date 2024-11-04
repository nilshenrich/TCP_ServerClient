#ifndef CONTINUOUS_TLS_SERVER_TEST_SEND_H_
#define CONTINUOUS_TLS_SERVER_TEST_SEND_H_

#include "TlsServerApi.h"
#include "TlsClientApi.h"

namespace Test
{
    class Continuous_TlsServer_Test_Send : public testing::Test
    {
    public:
        Continuous_TlsServer_Test_Send();
        virtual ~Continuous_TlsServer_Test_Send();

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

#endif // CONTINUOUS_TLS_SERVER_TEST_SEND_H_
