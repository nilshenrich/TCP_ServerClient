#include "forwarding/TcpClient_Test_Send.h"

using namespace std;
using namespace Test;
using namespace networking;

Forwarding_TcpClient_Test_Send::Forwarding_TcpClient_Test_Send() {}
Forwarding_TcpClient_Test_Send::~Forwarding_TcpClient_Test_Send() {}

void Forwarding_TcpClient_Test_Send::SetUp()
{
    // Get free TCP port
    port = HelperFunctions::getFreePort();
    ASSERT_NE(port, -1) << "No free port found";

    // Start TCP server and connect client
    ASSERT_EQ(tcpServer.start(port), NETWORKLISTENER_START_OK) << "Unable to start TCP server on port " << port;
    ASSERT_EQ(tcpClient.start("localhost", port), NETWORKCLIENT_START_OK) << "Unable to connect TCP client to localhost on port " << port;

    return;
}

void Forwarding_TcpClient_Test_Send::TearDown()
{
    // Stop TCP server and client
    tcpClient.stop();
    tcpServer.stop();

    // Check if no pipe error occurred
    EXPECT_FALSE(HelperFunctions::getAndResetPipeError()) << "Pipe error occurred!";

    return;
}

// ====================================================================================================================
// Desc:       Send message to server that has stopped running immediately before
// Steps:      Try to send message to server that has stopped running immediately before
// Exp Result: Message is not sent
// ====================================================================================================================
TEST_F(Forwarding_TcpClient_Test_Send, NegTest_ServerNotRunning)
{
    // Stop TCP server
    tcpServer.stop();
    this_thread::sleep_for(TestConstants::DISCONNECTION_TIMEOUT);

    // Send message to server that has stopped running immediately before
    EXPECT_FALSE(tcpClient.sendMsg("Test message"));

    // Check if no message was received by server
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TCP);
    EXPECT_EQ(tcpServer.getBufferedMsg().size(), 0);

    return;
}

// ====================================================================================================================
// Desc:       Send message to server while client is not running
// Steps:      Try to send message to server while client is not running
// Exp Result: Message is not sent
// ====================================================================================================================
TEST_F(Forwarding_TcpClient_Test_Send, NegTest_ClientNotRunning)
{
    // Stop TCP client
    tcpClient.stop();

    // Send message to server while client is not running
    EXPECT_FALSE(tcpClient.sendMsg("Test message"));

    // Check if no message was received by server
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TCP);
    EXPECT_EQ(tcpServer.getBufferedMsg().size(), 0);

    return;
}
