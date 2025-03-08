#include <vector>
#include <string>

#include "general/TlsClient_Test_ReadServerData.h"
#include "HelperFunctions.h"

using namespace std;
using namespace Test;
using namespace tcp;

General_TlsClient_Test_ReadServerData::General_TlsClient_Test_ReadServerData() {}
General_TlsClient_Test_ReadServerData::~General_TlsClient_Test_ReadServerData() {}

void General_TlsClient_Test_ReadServerData::SetUp()
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

void General_TlsClient_Test_ReadServerData::TearDown()
{
    // Stop TLS server and client
    tlsClient.stop();
    tlsServer.stop();

    // Check if no pipe error occurred
    EXPECT_FALSE(HelperFunctions::getAndResetPipeError()) << "Pipe error occurred!";

    return;
}

// ====================================================================================================================
// Desc:       Read Server Subject Part (positive)
// Steps:      Read the certificate subject part of the connected server
// Exp Result: Subject part is read successfully
// ====================================================================================================================
TEST_F(General_TlsClient_Test_ReadServerData, PosTest_ReadServerSubjectPart)
{
    // Read server subject part
    string serverSubjectPart{tlsClient.getSubjPartFromServerCert(NID_localityName)};

    // Check the server subject part
    EXPECT_EQ(serverSubjectPart, "<my city>");

    return;
}

// ====================================================================================================================
// Desc:       Read Server Subject Part (negative)
// Steps:      Read the certificate subject part of a not connected server
// Exp Result: Exception is thrown
// ====================================================================================================================
TEST_F(General_TlsClient_Test_ReadServerData, NegTest_ReadServerSubjectPart)
{
    // Stop client to disconnect from server
    tlsClient.stop();

    // Read server subject part
    EXPECT_THROW(tlsClient.getSubjPartFromServerCert(NID_localityName), Client_error);

    return;
}

// ====================================================================================================================
// Desc:       Read Server Subject Part (negative)
// Steps:      Read the certificate subject part of the connected server with an invalid NID
// Exp Result: Exception is thrown
// ====================================================================================================================
TEST_F(General_TlsClient_Test_ReadServerData, NegTest_ReadServerSubjectPart_InvalidNid)
{
    // Read server subject part
    EXPECT_THROW(tlsClient.getSubjPartFromServerCert(NID_undef), Client_error);

    return;
}
