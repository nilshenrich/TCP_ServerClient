#include "forwarding/TlsServer_Test_Send.h"

using namespace std;
using namespace Test;
using namespace networking;

Forwarding_TlsServer_Test_Send::Forwarding_TlsServer_Test_Send() {}
Forwarding_TlsServer_Test_Send::~Forwarding_TlsServer_Test_Send() {}

void Forwarding_TlsServer_Test_Send::SetUp()
{
    // Get free TLS port
    port = HelperFunctions::getFreePort();
    ASSERT_NE(port, -1) << "No free port found";

    // Start TLS server and connect client
    ASSERT_EQ(tlsServer.start(port), NETWORKLISTENER_START_OK) << "Unable to start TLS server on port " << port;
    ASSERT_EQ(tlsClient.start("localhost", port), NETWORKCLIENT_START_OK) << "Unable to connect TLS client to localhost on port " << port;

    // Get client ID
    vector<int> clientIds{tlsServer.getClientIds()};
    ASSERT_EQ(clientIds.size(), 1);
    clientId = clientIds[0];

    return;
}

void Forwarding_TlsServer_Test_Send::TearDown()
{
    // Stop TLS server and client
    tlsClient.stop();
    tlsServer.stop();

    // Check if no pipe error occurred
    EXPECT_FALSE(HelperFunctions::getAndResetPipeError()) << "Pipe error occurred!";

    return;
}

// ====================================================================================================================
// Desc:       Send message to client that is not connected
// Steps:      Try to send message to client that is not connected
// Exp Result: Message is not sent
// ====================================================================================================================
TEST_F(Forwarding_TlsServer_Test_Send, NegTest_ClientNotConnected)
{
    // Send message to client that is not connected
    EXPECT_FALSE(tlsServer.sendMsg(clientId + 1, "Test message"));

    // Check if no message was received by client
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TLS);
    EXPECT_EQ(tlsClient.getBufferedMsg().size(), 0);

    return;
}

// ====================================================================================================================
// Desc:       Send message to client that is disconnected immediately before
// Steps:      Try to send message to client that is disconnected immediately before
// Exp Result: Message is not sent
// ====================================================================================================================
TEST_F(Forwarding_TlsServer_Test_Send, NegTest_ClientDisconnectedBefore)
{
    // Disconnect client
    tlsClient.stop();
    this_thread::sleep_for(TestConstants::DISCONNECTION_TIMEOUT);

    // Send message to client that is disconnected immediately before
    EXPECT_FALSE(tlsServer.sendMsg(clientId, "Test message"));

    // Check if no message was received by client
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TLS);
    EXPECT_EQ(tlsClient.getBufferedMsg().size(), 0);

    return;
}

// ====================================================================================================================
// Desc:       Send message to client while server is not running
// Steps:      Try to send message to client while server is not running
// Exp Result: Message is not sent
// ====================================================================================================================
TEST_F(Forwarding_TlsServer_Test_Send, NegTest_ServerNotRunning)
{
    // Stop TLS server
    tlsServer.stop();

    // Send message to client while server is not running
    EXPECT_FALSE(tlsServer.sendMsg(clientId, "Test message"));

    // Check if no message was received by client
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TLS);
    EXPECT_EQ(tlsClient.getBufferedMsg().size(), 0);

    return;
}
