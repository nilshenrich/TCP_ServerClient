#include <vector>
#include <string>

#include "general/TcpServer_Test_ReadClientData.h"
#include "HelperFunctions.h"

using namespace std;
using namespace Test;
using namespace tcp;

General_TcpServer_Test_ReadClientData::General_TcpServer_Test_ReadClientData() {}
General_TcpServer_Test_ReadClientData::~General_TcpServer_Test_ReadClientData() {}

void General_TcpServer_Test_ReadClientData::SetUp()
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

void General_TcpServer_Test_ReadClientData::TearDown()
{
    // Stop TCP server and client
    tcpClient.stop();
    tcpServer.stop();

    // Check if no pipe error occurred
    EXPECT_FALSE(HelperFunctions::getAndResetPipeError()) << "Pipe error occurred!";

    return;
}

// ====================================================================================================================
// Desc:       Read Client IP (positive)
// Steps:      Read the IP of the connected client
// Exp Result: IP is read successfully
// ====================================================================================================================
TEST_F(General_TcpServer_Test_ReadClientData, PosTest_ReadClientIp)
{
    // Read client IP
    string clientIp{tcpServer.getClientIp(clientId)};

    // Check if client IP is not empty
    EXPECT_EQ(clientIp, "127.0.0.1");

    return;
}

// ====================================================================================================================
// Desc:       Read Client IP (negative)
// Steps:      Read the IP of a client that is not connected
// Exp Result: Exception is thrown
// ====================================================================================================================
TEST_F(General_TcpServer_Test_ReadClientData, NegTest_ReadClientIp_NotConnected)
{
    // Read client IP
    EXPECT_THROW(tcpServer.getClientIp(0), Server_error); // 0 never exists as a client ID

    return;
}
