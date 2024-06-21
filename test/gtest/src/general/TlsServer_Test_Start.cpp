#include "general/TlsServer_Test_Start.h"

using namespace std;
using namespace Test;
using namespace tcp;

General_TlsServer_Test_Start::General_TlsServer_Test_Start() {}
General_TlsServer_Test_Start::~General_TlsServer_Test_Start() {}

void General_TlsServer_Test_Start::SetUp()
{
    // Get free TLS port
    port = HelperFunctions::getFreePort();
    ASSERT_NE(port, -1) << "No free port found";
    return;
}

void General_TlsServer_Test_Start::TearDown()
{
    // Stop TLS server
    tlsServer.stop();

    // Check if no pipe error occurred
    EXPECT_FALSE(HelperFunctions::getAndResetPipeError()) << "Pipe error occurred!";

    return;
}

// ====================================================================================================================
// Desc:       Check if the TLS server starts properly
// Steps:      Start TLS server with correct parameters
// Exp Result: SERVER_START_OK
// ====================================================================================================================
TEST_F(General_TlsServer_Test_Start, PosTest)
{
    EXPECT_EQ(tlsServer.start(port), SERVER_START_OK);
}

// ====================================================================================================================
// Desc:       Check if server doesn't accept negative port number
// Steps:      Try to start TLS server with -1
// Exp Result: SERVER_ERROR_START_WRONG_PORT
// ====================================================================================================================
TEST_F(General_TlsServer_Test_Start, NegTest_WrongPort_Negative)
{
    EXPECT_EQ(tlsServer.start(-1), SERVER_ERROR_START_WRONG_PORT);
}

// ====================================================================================================================
// Desc:       Check if server doesn't accept too big port number
// Steps:      Try to start TLS server with 65536
// Exp Result: SERVER_ERROR_START_WRONG_PORT
// ====================================================================================================================
TEST_F(General_TlsServer_Test_Start, NegTest_WrongPort_TooBig)
{
    EXPECT_EQ(tlsServer.start(65536), SERVER_ERROR_START_WRONG_PORT);
}

// ====================================================================================================================
// Desc:       Check if server doesn't start if it is already running having no active connections
// Steps:      Start TLS server and then try to start it again on free port
// Exp Result: -1
// ====================================================================================================================
TEST_F(General_TlsServer_Test_Start, NegTest_AlreadyRunning_NoConnections)
{
    ASSERT_EQ(tlsServer.start(port), SERVER_START_OK);
    EXPECT_EQ(tlsServer.start(HelperFunctions::getFreePort()), -1);
}

// ====================================================================================================================
// Desc:       Check if server doesn't start if it is already running having 1 active connection
// Steps:      Start TLS server, connect a client and then try to start it again on free port
// Exp Result: -1
// ====================================================================================================================
TEST_F(General_TlsServer_Test_Start, NegTest_AlreadyRunning_OneConnection)
{
    ASSERT_EQ(tlsServer.start(port), SERVER_START_OK);
    TestApi::TlsClientApi_fragmentation tlsClient{};
    ASSERT_EQ(tlsClient.start("localhost", port), CLIENT_START_OK);
    EXPECT_EQ(tlsServer.start(HelperFunctions::getFreePort()), -1);
}

// ====================================================================================================================
// Desc:       Check if server doesn't start if port is already in use
// Steps:      Start TLS server and try starting another instance with same port
// Exp Result: SERVER_ERROR_START_BIND_PORT
// ====================================================================================================================
TEST_F(General_TlsServer_Test_Start, NegTest_PortInUse)
{
    ASSERT_EQ(tlsServer.start(port), SERVER_START_OK);
    TestApi::TlsServerApi_fragmentation tlsServer2{};
    EXPECT_EQ(tlsServer2.start(port), SERVER_ERROR_START_BIND_PORT);
}

