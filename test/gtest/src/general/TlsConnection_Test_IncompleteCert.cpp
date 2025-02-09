#include <chrono>
#include <thread>

#include "general/TlsConnection_Test_IncompleteCert.h"
#include "HelperFunctions.h"

using namespace std;
using namespace Test;
using namespace tcp;

General_TlsConnection_Test_IncompleteCert::General_TlsConnection_Test_IncompleteCert() {}
General_TlsConnection_Test_IncompleteCert::~General_TlsConnection_Test_IncompleteCert() {}

void General_TlsConnection_Test_IncompleteCert::SetUp()
{
    // Get free TLS port
    port = HelperFunctions::getFreePort();
    ASSERT_NE(port, -1) << "No free port found";
    return;
}

void General_TlsConnection_Test_IncompleteCert::TearDown()
{
    // Stop TLS server and client
    tlsClient.stop();
    tlsServer.stop();

    // Check if no pipe error occurred
    EXPECT_FALSE(HelperFunctions::getAndResetPipeError()) << "Pipe error occurred!";

    return;
}

// ====================================================================================================================
// Desc:       All server certificates given, but no client certificates given
// Steps:      Connect to server without client certificate:
//             Server: CA: valid,   Cert: valid,    Key: valid,     Require client authentication: false
//             Client: CA: empty,   Cert: empty,    Key: empty,     Require server authentication: false (Not possible without CA)
// Exp Result: SERVER_START_OK, CLIENT_START_OK
// ====================================================================================================================
TEST_F(General_TlsConnection_Test_IncompleteCert, PosTest_ServerAll_ClientNone_ff)
{
    EXPECT_EQ(tlsServer.start(port, KeyPaths::CaCert, KeyPaths::ServerCert, KeyPaths::ServerKey, false), SERVER_START_OK);
    EXPECT_EQ(tlsClient.start("localhost", port, "", "", "", false), CLIENT_START_OK);
    EXPECT_EQ(tlsServer.getClientIds().size(), 1);
}

// ====================================================================================================================
// Desc:       All server certificates given, but no client certificates given
// Steps:      Connect to server without client certificate:
//             Server: CA: valid,   Cert: valid,    Key: valid,     Require client authentication: false
//             Client: CA: empty,   Cert: empty,    Key: empty,     Require server authentication: true (Should be treated as false)
// Exp Result: SERVER_START_OK, CLIENT_START_OK
// ====================================================================================================================
TEST_F(General_TlsConnection_Test_IncompleteCert, PosTest_ServerAll_ClientNone_ft)
{
    EXPECT_EQ(tlsServer.start(port, KeyPaths::CaCert, KeyPaths::ServerCert, KeyPaths::ServerKey, false), SERVER_START_OK);
    EXPECT_EQ(tlsClient.start("localhost", port, "", "", "", true), CLIENT_START_OK);
    EXPECT_EQ(tlsServer.getClientIds().size(), 1);
}

// ====================================================================================================================
// Desc:       All server certificates given, but no client certificates given (only CA)
// Steps:      Connect to server without client certificate:
//             Server: CA: valid,   Cert: valid,    Key: valid,     Require client authentication: false
//             Client: CA: valid,   Cert: empty,    Key: empty,     Require server authentication: false
// Exp Result: SERVER_START_OK, CLIENT_START_OK
// ====================================================================================================================
TEST_F(General_TlsConnection_Test_IncompleteCert, PosTest_ServerAll_ClientCA_ff)
{
    EXPECT_EQ(tlsServer.start(port, KeyPaths::CaCert, KeyPaths::ServerCert, KeyPaths::ServerKey, false), SERVER_START_OK);
    EXPECT_EQ(tlsClient.start("localhost", port, KeyPaths::CaCert, "", "", false), CLIENT_START_OK);
    EXPECT_EQ(tlsServer.getClientIds().size(), 1);
}

// ====================================================================================================================
// Desc:       All server certificates given, but no client certificates given (only CA)
// Steps:      Connect to server without client certificate:
//             Server: CA: valid,   Cert: valid,    Key: valid,     Require client authentication: false
//             Client: CA: valid,   Cert: empty,    Key: empty,     Require server authentication: true
// Exp Result: SERVER_START_OK, CLIENT_START_OK
// ====================================================================================================================
TEST_F(General_TlsConnection_Test_IncompleteCert, PosTest_ServerAll_ClientCA_ft)
{
    EXPECT_EQ(tlsServer.start(port, KeyPaths::CaCert, KeyPaths::ServerCert, KeyPaths::ServerKey, false), SERVER_START_OK);
    EXPECT_EQ(tlsClient.start("localhost", port, KeyPaths::CaCert, "", "", true), CLIENT_START_OK);
    EXPECT_EQ(tlsServer.getClientIds().size(), 1);
}

// ====================================================================================================================
// Desc:       No server certificates given, but all client certificates given
// Steps:      Connect to server without server certificate:
//             Server: CA: empty,   Cert: self,     Key: self,      Require client authentication: false (Not possible without CA)
//             Client: CA: valid,   Cert: valid,    Key: valid,     Require server authentication: false
// Exp Result: SERVER_START_OK, CLIENT_START_OK
// ====================================================================================================================
TEST_F(General_TlsConnection_Test_IncompleteCert, PosTest_ServerNone_ClientAll_ff)
{
    EXPECT_EQ(tlsServer.start(port, "", SelfSignedKeyPaths::ServerCert, SelfSignedKeyPaths::ServerKey, false), SERVER_START_OK);
    EXPECT_EQ(tlsClient.start("localhost", port, KeyPaths::CaCert, KeyPaths::ClientCert, KeyPaths::ClientKey, false), CLIENT_START_OK);
    EXPECT_EQ(tlsServer.getClientIds().size(), 1);
}

