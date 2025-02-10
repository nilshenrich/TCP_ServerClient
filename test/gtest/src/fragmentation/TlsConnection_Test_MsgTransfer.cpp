#include <chrono>
#include <thread>
#include <vector>
#include <string>

#include "fragmentation/TlsConnection_Test_MsgTransfer.h"
#include "HelperFunctions.h"

using namespace std;
using namespace Test;
using namespace tcp;

Fragmentation_TlsConnection_Test_MsgTransfer::Fragmentation_TlsConnection_Test_MsgTransfer() {}
Fragmentation_TlsConnection_Test_MsgTransfer::~Fragmentation_TlsConnection_Test_MsgTransfer() {}

void Fragmentation_TlsConnection_Test_MsgTransfer::SetUp()
{
    // ==============================================
    // ========== Long server, long client ==========
    // ==============================================
    {
        // Get free TLS port
        port_serverLong_clientLong = HelperFunctions::getFreePort();
        ASSERT_NE(port_serverLong_clientLong, -1) << "No free port found";

        // Start TLS server and connect client
        ASSERT_EQ(tlsServer_selfLong_frgnLong.start(port_serverLong_clientLong), SERVER_START_OK) << "Unable to start TLS server on port " << port_serverLong_clientLong;
        ASSERT_EQ(tlsClient_selfLong_frgnLong.start("localhost", port_serverLong_clientLong), CLIENT_START_OK) << "Unable to connect TLS client to localhost on port " << port_serverLong_clientLong;

        // Get client ID
        vector<int> clientIds{tlsServer_selfLong_frgnLong.getClientIds()};
        ASSERT_EQ(clientIds.size(), 1);
        clientId_serverLong_clientLong = clientIds[0];
    }
    // ==============================================
    // ========== Long server, short client =========
    // ==============================================
    {
        // Get free TLS port
        port_serverLong_clientShort = HelperFunctions::getFreePort();
        ASSERT_NE(port_serverLong_clientShort, -1) << "No free port found";

        // Start TLS server and connect client
        ASSERT_EQ(tlsServer_selfLong_frgnShort.start(port_serverLong_clientShort), SERVER_START_OK) << "Unable to start TLS server on port " << port_serverLong_clientShort;
        ASSERT_EQ(tlsClient_selfLong_frgnShort.start("localhost", port_serverLong_clientShort), CLIENT_START_OK) << "Unable to connect TLS client to localhost on port " << port_serverLong_clientShort;

        // Get client ID
        vector<int> clientIds{tlsServer_selfLong_frgnShort.getClientIds()};
        ASSERT_EQ(clientIds.size(), 1);
        clientId_serverLong_clientShort = clientIds[0];
    }
    // ==============================================
    // ========== Short server, long client =========
    // ==============================================
    {
        // Get free TLS port
        port_serverShort_clientLong = HelperFunctions::getFreePort();
        ASSERT_NE(port_serverShort_clientLong, -1) << "No free port found";

        // Start TLS server and connect client
        ASSERT_EQ(tlsServer_selfShort_frgnLong.start(port_serverShort_clientLong), SERVER_START_OK) << "Unable to start TLS server on port " << port_serverShort_clientLong;
        ASSERT_EQ(tlsClient_selfShort_frgnLong.start("localhost", port_serverShort_clientLong), CLIENT_START_OK) << "Unable to connect TLS client to localhost on port " << port_serverShort_clientLong;

        // Get client ID
        vector<int> clientIds{tlsServer_selfShort_frgnLong.getClientIds()};
        ASSERT_EQ(clientIds.size(), 1);
        clientId_serverShort_clientLong = clientIds[0];
    }
    // ==============================================
    // ========= Short server, short client =========
    // ==============================================
    {
        // Get free TLS port
        port_serverShort_clientShort = HelperFunctions::getFreePort();
        ASSERT_NE(port_serverShort_clientShort, -1) << "No free port found";

        // Start TLS server and connect client
        ASSERT_EQ(tlsServer_selfShort_frgnShort.start(port_serverShort_clientShort), SERVER_START_OK) << "Unable to start TLS server on port " << port_serverShort_clientShort;
        ASSERT_EQ(tlsClient_selfShort_frgnShort.start("localhost", port_serverShort_clientShort), CLIENT_START_OK) << "Unable to connect TLS client to localhost on port " << port_serverShort_clientShort;

        // Get client ID
        vector<int> clientIds{tlsServer_selfShort_frgnShort.getClientIds()};
        ASSERT_EQ(clientIds.size(), 1);
        clientId_serverShort_clientShort = clientIds[0];
    }
    return;
}

