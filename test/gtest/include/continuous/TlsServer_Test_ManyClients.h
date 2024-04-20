#ifndef CONTINUOUS_TLS_SERVER_TEST_MANYCLIENTS_H_
#define CONTINUOUS_TLS_SERVER_TEST_MANYCLIENTS_H_

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
    class Continuous_TlsServer_Test_ManyClients : public testing::Test
    {
    public:
        Continuous_TlsServer_Test_ManyClients();
        ~Continuous_TlsServer_Test_ManyClients();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TLS server and collection of clients
        TestApi::TlsServerApi_continuous tlsServer;
        std::map<int, std::unique_ptr<TestApi::TlsClientApi_continuous>> tlsClients;

        // Port to use
        int port;
    };
}

#endif // CONTINUOUS_TLS_SERVER_TEST_MANYCLIENTS_H_
