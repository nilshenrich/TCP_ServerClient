#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <vector>
#include <string>

#include "fragmentation/TcpConnection_Test_MsgAppend.h"
#include "HelperFunctions.h"

using namespace std;
using namespace Test;
using namespace tcp;

Fragmentation_TcpConnection_Test_MsgAppend::Fragmentation_TcpConnection_Test_MsgAppend() {}
Fragmentation_TcpConnection_Test_MsgAppend::~Fragmentation_TcpConnection_Test_MsgAppend() {}

void Fragmentation_TcpConnection_Test_MsgAppend::SetUp()
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
void Fragmentation_TcpConnection_Test_MsgAppend::TearDown()
{
    tcpServer.stop();
    tcpClient.stop();

    // Check if no pipe error occurred
    EXPECT_FALSE(HelperFunctions::getAndResetPipeError()) << "Pipe error occurred!";

    return;
}

// ====================================================================================================================
// Desc:       Send normal message from client to server
// Steps:      Send normal message from client to server
// Exp Result: Message received by server with string appended to the end
// ====================================================================================================================
TEST_F(Fragmentation_TcpConnection_Test_MsgAppend, PosTest_ClientToServer_NormalMsg)
{
    // Send message from client to server
    string msg{"Hello server!"};
    EXPECT_TRUE(tcpClient.sendMsg(msg));
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TCP);

    // Check if message received by server with string appended to the end
    vector<TestApi::MessageFromClient> messagesExpected{TestApi::MessageFromClient{clientId, msg + messageAppendClient}};
    EXPECT_EQ(tcpServer.getBufferedMsg(), messagesExpected);
}

// ====================================================================================================================
// Desc:       Send normal message from server to client
// Steps:      Send normal message from server to client
// Exp Result: Message received by client with string appended to the end
// ====================================================================================================================
TEST_F(Fragmentation_TcpConnection_Test_MsgAppend, PosTest_ServerToClient_NormalMsg)
{
    // Send message from server to client
    string msg{"Hello client!"};
    EXPECT_TRUE(tcpServer.sendMsg(clientId, msg));
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TCP);

    // Check if message received by client with string appended to the end
    vector<string> messagesExpected{msg + messageAppendServer};
    EXPECT_EQ(tcpClient.getBufferedMsg(), messagesExpected);
}