void Fragmentation_TlsConnection_Test_MsgTransfer::TearDown()
{
    // Stop TLS server and client
    tlsClient_selfLong_frgnLong.stop();
    tlsClient_selfLong_frgnShort.stop();
    tlsClient_selfShort_frgnLong.stop();
    tlsClient_selfShort_frgnShort.stop();
    tlsServer_selfLong_frgnLong.stop();
    tlsServer_selfLong_frgnShort.stop();
    tlsServer_selfShort_frgnLong.stop();
    tlsServer_selfShort_frgnShort.stop();

    // Check if no pipe error occurred
    EXPECT_FALSE(HelperFunctions::getAndResetPipeError()) << "Pipe error occurred!";

    return;
}

// ====================================================================================================================
// Desc:       Send normal message from client to server
// Steps:      Send normal message from client to server
// Exp Result: Message received by server
// ====================================================================================================================
TEST_F(Fragmentation_TlsConnection_Test_MsgTransfer, PosTest_ClientToServer_NormalMsg)
{
    // Send message from client to server
    string msg{"Hello server!"};
    EXPECT_TRUE(tlsClient_selfLong_frgnLong.sendMsg(msg));
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TLS);

    // Check if message received by server
    vector<TestApi::MessageFromClient> messagesExpected{TestApi::MessageFromClient{clientId_serverLong_clientLong, msg}};
    EXPECT_EQ(tlsServer_selfLong_frgnLong.getBufferedMsg(), messagesExpected);
}

// ====================================================================================================================
// Desc:       Send normal message from server to client
// Steps:      Send normal message from server to client
// Exp Result: Message received by client
// ====================================================================================================================
TEST_F(Fragmentation_TlsConnection_Test_MsgTransfer, PosTest_ServerToClient_NormalMsg)
{
    // Send message from server to client
    string msg{"Hello client!"};
    EXPECT_TRUE(tlsServer_selfLong_frgnLong.sendMsg(clientId_serverLong_clientLong, msg));
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TLS);

    // Check if message received by client
    vector<string> messagesExpected{msg};
    EXPECT_EQ(tlsClient_selfLong_frgnLong.getBufferedMsg(), messagesExpected);
}

// ====================================================================================================================
// Desc:       Send message with max length from client to server
// Steps:      Send message with max length from short client to short server
// Exp Result: Message receive by server
// ====================================================================================================================
TEST_F(Fragmentation_TlsConnection_Test_MsgTransfer, PosTest_ClientToServer_MaxLen)
{
    // Generate message with max elements of ASCII characters 33 - 126
    string msg;
    for (size_t i = 0; i < TestConstants::MAXLEN_MSG_SHORT_B; i += 1)
        msg += static_cast<char>(i % 94 + 33);

    // Send message from client to server
    EXPECT_TRUE(tlsClient_selfShort_frgnShort.sendMsg(msg));
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TLS);

    // Check if message received by server
    vector<TestApi::MessageFromClient> messagesExpected{TestApi::MessageFromClient{clientId_serverShort_clientShort, msg}};
    EXPECT_EQ(tlsServer_selfShort_frgnShort.getBufferedMsg(), messagesExpected);
}

// ====================================================================================================================
// Desc:       Send message exceeding max receiving length from client to server
// Steps:      Send message with length max+1 from long client to short server
// Exp Result: Message sent but not received
// ====================================================================================================================
TEST_F(Fragmentation_TlsConnection_Test_MsgTransfer, NegTest_ClientToServer_ExceedMaxLen_OnRec)
{
    // Generate message with more than max elements of ASCII characters 33 - 126
    string msg;
    for (size_t i = 0; i < TestConstants::MAXLEN_MSG_SHORT_B + 1; i += 1)
        msg += static_cast<char>(i % 94 + 33);

    // Send message from client to server
    EXPECT_TRUE(tlsClient_selfLong_frgnShort.sendMsg(msg));
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TLS);

    // Check no message received by server
    EXPECT_EQ(tlsServer_selfShort_frgnLong.getBufferedMsg().size(), 0);
}

// ====================================================================================================================
// Desc:       Send message exceeding max sending length from client to server
// Steps:      Try sending message with length max+1 from short client to long server
// Exp Result: Message not sent
// ====================================================================================================================
TEST_F(Fragmentation_TlsConnection_Test_MsgTransfer, NegTest_ClientToServer_ExceedMaxLen_OnSend)
{
    // Generate message with max elements of ASCII characters 33 - 126
    string msg;
    for (size_t i = 0; i < TestConstants::MAXLEN_MSG_SHORT_B + 1; i += 1)
        msg += static_cast<char>(i % 94 + 33);

    // Send message from client to server
    EXPECT_FALSE(tlsClient_selfShort_frgnLong.sendMsg(msg));
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TLS);

    // Check no message received by server
    EXPECT_EQ(tlsServer_selfLong_frgnShort.getBufferedMsg().size(), 0);
}

