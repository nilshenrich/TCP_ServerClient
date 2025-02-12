#include <thread>
#include <map>
#include <memory>
#include <string>

#include "continuous/TcpServer_Test_ManyClients.h"
#include "HelperFunctions.h"

using namespace std;
using namespace Test;
using namespace tcp;

Continuous_TcpServer_Test_ManyClients::Continuous_TcpServer_Test_ManyClients() {}
Continuous_TcpServer_Test_ManyClients::~Continuous_TcpServer_Test_ManyClients() {}

void Continuous_TcpServer_Test_ManyClients::SetUp()
{
    // Get free TCP port
    port = HelperFunctions::getFreePort();
    ASSERT_NE(port, -1) << "No free port found";

    // Start TCP server
    ASSERT_EQ(tcpServer.start(port), SERVER_START_OK) << "Unable to start TCP server on port " << port;

    // Create and connect all TCP clients
    for (int i{0}; i < TestConstants::MANYCLIENTS_NUMBER; i += 1)
    {
        unique_ptr<TestApi::TcpClientApi_continuous> tcpClientNew{new TestApi::TcpClientApi_continuous()};
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

void Continuous_TcpServer_Test_ManyClients::TearDown()
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
TEST_F(Continuous_TcpServer_Test_ManyClients, SendingClientsSingleThread)
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
    // TODO: First check if key exists in received messages
    map<int, string> messagesReceived{tcpServer.getBufferedMsg()};
    EXPECT_EQ(messagesReceived.size(), TestConstants::MANYCLIENTS_NUMBER) << "Messages count doesn't match number of clients";
    for (auto &client : tcpClients)
    {
        EXPECT_EQ(messagesReceived[client.first], messages[client.first]) << "Message from client " << client.first << " doesn't match buffer" << messages[client.first];
    }
}

// ====================================================================================================================
// Desc:       Server to clients consecutively
// Steps:      Server sends messages to all clients in single thread
// Exp Result: All messages received
// ====================================================================================================================
TEST_F(Continuous_TcpServer_Test_ManyClients, SendingServerSingleThread)
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
        EXPECT_EQ(client.second->getBufferedMsg(), messages[client.first]);
    }
}

// ====================================================================================================================
// Desc:       Clients to server in parallel
// Steps:      All clients send messages to server in multiple threads
// Exp Result: All messages received (Order doesn't matter)
// ====================================================================================================================
TEST_F(Continuous_TcpServer_Test_ManyClients, SendingClientsMultipleThreads)
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
    // TODO: First check if key exists in received messages
    map<int, string> messagesReceived{tcpServer.getBufferedMsg()};
    EXPECT_EQ(messagesReceived.size(), TestConstants::MANYCLIENTS_NUMBER) << "Messages count doesn't match number of clients";
    for (auto &client : tcpClients)
    {
        EXPECT_EQ(messagesReceived[client.first], messages[client.first]) << "Message from client " << client.first << " doesn't match buffer" << messages[client.first];
    }
}

// ====================================================================================================================
// Desc:       Server to clients in parallel
// Steps:      Server sends messages to all clients in multiple threads
// Exp Result: All messages received
// ====================================================================================================================
TEST_F(Continuous_TcpServer_Test_ManyClients, SendingServerMultipleThreads)
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
        EXPECT_EQ(client.second->getBufferedMsg(), messages[client.first]);
    }
}