// ====================================================================================================================
// Desc:       No server certificates given, but all client certificates given
// Steps:      Connect to server without server certificate:
//             Server: CA: empty,   Cert: self,     Key: self,      Require client authentication: true (Should be treated as false)
//             Client: CA: valid,   Cert: valid,    Key: valid,     Require server authentication: false
// Exp Result: SERVER_START_OK, CLIENT_START_OK
// ====================================================================================================================
TEST_F(General_TlsConnection_Test_IncompleteCert, PosTest_ServerNone_ClientAll_tf)
{
    EXPECT_EQ(tlsServer.start(port, "", SelfSignedKeyPaths::ServerCert, SelfSignedKeyPaths::ServerKey, true), SERVER_START_OK);
    EXPECT_EQ(tlsClient.start("localhost", port, KeyPaths::CaCert, KeyPaths::ClientCert, KeyPaths::ClientKey, false), CLIENT_START_OK);
    EXPECT_EQ(tlsServer.getClientIds().size(), 1);
}

// ====================================================================================================================
// Desc:       No server certificates given, but all client certificates given (only CA)
// Steps:      Connect to server without server certificate:
//             Server: CA: valid,   Cert: self,     Key: self,      Require client authentication: false
//             Client: CA: valid,   Cert: valid,    Key: valid,     Require server authentication: false
// Exp Result: SERVER_START_OK, CLIENT_START_OK
// ====================================================================================================================
TEST_F(General_TlsConnection_Test_IncompleteCert, PosTest_ServerCA_ClientAll_ff)
{
    EXPECT_EQ(tlsServer.start(port, KeyPaths::CaCert, SelfSignedKeyPaths::ServerCert, SelfSignedKeyPaths::ServerKey, false), SERVER_START_OK);
    EXPECT_EQ(tlsClient.start("localhost", port, KeyPaths::CaCert, KeyPaths::ClientCert, KeyPaths::ClientKey, false), CLIENT_START_OK);
    EXPECT_EQ(tlsServer.getClientIds().size(), 1);
}

// ====================================================================================================================
// Desc:       No server certificates given, but all client certificates given (only CA)
// Steps:      Connect to server without server certificate:
//             Server: CA: valid,   Cert: self,     Key: self,      Require client authentication: true
//             Client: CA: valid,   Cert: valid,    Key: valid,     Require server authentication: false
// Exp Result: SERVER_START_OK, CLIENT_START_OK
// ====================================================================================================================
TEST_F(General_TlsConnection_Test_IncompleteCert, PosTest_ServerCA_ClientAll_tf)
{
    EXPECT_EQ(tlsServer.start(port, KeyPaths::CaCert, SelfSignedKeyPaths::ServerCert, SelfSignedKeyPaths::ServerKey, true), SERVER_START_OK);
    EXPECT_EQ(tlsClient.start("localhost", port, KeyPaths::CaCert, KeyPaths::ClientCert, KeyPaths::ClientKey, false), CLIENT_START_OK);
    EXPECT_EQ(tlsServer.getClientIds().size(), 1);
}

// ====================================================================================================================
// Desc:       No server certificates given, no client certificates given
// Steps:      Connect to server without server and client certificates:
//             Server: CA: empty,   Cert: self,     Key: self,      Require client authentication: false (Not possible without CA)
//             Client: CA: empty,   Cert: empty,    Key: empty,     Require server authentication: false (Not possible without CA)
// Exp Result: SERVER_START_OK, CLIENT_START_OK
// ====================================================================================================================
TEST_F(General_TlsConnection_Test_IncompleteCert, PosTest_ServerNone_ClientNone_ff)
{
    EXPECT_EQ(tlsServer.start(port, "", SelfSignedKeyPaths::ServerCert, SelfSignedKeyPaths::ServerKey, false), SERVER_START_OK);
    EXPECT_EQ(tlsClient.start("localhost", port, "", "", "", false), CLIENT_START_OK);
    EXPECT_EQ(tlsServer.getClientIds().size(), 1);
}

// ====================================================================================================================
// Desc:       No server certificates given, no client certificates given
// Steps:      Connect to server without server and client certificates:
//             Server: CA: empty,   Cert: self,     Key: self,      Require client authentication: true (Should be treated as false)
//             Client: CA: empty,   Cert: empty,    Key: empty,     Require server authentication: true (Should be treated as false)
// Exp Result: SERVER_START_OK, CLIENT_START_OK
// ====================================================================================================================
TEST_F(General_TlsConnection_Test_IncompleteCert, PosTest_ServerNone_ClientNone_tt)
{
    EXPECT_EQ(tlsServer.start(port, "", SelfSignedKeyPaths::ServerCert, SelfSignedKeyPaths::ServerKey, true), SERVER_START_OK);
    EXPECT_EQ(tlsClient.start("localhost", port, "", "", "", true), CLIENT_START_OK);
    EXPECT_EQ(tlsServer.getClientIds().size(), 1);
}
