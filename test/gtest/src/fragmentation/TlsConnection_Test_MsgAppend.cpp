#include <chrono>
#include <thread>
#include <vector>
#include <string>

#include "fragmentation/TlsConnection_Test_MsgAppend.h"
#include "HelperFunctions.h"

using namespace std;
using namespace Test;
using namespace tcp;

Fragmentation_TlsConnection_Test_MsgAppend::Fragmentation_TlsConnection_Test_MsgAppend() {}
Fragmentation_TlsConnection_Test_MsgAppend::~Fragmentation_TlsConnection_Test_MsgAppend() {}

void Fragmentation_TlsConnection_Test_MsgAppend::SetUp()
{
    // Get free TLS port
    port = HelperFunctions::getFreePort();
    ASSERT_NE(port, -1) << "No free port found";

    // Start TLS server and connect client
    ASSERT_EQ(tlsServer.start(port), SERVER_START_OK) << "Unable to start TLS server on port " << port;
    ASSERT_EQ(tlsClient.start("localhost", port), CLIENT_START_OK) << "Unable to connect TLS client to localhost on port " << port;

    // Get client ID
    vector<int> clientIds{tlsServer.getClientIds()};
    ASSERT_EQ(clientIds.size(), 1);
    clientId = clientIds[0];

    return;
}
void Fragmentation_TlsConnection_Test_MsgAppend::TearDown()
{
    tlsServer.stop();
    tlsClient.stop();

    // Check if no pipe error occurred
    EXPECT_FALSE(HelperFunctions::getAndResetPipeError()) << "Pipe error occurred!";

    return;
}

// ====================================================================================================================
// Desc:       Send normal message from client to server
// Steps:      Send normal message from client to server
// Exp Result: Message received by server with string appended to the end
// ====================================================================================================================
TEST_F(Fragmentation_TlsConnection_Test_MsgAppend, PosTest_ClientToServer_NormalMsg)
{
    // Send message from client to server
    string msg{"Hello server!"};
    EXPECT_TRUE(tlsClient.sendMsg(msg));
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TLS);

    // Check if message received by server with string appended to the end
    vector<TestApi::MessageFromClient> messagesExpected{TestApi::MessageFromClient{clientId, msg + messageAppendClient}};
    EXPECT_EQ(tlsServer.getBufferedMsg(), messagesExpected);
}

// ====================================================================================================================
// Desc:       Send normal message from server to client
// Steps:      Send normal message from server to client
// Exp Result: Message received by client with string appended to the end
// ====================================================================================================================
TEST_F(Fragmentation_TlsConnection_Test_MsgAppend, PosTest_ServerToClient_NormalMsg)
{
    // Send message from server to client
    string msg{"Hello client!"};
    EXPECT_TRUE(tlsServer.sendMsg(clientId, msg));
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TLS);

    // Check if message received by client with string appended to the end
    vector<string> messagesExpected{msg + messageAppendServer};
    EXPECT_EQ(tlsClient.getBufferedMsg(), messagesExpected);
}
