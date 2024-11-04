#ifndef FRAGMENTATION_TLS_SERVER_TEST_MANYCLIENTS_H_
#define FRAGMENTATION_TLS_SERVER_TEST_MANYCLIENTS_H_

#include "TlsServerApi.h"
#include "TlsClientApi.h"

namespace Test
{
    class Fragmentation_TlsServer_Test_ManyClients : public testing::Test
    {
    public:
        Fragmentation_TlsServer_Test_ManyClients();
        ~Fragmentation_TlsServer_Test_ManyClients();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TLS server and collection of clients
        TestApi::TlsServerApi_fragmentation tlsServer;
        ::std::map<int, ::std::unique_ptr<TestApi::TlsClientApi_fragmentation>> tlsClients;

        // Port to use
        int port;
    };
}

#endif // FRAGMENTATION_TLS_SERVER_TEST_MANYCLIENTS_H_
