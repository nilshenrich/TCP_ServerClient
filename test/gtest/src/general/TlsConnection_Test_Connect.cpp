#include <chrono>
#include <thread>

#include "general/TlsConnection_Test_Connect.h"
#include "HelperFunctions.h"

using namespace std;
using namespace Test;
using namespace tcp;

General_TlsConnection_Test_Connect::General_TlsConnection_Test_Connect() {}
General_TlsConnection_Test_Connect::~General_TlsConnection_Test_Connect() {}

void General_TlsConnection_Test_Connect::SetUp()
{
    // Get free TLS port
    port = HelperFunctions::getFreePort();
    ASSERT_NE(port, -1) << "No free port found";
    return;
}

void General_TlsConnection_Test_Connect::TearDown()
{
    // Stop TLS server and client
    tlsClient.stop();
    tlsServer.stop();

    // Check if no pipe error occurred
    EXPECT_FALSE(HelperFunctions::getAndResetPipeError()) << "Pipe error occurred!";

    return;
}

// ====================================================================================================================
// Desc:       Check if the TLS client connects to the server properly with good certificates
// Steps:      Connect to server with good certificates
// Exp Result: SERVER_START_OK, CLIENT_START_OK
// ====================================================================================================================
TEST_F(General_TlsConnection_Test_Connect, PosTest_MainCerts)
{
    EXPECT_EQ(tlsServer.start(port, KeyPaths::CaCert, KeyPaths::ServerCert, KeyPaths::ServerKey), SERVER_START_OK);
    EXPECT_EQ(tlsClient.start("localhost", port, KeyPaths::CaCert, KeyPaths::ClientCert, KeyPaths::ClientKey), CLIENT_START_OK);
    EXPECT_EQ(tlsServer.getClientIds().size(), 1);
}

// ====================================================================================================================
// Desc:       Check if the TLS client connects to the server properly with second certificates
// Steps:      Connect to server with second certificates
// Exp Result: SERVER_START_OK, CLIENT_START_OK
// ====================================================================================================================
TEST_F(General_TlsConnection_Test_Connect, PosTest_SecondCerts)
{
    EXPECT_EQ(tlsServer.start(port, SecondKeyPaths::CaCert, SecondKeyPaths::ServerCert, SecondKeyPaths::ServerKey), SERVER_START_OK);
    EXPECT_EQ(tlsClient.start("localhost", port, SecondKeyPaths::CaCert, SecondKeyPaths::ClientCert, SecondKeyPaths::ClientKey), CLIENT_START_OK);
    EXPECT_EQ(tlsServer.getClientIds().size(), 1);
}

// ====================================================================================================================
// Desc:       Check if the TLS server rejects connections with self signed certificates
// Steps:      Connect to server a with self signed certificate
// Exp Result: SERVER_START_OK, CLIENT_START_OK
// ====================================================================================================================
TEST_F(General_TlsConnection_Test_Connect, NegTest_ClientWithSelfSignedCert)
{
    EXPECT_EQ(tlsServer.start(port, KeyPaths::CaCert, KeyPaths::ServerCert, KeyPaths::ServerKey), SERVER_START_OK);
    EXPECT_EQ(tlsClient.start("localhost", port, KeyPaths::CaCert, SelfSignedKeyPaths::ClientCert, SelfSignedKeyPaths::ClientKey), CLIENT_START_OK);
    EXPECT_EQ(tlsServer.getClientIds().size(), 0);
}

// ====================================================================================================================
// Desc:       Check if the TLS server rejects connections with certificates signed by unknown CA
// Steps:      Connect to server a with certificates signed by unknown CA
// Exp Result: SERVER_START_OK, CLIENT_START_OK
// ====================================================================================================================
TEST_F(General_TlsConnection_Test_Connect, NegTest_ClientWithUnknownCa)
{
    EXPECT_EQ(tlsServer.start(port, KeyPaths::CaCert, KeyPaths::ServerCert, KeyPaths::ServerKey), SERVER_START_OK);
    EXPECT_EQ(tlsClient.start("localhost", port, KeyPaths::CaCert, SecondKeyPaths::ClientCert, SecondKeyPaths::ClientKey), CLIENT_START_OK);
    EXPECT_EQ(tlsServer.getClientIds().size(), 0);
}

// ====================================================================================================================
// Desc:       Check if the TLS server rejects connections with certificates with a certificate chain depth of 2
// Steps:      Connect to server a with certificates with a certificate chain depth of 2
// Exp Result: SERVER_START_OK, CLIENT_START_OK
// ====================================================================================================================
TEST_F(General_TlsConnection_Test_Connect, NegTest_ClientWithCertChainDepth2)
{
    EXPECT_EQ(tlsServer.start(port, KeyPaths::CaCert, KeyPaths::ServerCert, KeyPaths::ServerKey), SERVER_START_OK);
    EXPECT_EQ(tlsClient.start("localhost", port, KeyPaths::CaCert, CertChainDepth2KeyPaths::ClientCert, CertChainDepth2KeyPaths::ClientKey), CLIENT_START_OK);
    EXPECT_EQ(tlsServer.getClientIds().size(), 0);
}

