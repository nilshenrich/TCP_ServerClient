#ifndef FRAGMENTATION_TLS_CONNECTION_TEST_MSGAPPEND_H_
#define FRAGMENTATION_TLS_CONNECTION_TEST_MSGAPPEND_H_

#include <gtest/gtest.h>

#include "TlsServerApi.h"
#include "TlsClientApi.h"

namespace Test
{

    class Fragmentation_TlsConnection_Test_MsgAppend : public testing::Test
    {
    public:
        Fragmentation_TlsConnection_Test_MsgAppend();
        ~Fragmentation_TlsConnection_Test_MsgAppend();

    protected:
        void SetUp() override;
        void TearDown() override;

        // TLS server and Client
        ::std::string messageAppendServer{"Appended message from server"};
        ::std::string messageAppendClient{"Appended message from client"};
        TestApi::TlsServerApi_fragmentation tlsServer{messageAppendServer};
        TestApi::TlsClientApi_fragmentation tlsClient{messageAppendClient};

        // Port to use
        int port;

        // Client ID
        int clientId;
    };
}

#endif // FRAGMENTATION_TLS_CONNECTION_TEST_MSGAPPEND_H_