// ====================================================================================================================
// Desc:       Check if server doesn't start if path to CA cert is incorrect
// Steps:      Try to start TLS server with incorrect path to CA cert
// Exp Result: SERVER_ERROR_START_WRONG_CA_PATH
// ====================================================================================================================
TEST_F(General_TlsServer_Test_Start, NegTest_WrongCaPath)
{
    EXPECT_EQ(tlsServer.start(port, "fake/path/to/ca.crt", KeyPaths::ServerCert, KeyPaths::ServerKey), SERVER_ERROR_START_WRONG_CA_PATH);
}

// ====================================================================================================================
// Desc:       Check if server doesn't start if path to server cert is incorrect
// Steps:      Try to start TLS server with incorrect path to server cert
// Exp Result: SERVER_ERROR_START_WRONG_CERT_PATH
// ====================================================================================================================
TEST_F(General_TlsServer_Test_Start, NegTest_WrongCertPath)
{
    EXPECT_EQ(tlsServer.start(port, KeyPaths::CaCert, "fake/path/to/server.crt", KeyPaths::ServerKey), SERVER_ERROR_START_WRONG_CERT_PATH);
}

// ====================================================================================================================
// Desc:       Check if server doesn't start if path to server key is incorrect
// Steps:      Try to start TLS server with incorrect path to server key
// Exp Result: SERVER_ERROR_START_WRONG_KEY_PATH
// ====================================================================================================================
TEST_F(General_TlsServer_Test_Start, NegTest_WrongKeyPath)
{
    EXPECT_EQ(tlsServer.start(port, KeyPaths::CaCert, KeyPaths::ServerCert, "fake/path/to/server.key"), SERVER_ERROR_START_WRONG_KEY_PATH);
}

// ====================================================================================================================
// Desc:       Check if server doesn't start if CA cert is incorrect (wrong format)
// Steps:      Try to start TLS server with incorrect CA cert
// Exp Result: SERVER_ERROR_START_WRONG_CA
// ====================================================================================================================
TEST_F(General_TlsServer_Test_Start, NegTest_FakeCa)
{
    EXPECT_EQ(tlsServer.start(port, FakeKeyPaths::CaCert, KeyPaths::ServerCert, KeyPaths::ServerKey), SERVER_ERROR_START_WRONG_CA);
}

// ====================================================================================================================
// Desc:       Check if server doesn't start if server cert is incorrect (wrong format)
// Steps:      Try to start TLS server with incorrect server cert
// Exp Result: SERVER_ERROR_START_WRONG_CERT
// ====================================================================================================================
TEST_F(General_TlsServer_Test_Start, NegTest_FakeCert)
{
    EXPECT_EQ(tlsServer.start(port, KeyPaths::CaCert, FakeKeyPaths::ServerCert, KeyPaths::ServerKey), SERVER_ERROR_START_WRONG_CERT);
}

// ====================================================================================================================
// Desc:       Check if server doesn't start if server key is incorrect (wrong format)
// Steps:      Try to start TLS server with incorrect server key
// Exp Result: SERVER_ERROR_START_WRONG_KEY
// ====================================================================================================================
TEST_F(General_TlsServer_Test_Start, NegTest_FakeKey)
{
    EXPECT_EQ(tlsServer.start(port, KeyPaths::CaCert, KeyPaths::ServerCert, FakeKeyPaths::ServerKey), SERVER_ERROR_START_WRONG_KEY);
}

// ====================================================================================================================
// Desc:       Check if server doesn't start if server certificate and key don't match
// Steps:      Try to start TLS server with incorrect server cert and key
// Exp Result: SERVER_ERROR_START_WRONG_KEY
// ====================================================================================================================
TEST_F(General_TlsServer_Test_Start, NegTest_NotMatchingCertAndKey)
{
    EXPECT_EQ(tlsServer.start(port, KeyPaths::CaCert, KeyPaths::ServerCert, SelfSignedKeyPaths::ServerKey), SERVER_ERROR_START_WRONG_KEY);
}
