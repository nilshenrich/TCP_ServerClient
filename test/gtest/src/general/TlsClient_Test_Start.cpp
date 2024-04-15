#include "general/TlsClient_Test_Start.h"

using namespace std;
using namespace Test;
using namespace networking;

General_TlsClient_Test_Start::General_TlsClient_Test_Start() {}
General_TlsClient_Test_Start::~General_TlsClient_Test_Start() {}

void General_TlsClient_Test_Start::SetUp()
{
    // Get free TLS port
    port = HelperFunctions::getFreePort();
    ASSERT_NE(port, -1) << "No free port found";
    ASSERT_EQ(tlsServer.start(port), NETWORKLISTENER_START_OK) << "Unable to start TLS server on port " << port;
    return;
}

void General_TlsClient_Test_Start::TearDown()
{
    // Stop TLS server and client
    tlsClient.stop();
    tlsServer.stop();

    // Check if no pipe error occurred
    EXPECT_FALSE(HelperFunctions::getAndResetPipeError()) << "Pipe error occurred!";

    return;
}

// ====================================================================================================================
// Desc:       Check if the TLS client starts properly
// Steps:      Start TLS client with correct parameters
// Exp Result: NETWORKCLIENT_START_OK
// ====================================================================================================================
TEST_F(General_TlsClient_Test_Start, PosTest)
{
    EXPECT_EQ(tlsClient.start("localhost", port), NETWORKCLIENT_START_OK);
    EXPECT_EQ(tlsServer.getClientIds().size(), 1);
}

// ====================================================================================================================
// Desc:       Check if client doesn't accept negative port number
// Steps:      Try to start TLS client with -1
// Exp Result: NETWORKCLIENT_ERROR_START_WRONG_PORT
// ====================================================================================================================
TEST_F(General_TlsClient_Test_Start, NegTest_WrongPort_Negative)
{
    EXPECT_EQ(tlsClient.start("localhost", -1), NETWORKCLIENT_ERROR_START_WRONG_PORT);
    EXPECT_EQ(tlsServer.getClientIds().size(), 0);
}

// ====================================================================================================================
// Desc:       Check if client doesn't accept too big port number
// Steps:      Try to start TLS client with 65536
// Exp Result: NETWORKCLIENT_ERROR_START_WRONG_PORT
// ====================================================================================================================
TEST_F(General_TlsClient_Test_Start, NegTest_WrongPort_TooBig)
{
    EXPECT_EQ(tlsClient.start("localhost", 65536), NETWORKCLIENT_ERROR_START_WRONG_PORT);
    EXPECT_EQ(tlsServer.getClientIds().size(), 0);
}

// ====================================================================================================================
// Desc:       Check if client stops properly when server is not running
// Steps:      Try to start TLS client after stopping server
// Exp Result: NETWORKCLIENT_ERROR_START_CONNECT
// ====================================================================================================================
TEST_F(General_TlsClient_Test_Start, NegTest_ServerNotRunning)
{
    tlsServer.stop();
    EXPECT_EQ(tlsClient.start("localhost", port), NETWORKCLIENT_ERROR_START_CONNECT);
    EXPECT_EQ(tlsServer.getClientIds().size(), 0);
}

// ====================================================================================================================
// Desc:       Check if client doesn't start if it is already running
// Steps:      Try to start TLS client and then try to start it again
// Exp Result: -1
// ====================================================================================================================
TEST_F(General_TlsClient_Test_Start, NegTest_AlreadyRunning)
{
    EXPECT_EQ(tlsClient.start("localhost", port), NETWORKCLIENT_START_OK);
    EXPECT_EQ(tlsClient.start("localhost", port), -1);
    EXPECT_EQ(tlsServer.getClientIds().size(), 1);
}

