#ifndef GENERAL_TLS_SERVER_TEST_START_H_
#define GENERAL_TLS_SERVER_TEST_START_H_

#include <gtest/gtest.h>

#include "TlsServerApi.h"
#include "TlsClientApi.h"

namespace Test
{
    class General_TlsServer_Test_Start : public testing::Test
    {
    public:
        General_TlsServer_Test_Start();
        virtual ~General_TlsServer_Test_Start();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TLS Server
        TestApi::TlsServerApi_fragmentation tlsServer{};

        // Port to use
        int port;
    };
} // namespace Test

#endif // GENERAL_TLS_SERVER_TEST_START_H_
