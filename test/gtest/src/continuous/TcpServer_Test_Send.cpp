#include <chrono>
#include <thread>
#include <vector>
#include <string>

#include "continuous/TcpServer_Test_Send.h"
#include "HelperFunctions.h"

using namespace std;
using namespace Test;
using namespace tcp;

Continuous_TcpServer_Test_Send::Continuous_TcpServer_Test_Send() {}
Continuous_TcpServer_Test_Send::~Continuous_TcpServer_Test_Send() {}

void Continuous_TcpServer_Test_Send::SetUp()
{
    // Get free TCP port
    port = HelperFunctions::getFreePort();
    ASSERT_NE(port, -1) << "No free port found";

    // Start TCP server and connect client
    ASSERT_EQ(tcpServer.start(port), SERVER_START_OK) << "Unable to start TCP server on port " << port;
    ASSERT_EQ(tcpClient.start("localhost", port), CLIENT_START_OK) << "Unable to connect TCP client to localhost on port " << port;

    // Get client ID
    vector<int> clientIds{tcpServer.getClientIds()};
    ASSERT_EQ(clientIds.size(), 1);
    clientId = clientIds[0];

    return;
}

void Continuous_TcpServer_Test_Send::TearDown()
{
    // Stop TCP server and client
    tcpClient.stop();
    tcpServer.stop();

    // Check if no pipe error occurred
    EXPECT_FALSE(HelperFunctions::getAndResetPipeError()) << "Pipe error occurred!";

    return;
}

// ====================================================================================================================
// Desc:       Send message to client that is not connected
// Steps:      Try to send message to client that is not connected
// Exp Result: Message is not sent
// ====================================================================================================================
TEST_F(Continuous_TcpServer_Test_Send, NegTest_ClientNotConnected)
{
    // Send message to client that is not connected
    EXPECT_FALSE(tcpServer.sendMsg(clientId + 1, "Test message"));

    // Check if no message was received by client
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TCP);
    EXPECT_EQ(tcpClient.getBufferedMsg().size(), 0);

    return;
}

// ====================================================================================================================
// Desc:       Send message to client that is disconnected immediately before
// Steps:      Try to send message to client that is disconnected immediately before
// Exp Result: Message is not sent
// ====================================================================================================================
TEST_F(Continuous_TcpServer_Test_Send, NegTest_ClientDisconnectedBefore)
{
    // Disconnect client
    tcpClient.stop();

    // Send message to client that is disconnected immediately before
    EXPECT_FALSE(tcpServer.sendMsg(clientId, "Test message"));

    // Check if no message was received by client
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TCP);
    EXPECT_EQ(tcpClient.getBufferedMsg().size(), 0);

    return;
}

// ====================================================================================================================
// Desc:       Send message to client while server is not running
// Steps:      Try to send message to client while server is not running
// Exp Result: Message is not sent
// ====================================================================================================================
TEST_F(Continuous_TcpServer_Test_Send, NegTest_ServerNotRunning)
{
    // Stop TCP server
    tcpServer.stop();

    // Send message to client while server is not running
    EXPECT_FALSE(tcpServer.sendMsg(clientId, "Test message"));

    // Check if no message was received by client
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TCP);
    EXPECT_EQ(tcpClient.getBufferedMsg().size(), 0);

    return;
}
