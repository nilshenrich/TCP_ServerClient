#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <vector>
#include <string>

#include "continuous/TcpConnection_Test_MsgTransfer.h"
#include "HelperFunctions.h"

using namespace std;
using namespace Test;
using namespace tcp;

Continuous_TcpConnection_Test_MsgTransfer::Continuous_TcpConnection_Test_MsgTransfer() {}
Continuous_TcpConnection_Test_MsgTransfer::~Continuous_TcpConnection_Test_MsgTransfer() {}

void Continuous_TcpConnection_Test_MsgTransfer::SetUp()
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

void Continuous_TcpConnection_Test_MsgTransfer::TearDown()
{
    // Stop TCP server and client
    tcpClient.stop();
    tcpServer.stop();

    // Check if no pipe error occurred
    EXPECT_FALSE(HelperFunctions::getAndResetPipeError()) << "Pipe error occurred!";

    return;
}

// ====================================================================================================================
// Desc:       Send normal message from client to server
// Steps:      Send normal message from client to server
// Exp Result: Message received by server
// ====================================================================================================================
TEST_F(Continuous_TcpConnection_Test_MsgTransfer, PosTest_ClientToServer_NormalMsg)
{
    // Send message from client to server
    string msg{"Hello server!"};
    EXPECT_TRUE(tcpClient.sendMsg(msg));
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TCP);

    // Check if message received by server
    map<int, string> messagesExpected{{clientId, msg}};
    EXPECT_EQ(tcpServer.getBufferedMsg(), messagesExpected);
}

// ====================================================================================================================
// Desc:       Send normal message from server to client
// Steps:      Send normal message from server to client
// Exp Result: Message received by client
// ====================================================================================================================
TEST_F(Continuous_TcpConnection_Test_MsgTransfer, PosTest_ServerToClient_NormalMsg)
{
    // Send message from server to client
    string msg{"Hello client!"};
    EXPECT_TRUE(tcpServer.sendMsg(clientId, msg));
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TCP);

    // Check if message received by client
    EXPECT_EQ(tcpClient.getBufferedMsg(), msg);
}

// ====================================================================================================================
// Desc:       Send long message from client to server
// Steps:      Send long message from client to server (1000000)
// Exp Result: Message received by server
// ====================================================================================================================
TEST_F(Continuous_TcpConnection_Test_MsgTransfer, PosTest_ClientToServer_LongMsg)
{
    // Generate message with 1000000 elements of ASCII characters 33 - 126
    string msg;
    for (size_t i = 0; i < 1000000; i += 1)
        msg += static_cast<char>(i % 94 + 33);

    // Send message from client to server
    EXPECT_TRUE(tcpClient.sendMsg(msg));
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_LONG_TCP);

    // Check if message received by server
    map<int, string> messagesExpected{{clientId, msg}};
    EXPECT_EQ(tcpServer.getBufferedMsg(), messagesExpected);
}

// ====================================================================================================================
// Desc:       Send long message from server to client
// Steps:      Send long message from server to client (1000000)
// Exp Result: Message received by client
// ====================================================================================================================
TEST_F(Continuous_TcpConnection_Test_MsgTransfer, PosTest_ServerToClient_LongMsg)
{
    // Generate message with 1000000 elements of ASCII characters 33 - 126
    string msg;
    for (size_t i = 0; i < 1000000; i += 1)
        msg += static_cast<char>(i % 94 + 33);

    // Send message from server to client
    EXPECT_TRUE(tcpServer.sendMsg(clientId, msg));
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_LONG_TCP);

    // Check if message received by client
    EXPECT_EQ(tcpClient.getBufferedMsg(), msg);
}
