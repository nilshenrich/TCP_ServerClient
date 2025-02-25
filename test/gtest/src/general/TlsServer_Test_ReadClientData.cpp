#include <vector>
#include <string>

#include "general/TlsServer_Test_ReadClientData.h"
#include "HelperFunctions.h"

using namespace std;
using namespace Test;
using namespace tcp;

General_TlsServer_Test_ReadClientData::General_TlsServer_Test_ReadClientData() {}
General_TlsServer_Test_ReadClientData::~General_TlsServer_Test_ReadClientData() {}

void General_TlsServer_Test_ReadClientData::SetUp()
{
    // Get free TCP port
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

void General_TlsServer_Test_ReadClientData::TearDown()
{
    // Stop TLS server and client
    tlsClient.stop();
    tlsServer.stop();

    // Check if no pipe error occurred
    EXPECT_FALSE(HelperFunctions::getAndResetPipeError()) << "Pipe error occurred!";

    return;
}

// ====================================================================================================================
// Desc:       Read Client IP (positive)
// Steps:      Read the IP of the connected client
// Exp Result: IP is read successfully
// ====================================================================================================================
TEST_F(General_TlsServer_Test_ReadClientData, PosTest_ReadClientIp)
{
    // Read client IP
    string clientIp{tlsServer.getClientIp(clientId)};

    // Check if client IP is not empty
    EXPECT_EQ(clientIp, "127.0.0.1");

    return;
}

// ====================================================================================================================
// Desc:       Read Client IP (negative)
// Steps:      Read the IP of the connected client
// Exp Result: Exception is thrown
// ====================================================================================================================
TEST_F(General_TlsServer_Test_ReadClientData, NegTest_ReadClientIp_NotConnected)
{
    // Read client IP
    EXPECT_THROW(tlsServer.getClientIp(0), Server_error); // 0 never exists as a client ID

    return;
}
