#ifndef FORWARDING_TLS_SERVER_TEST_MANYCLIENTS_H_
#define FORWARDING_TLS_SERVER_TEST_MANYCLIENTS_H_

#include <gtest/gtest.h>
#include <thread>
#include <map>
#include <memory>
#include <string>

#include "TlsServerApi.h"
#include "TlsClientApi.h"
#include "HelperFunctions.h"

namespace Test
{
    class Forwarding_TlsServer_Test_ManyClients : public testing::Test
    {
    public:
        Forwarding_TlsServer_Test_ManyClients();
        ~Forwarding_TlsServer_Test_ManyClients();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TLS server and collection of clients
        TestApi::TlsServerApi_forwarding tlsServer;
        std::map<int, std::unique_ptr<TestApi::TlsClientApi_forwarding>> tlsClients;

        // Port to use
        int port;
    };
}

#endif // FORWARDING_TLS_SERVER_TEST_MANYCLIENTS_H_
