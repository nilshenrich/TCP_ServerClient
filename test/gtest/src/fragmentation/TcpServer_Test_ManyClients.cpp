#include "fragmentation/TcpServer_Test_ManyClients.h"

using namespace std;
using namespace Test;
using namespace tcp_serverclient;

Fragmentation_TcpServer_Test_ManyClients::Fragmentation_TcpServer_Test_ManyClients() {}
Fragmentation_TcpServer_Test_ManyClients::~Fragmentation_TcpServer_Test_ManyClients() {}

void Fragmentation_TcpServer_Test_ManyClients::SetUp()
{
    // Get free TCP port
    port = HelperFunctions::getFreePort();
    ASSERT_NE(port, -1) << "No free port found";

    // Start TCP server
    ASSERT_EQ(tcpServer.start(port), SERVER_START_OK) << "Unable to start TCP server on port " << port;

    // Create and connect all TCP clients
    for (int i{0}; i < TestConstants::MANYCLIENTS_NUMBER; i += 1)
    {
        unique_ptr<TestApi::TcpClientApi_fragmentation> tcpClientNew{new TestApi::TcpClientApi_fragmentation()};
        ASSERT_EQ(tcpClientNew->start("localhost", port), CLIENT_START_OK);

        // Find out ID of newly connected client (The one, that is not added to clients collection yet)
        vector<int> connectedClients{tcpServer.getClientIds()};
        bool newClientAdded{false};
        for (int id : connectedClients)
        {
            if (tcpClients.find(id) == tcpClients.end())
            {
                tcpClients[id] = move(tcpClientNew);
                newClientAdded = true;
                break;
            }
        }
        ASSERT_TRUE(newClientAdded) << "No ID for client No. " << (i + 1) << " found";
    }
}

void Fragmentation_TcpServer_Test_ManyClients::TearDown()
{
    // Stop server and all clients
    for (auto &client : tcpClients)
        client.second->stop();
    tcpServer.stop();

    // Check if no pipe error occurred
    EXPECT_FALSE(HelperFunctions::getAndResetPipeError()) << "Pipe error occurred!";

    return;
}

// ====================================================================================================================
// Desc:       Clients to server consecutively
// Steps:      All clients send messages to server in single thread
// Exp Result: All messages received (Order doesn't matter)
// ====================================================================================================================
TEST_F(Fragmentation_TcpServer_Test_ManyClients, SendingClientsSingleThread)
{
    // Create messages to send
    map<int, string> messages;
    for (auto &client : tcpClients)
        messages[client.first] = "Sending from client " + to_string(client.first) + " to server in single thread";

    // Send messages consecutively
    for (auto &client : tcpClients)
        EXPECT_TRUE(client.second->sendMsg(messages[client.first]));
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TCP);

    // Check all messages are received by server (Order doesn't matter)
    vector<TestApi::MessageFromClient> messagesReceived{tcpServer.getBufferedMsg()};
    EXPECT_EQ(messagesReceived.size(), TestConstants::MANYCLIENTS_NUMBER) << "Messages count doesn't match number of clients";
    for (auto &msg : messages)
    {
        TestApi::MessageFromClient messageExpected{msg.first, msg.second};
        EXPECT_NE(find(messagesReceived.begin(), messagesReceived.end(), messageExpected), messagesReceived.end()) << "Message not found in buffer: " << messageExpected;
    }
}

// ====================================================================================================================
// Desc:       Server to clients consecutively
// Steps:      Server sends messages to all clients in single thread
// Exp Result: All messages received
// ====================================================================================================================
TEST_F(Fragmentation_TcpServer_Test_ManyClients, SendingServerSingleThread)
{
    map<int, string> messages;
    for (auto &client : tcpClients)
        messages[client.first] = "Sending from server to client " + to_string(client.first) + " in single thread";

    // Send messages consecutively
    for (auto &msg : messages)
        EXPECT_TRUE(tcpServer.sendMsg(msg.first, msg.second));
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TCP);

    // Check all messages are received by clients
    for (auto &client : tcpClients)
    {
        vector<string> messagesReceived{client.second->getBufferedMsg()};
        EXPECT_EQ(messagesReceived.size(), 1);
        if (messagesReceived.empty())
            continue;
        EXPECT_EQ(messagesReceived[0], messages[client.first]);
    }
}

// ====================================================================================================================
// Desc:       Clients to server in parallel
// Steps:      All clients send messages to server in multiple threads
// Exp Result: All messages received (Order doesn't matter)
// ====================================================================================================================
TEST_F(Fragmentation_TcpServer_Test_ManyClients, SendingClientsMultipleThreads)
{
    // Create messages to send
    map<int, string> messages;
    for (auto &client : tcpClients)
        messages[client.first] = "Sending from client " + to_string(client.first) + " to server in multiple threads";

    // Send messages in parallel
    vector<thread> sendingThreads;
    for (auto &client : tcpClients)
        sendingThreads.push_back(thread{[&]()
                                        { EXPECT_TRUE(client.second->sendMsg(messages[client.first])); }});
    for (thread &t : sendingThreads)
        t.join();
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TCP);

    // Check all messages are received by server (Order doesn't matter)
    vector<TestApi::MessageFromClient> messagesReceived{tcpServer.getBufferedMsg()};
    EXPECT_EQ(messagesReceived.size(), TestConstants::MANYCLIENTS_NUMBER) << "Messages count doesn't match number of clients";
    for (auto &msg : messages)
    {
        TestApi::MessageFromClient messageExpected{msg.first, msg.second};
        EXPECT_NE(find(messagesReceived.begin(), messagesReceived.end(), messageExpected), messagesReceived.end()) << "Message not found in buffer: " << messageExpected;
    }
}

// ====================================================================================================================
// Desc:       Server to clients in parallel
// Steps:      Server sends messages to all clients in multiple threads
// Exp Result: All messages received
// ====================================================================================================================
TEST_F(Fragmentation_TcpServer_Test_ManyClients, SendingServerMultipleThreads)
{
    map<int, string> messages;
    for (auto &client : tcpClients)
        messages[client.first] = "Sending from server to client " + to_string(client.first) + " in multiple threads";

    // Send messages in parallel
    vector<thread> sendingThreads;
    for (auto &msg : messages)
        sendingThreads.push_back(thread{
            [&]()
            { EXPECT_TRUE(tcpServer.sendMsg(msg.first, msg.second)); }});
    for (thread &t : sendingThreads)
        t.join();
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TCP);

    // Check all messages are received by clients
    for (auto &client : tcpClients)
    {
        vector<string> messagesReceived{client.second->getBufferedMsg()};
        EXPECT_EQ(messagesReceived.size(), 1);
        if (messagesReceived.empty())
            continue;
        EXPECT_EQ(messagesReceived[0], messages[client.first]);
    }
}
