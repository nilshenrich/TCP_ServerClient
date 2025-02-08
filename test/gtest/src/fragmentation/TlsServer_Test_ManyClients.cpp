#include <thread>
#include <map>
#include <memory>
#include <string>

#include "fragmentation/TlsServer_Test_ManyClients.h"
#include "HelperFunctions.h"

using namespace std;
using namespace Test;
using namespace tcp;

Fragmentation_TlsServer_Test_ManyClients::Fragmentation_TlsServer_Test_ManyClients() {}
Fragmentation_TlsServer_Test_ManyClients::~Fragmentation_TlsServer_Test_ManyClients() {}

void Fragmentation_TlsServer_Test_ManyClients::SetUp()
{
    // Get free TLS port
    port = HelperFunctions::getFreePort();
    ASSERT_NE(port, -1) << "No free port found";

    // Start TLS server
    ASSERT_EQ(tlsServer.start(port), SERVER_START_OK) << "Unable to start TLS server on port " << port;

    // Create and connect all TLS clients
    for (int i{0}; i < TestConstants::MANYCLIENTS_NUMBER; i += 1)
    {
        unique_ptr<TestApi::TlsClientApi_fragmentation> tlsClientNew{new TestApi::TlsClientApi_fragmentation()};
        ASSERT_EQ(tlsClientNew->start("localhost", port), CLIENT_START_OK);

        // Find out ID of newly connected client (The one, that is not added to clients collection yet)
        vector<int> connectedClients{tlsServer.getClientIds()};
        bool newClientAdded{false};
        for (int id : connectedClients)
        {
            if (tlsClients.find(id) == tlsClients.end())
            {
                tlsClients[id] = move(tlsClientNew);
                newClientAdded = true;
                break;
            }
        }
        ASSERT_TRUE(newClientAdded) << "No ID for client No. " << (i + 1) << " found";
    }
}

void Fragmentation_TlsServer_Test_ManyClients::TearDown()
{
    // Stop server and all clients
    for (auto &client : tlsClients)
        client.second->stop();
    tlsServer.stop();

    // Check if no pipe error occurred
    EXPECT_FALSE(HelperFunctions::getAndResetPipeError()) << "Pipe error occurred!";

    return;
}

// ====================================================================================================================
// Desc:       Clients to server consecutively
// Steps:      All clients send messages to server in single thread
// Exp Result: All messages received (Order doesn't matter)
// ====================================================================================================================
TEST_F(Fragmentation_TlsServer_Test_ManyClients, SendingClientsSingleThread)
{
    // Create messages to send
    map<int, string> messages;
    for (auto &client : tlsClients)
        messages[client.first] = "Sending from client " + to_string(client.first) + " to server in single thread";

    // Send messages consecutively
    for (auto &client : tlsClients)
        EXPECT_TRUE(client.second->sendMsg(messages[client.first]));
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TLS);

    // Check all messages are received by server (Order doesn't matter)
    vector<TestApi::MessageFromClient> messagesReceived{tlsServer.getBufferedMsg()};
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
TEST_F(Fragmentation_TlsServer_Test_ManyClients, SendingServerSingleThread)
{
    map<int, string> messages;
    for (auto &client : tlsClients)
        messages[client.first] = "Sending from server to client " + to_string(client.first) + " in single thread";

    // Send messages consecutively
    for (auto &msg : messages)
        EXPECT_TRUE(tlsServer.sendMsg(msg.first, msg.second));
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TLS);

    // Check all messages are received by clients
    for (auto &client : tlsClients)
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
TEST_F(Fragmentation_TlsServer_Test_ManyClients, SendingClientsMultipleThreads)
{
    // Create messages to send
    map<int, string> messages;
    for (auto &client : tlsClients)
        messages[client.first] = "Sending from client " + to_string(client.first) + " to server in multiple threads";

    // Send messages in parallel
    vector<thread> sendingThreads;
    for (auto &client : tlsClients)
        sendingThreads.push_back(thread{[&]()
                                        { EXPECT_TRUE(client.second->sendMsg(messages[client.first])); }});
    for (thread &t : sendingThreads)
        t.join();
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TLS);

    // Check all messages are received by server (Order doesn't matter)
    vector<TestApi::MessageFromClient> messagesReceived{tlsServer.getBufferedMsg()};
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
TEST_F(Fragmentation_TlsServer_Test_ManyClients, SendingServerMultipleThreads)
{
    map<int, string> messages;
    for (auto &client : tlsClients)
        messages[client.first] = "Sending from server to client " + to_string(client.first) + " in multiple threads";

    // Send messages in parallel
    vector<thread> sendingThreads;
    for (auto &msg : messages)
        sendingThreads.push_back(thread{
            [&]()
            { EXPECT_TRUE(tlsServer.sendMsg(msg.first, msg.second)); }});
    for (thread &t : sendingThreads)
        t.join();
    this_thread::sleep_for(TestConstants::WAITFOR_MSG_TLS);

    // Check all messages are received by clients
    for (auto &client : tlsClients)
    {
        vector<string> messagesReceived{client.second->getBufferedMsg()};
        EXPECT_EQ(messagesReceived.size(), 1);
        if (messagesReceived.empty())
            continue;
        EXPECT_EQ(messagesReceived[0], messages[client.first]);
    }
}