// ====================================================================================================================
// Desc:       Check if the TLS client rejects connections with self signed certificates
// Steps:      Try to connect to server that uses self signed certificates
// Exp Result: SERVER_START_OK, CLIENT_ERROR_START_CONNECT_INIT
// ====================================================================================================================
TEST_F(General_TlsConnection_Test_Connect, NegTest_ServerWithSelfSignedCert)
{
    EXPECT_EQ(tlsServer.start(port, KeyPaths::CaCert, SelfSignedKeyPaths::ServerCert, SelfSignedKeyPaths::ServerKey), SERVER_START_OK);
    EXPECT_EQ(tlsClient.start("localhost", port, KeyPaths::CaCert, KeyPaths::ClientCert, KeyPaths::ClientKey), CLIENT_ERROR_START_CONNECT_INIT);
    EXPECT_EQ(tlsServer.getClientIds().size(), 0);
}

// ====================================================================================================================
// Desc:       Check if the TLS client rejects connections with certificates signed by unknown CA
// Steps:      Try to connect to server that uses certificates signed by unknown CA
// Exp Result: SERVER_START_OK, CLIENT_ERROR_START_CONNECT_INIT
// ====================================================================================================================
TEST_F(General_TlsConnection_Test_Connect, NegTest_ServerWithUnknownCa)
{
    EXPECT_EQ(tlsServer.start(port, KeyPaths::CaCert, SecondKeyPaths::ServerCert, SecondKeyPaths::ServerKey), SERVER_START_OK);
    EXPECT_EQ(tlsClient.start("localhost", port, KeyPaths::CaCert, KeyPaths::ClientCert, KeyPaths::ClientKey), CLIENT_ERROR_START_CONNECT_INIT);
    EXPECT_EQ(tlsServer.getClientIds().size(), 0);
}

// ====================================================================================================================
// Desc:       Check if the TLS client rejects connections with certificates with a certificate chain depth of 2
// Steps:      Try to connect to server that uses certificates with a certificate chain depth of 2
// Exp Result: SERVER_START_OK, CLIENT_ERROR_START_CONNECT_INIT
// ====================================================================================================================
TEST_F(General_TlsConnection_Test_Connect, NegTest_ServerWithCertChainDepth2)
{
    EXPECT_EQ(tlsServer.start(port, KeyPaths::CaCert, CertChainDepth2KeyPaths::ServerCert, CertChainDepth2KeyPaths::ServerKey), SERVER_START_OK);
    EXPECT_EQ(tlsClient.start("localhost", port, KeyPaths::CaCert, KeyPaths::ClientCert, KeyPaths::ClientKey), CLIENT_ERROR_START_CONNECT_INIT);
    EXPECT_EQ(tlsServer.getClientIds().size(), 0);
}

// ====================================================================================================================
// Desc:       Check TLS connection without client certificate (Nothing given)
// Steps:      Connect to server without client certificate: Self-authentication (No certificate means nullptr)
//             CA: nullptr, Cert: nullptr, Key: nullptr
// Exp Result: SERVER_START_OK, CLIENT_START_OK
// ====================================================================================================================
TEST_F(General_TlsConnection_Test_Connect, PosTest_NoClientCert_ca_cert_key)
{
    EXPECT_EQ(tlsServer.start(port, KeyPaths::CaCert, KeyPaths::ServerCert, KeyPaths::ServerKey, false), SERVER_START_OK);
    EXPECT_EQ(tlsClient.start("localhost", port, "", "", ""), CLIENT_START_OK); // Return CLIENT_ERROR_START_CONNECT_INIT (Error when doing TLS handshake)
    // EXPECT_EQ(tlsClient.start("localhost", port, SelfSignedKeyPaths::CaCert, SelfSignedKeyPaths::ClientCert, SelfSignedKeyPaths::ClientKey), CLIENT_START_OK); // Return CLIENT_ERROR_START_CONNECT_INIT (Error when doing TLS handshake)
    // EXPECT_EQ(tlsClient.start("localhost", port, KeyPaths::CaCert, KeyPaths::ClientCert, KeyPaths::ClientKey), CLIENT_START_OK); // Return CLIENT_START_OK (working fine)
    EXPECT_EQ(tlsServer.getClientIds().size(), 1);

    // Server says: error:0A000418:SSL routines::tlsv1 alert unknown ca (167773208)
    // Client says:error:0A000086:SSL routines::certificate verify failed (167772294)
}