// ====================================================================================================================
// Desc:       Check if client doesn't start if path to CA certificate is wrong
// Steps:      Try to start TLS client with wrong path to CA certificate
// Exp Result: NETWORKCLIENT_ERROR_START_WRONG_CA_PATH
// ====================================================================================================================
TEST_F(General_TlsClient_Test_Start, NegTest_WrongCaPath)
{
    EXPECT_EQ(tlsClient.start("localhost", port, "fake/path/to/ca.crt", KeyPaths::ClientCert, KeyPaths::ClientKey), NETWORKCLIENT_ERROR_START_WRONG_CA_PATH);
    EXPECT_EQ(tlsServer.getClientIds().size(), 0);
}

// ====================================================================================================================
// Desc:       Check if client doesn't start if path to client certificate is wrong
// Steps:      Try to start TLS client with wrong path to client certificate
// Exp Result: NETWORKCLIENT_ERROR_START_WRONG_CERT_PATH
// ====================================================================================================================
TEST_F(General_TlsClient_Test_Start, NegTest_WrongCertPath)
{
    EXPECT_EQ(tlsClient.start("localhost", port, KeyPaths::CaCert, "fake/path/to/client.crt", KeyPaths::ListenerKey), NETWORKCLIENT_ERROR_START_WRONG_CERT_PATH);
    EXPECT_EQ(tlsServer.getClientIds().size(), 0);
}

// ====================================================================================================================
// Desc:       Check if client doesn't start if path to client key is wrong
// Steps:      Try to start TLS client with wrong path to client key
// Exp Result: NETWORKCLIENT_ERROR_START_WRONG_KEY_PATH
// ====================================================================================================================
TEST_F(General_TlsClient_Test_Start, NegTest_WrongKeyPath)
{
    EXPECT_EQ(tlsClient.start("localhost", port, KeyPaths::CaCert, KeyPaths::ClientCert, "fake/path/to/client.key"), NETWORKCLIENT_ERROR_START_WRONG_KEY_PATH);
    EXPECT_EQ(tlsServer.getClientIds().size(), 0);
}

// ====================================================================================================================
// Desc:       Check if client doesn't start if CA certificate is incorrect (wrong format)
// Steps:      Try to start TLS client with incorrect CA certificate
// Exp Result: NETWORKCLIENT_ERROR_START_WRONG_CA
// ====================================================================================================================
TEST_F(General_TlsClient_Test_Start, NegTest_FakeCa)
{
    EXPECT_EQ(tlsClient.start("localhost", port, FakeKeyPaths::CaCert, KeyPaths::ClientCert, KeyPaths::ClientKey), NETWORKCLIENT_ERROR_START_WRONG_CA);
    EXPECT_EQ(tlsServer.getClientIds().size(), 0);
}

// ====================================================================================================================
// Desc:       Check if client doesn't start if client certificate is incorrect (wrong format)
// Steps:      Try to start TLS client with incorrect client certificate
// Exp Result: NETWORKCLIENT_ERROR_START_WRONG_CERT
// ====================================================================================================================
TEST_F(General_TlsClient_Test_Start, NegTest_FakeCert)
{
    EXPECT_EQ(tlsClient.start("localhost", port, KeyPaths::CaCert, FakeKeyPaths::ClientCert, KeyPaths::ClientKey), NETWORKCLIENT_ERROR_START_WRONG_CERT);
    EXPECT_EQ(tlsServer.getClientIds().size(), 0);
}

// ====================================================================================================================
// Desc:       Check if client doesn't start if client key is incorrect (wrong format)
// Steps:      Try to start TLS client with incorrect client key
// Exp Result: NETWORKCLIENT_ERROR_START_WRONG_KEY
// ====================================================================================================================
TEST_F(General_TlsClient_Test_Start, NegTest_FakeKey)
{
    EXPECT_EQ(tlsClient.start("localhost", port, KeyPaths::CaCert, KeyPaths::ClientCert, FakeKeyPaths::ClientKey), NETWORKCLIENT_ERROR_START_WRONG_KEY);
    EXPECT_EQ(tlsServer.getClientIds().size(), 0);
}