// ====================================================================================================================
// Desc:       Send message with max length from server to client
// Steps:      Send message with max length from short server to short client
// Exp Result: Message receive by client
// ====================================================================================================================
TEST_F(Fragmentation_TlsConnection_Test_MsgTransfer, PosTest_ServerToClient_MaxLen)
{
    // Generate message with max elements of ASCII characters 33 - 126
    string msg;
    for (size_t i = 0; i < TestConstants::MAXLEN_MSG_SHORT_B; i += 1)
        msg += static_cast<char>(i % 94 + 33);

    // Send message from server to client
    EXPECT_TRUE(tlsServer_selfShort_frgnShort.sendMsg(clientId_serverShort_clientShort, msg));
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TLS);

    // Check if message received by client
    vector<string> messagesExpected{msg};
    EXPECT_EQ(tlsClient_selfShort_frgnShort.getBufferedMsg(), messagesExpected);
}

// ====================================================================================================================
// Desc:       Send message exceeding max receiving length from server to client
// Steps:      Send message with length max+1 from long server to short client
// Exp Result: Message sent but not received
// ====================================================================================================================
TEST_F(Fragmentation_TlsConnection_Test_MsgTransfer, NegTest_ServerToClient_ExceedMaxLen_OnRec)
{
    // Generate message with max elements of ASCII characters 33 - 126
    string msg;
    for (size_t i = 0; i < TestConstants::MAXLEN_MSG_SHORT_B + 1; i += 1)
        msg += static_cast<char>(i % 94 + 33);

    // Send message from server to client
    EXPECT_TRUE(tlsServer_selfLong_frgnShort.sendMsg(clientId_serverLong_clientShort, msg));
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TLS);

    // Check no message received by client
    EXPECT_EQ(tlsClient_selfShort_frgnLong.getBufferedMsg().size(), 0);
}

// ====================================================================================================================
// Desc:       Send message exceeding max sending length from server to client
// Steps:      Try sending message with length max+1 from short server to long client
// Exp Result: Message not sent
// ====================================================================================================================
TEST_F(Fragmentation_TlsConnection_Test_MsgTransfer, NegTest_ServerToClient_ExceedMaxLen_OnSend)
{
    // Generate message with max elements of ASCII characters 33 - 126
    string msg;
    for (size_t i = 0; i < TestConstants::MAXLEN_MSG_SHORT_B + 1; i += 1)
        msg += static_cast<char>(i % 94 + 33);

    // Send message from server to client
    EXPECT_FALSE(tlsServer_selfShort_frgnLong.sendMsg(clientId_serverShort_clientLong, msg));
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TLS);

    // Check no message received by client
    EXPECT_EQ(tlsClient_selfLong_frgnShort.getBufferedMsg().size(), 0);
}

// ====================================================================================================================
// Desc:       Send long message from client to server
// Steps:      Send long message from client to server (1000000)
// Exp Result: Message received by server
// ====================================================================================================================
TEST_F(Fragmentation_TlsConnection_Test_MsgTransfer, PosTest_ClientToServer_LongMsg)
{
    // Generate message with 1000000 elements of ASCII characters 33 - 126
    string msg;
    for (size_t i = 0; i < 1000000; i += 1)
        msg += static_cast<char>(i % 94 + 33);

    // Send message from client to server
    EXPECT_TRUE(tlsClient_selfLong_frgnLong.sendMsg(msg));
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_LONG_TLS);

    // Check if message received by server
    vector<TestApi::MessageFromClient> messagesExpected{TestApi::MessageFromClient{clientId_serverLong_clientLong, msg}};
    EXPECT_EQ(tlsServer_selfLong_frgnLong.getBufferedMsg(), messagesExpected);
}

// ====================================================================================================================
// Desc:       Send long message from server to client
// Steps:      Send long message from server to client (1000000)
// Exp Result: Message received by client
// ====================================================================================================================
TEST_F(Fragmentation_TlsConnection_Test_MsgTransfer, PosTest_ServerToClient_LongMsg)
{
    // Generate message with 1000000 elements of ASCII characters 33 - 126
    string msg;
    for (size_t i = 0; i < 1000000; i += 1)
        msg += static_cast<char>(i % 94 + 33);

    // Send message from server to client
    EXPECT_TRUE(tlsServer_selfLong_frgnLong.sendMsg(clientId_serverLong_clientLong, msg));
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_LONG_TLS);

    // Check if message received by client
    vector<string> messagesExpected{msg};
    EXPECT_EQ(tlsClient_selfLong_frgnLong.getBufferedMsg(), messagesExpected);
}
