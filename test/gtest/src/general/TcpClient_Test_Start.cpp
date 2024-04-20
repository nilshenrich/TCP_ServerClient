#include "general/TcpClient_Test_Start.h"

using namespace std;
using namespace Test;
using namespace tcp;

General_TcpClient_Test_Start::General_TcpClient_Test_Start() {}
General_TcpClient_Test_Start::~General_TcpClient_Test_Start() {}

void General_TcpClient_Test_Start::SetUp()
{
    // Get free TCP port
    port = HelperFunctions::getFreePort();
    ASSERT_NE(port, -1) << "No free port found";
    ASSERT_EQ(tcpServer.start(port), SERVER_START_OK);
    return;
}

void General_TcpClient_Test_Start::TearDown()
{
    // Stop TCP server and client
    tcpClient.stop();
    tcpServer.stop();

    // Check if no pipe error occurred
    EXPECT_FALSE(HelperFunctions::getAndResetPipeError()) << "Pipe error occurred!";

    return;
}

// ====================================================================================================================
// Desc:       Check if the TCP client starts properly
// Steps:      Start TCP client with correct parameters
// Exp Result: CLIENT_START_OK
// ====================================================================================================================
TEST_F(General_TcpClient_Test_Start, PosTest)
{
    EXPECT_EQ(tcpClient.start("localhost", port), CLIENT_START_OK);
    EXPECT_EQ(tcpServer.getClientIds().size(), 1);
}

// ====================================================================================================================
// Desc:       Check if client doesn't accept negative port number
// Steps:      Try to start TCP client with -1
// Exp Result: CLIENT_ERROR_START_WRONG_PORT
// ====================================================================================================================
TEST_F(General_TcpClient_Test_Start, NegTest_WrongPort_Negative)
{
    EXPECT_EQ(tcpClient.start("localhost", -1), CLIENT_ERROR_START_WRONG_PORT);
    EXPECT_EQ(tcpServer.getClientIds().size(), 0);
}

// ====================================================================================================================
// Desc:       Check if client doesn't accept too big port number
// Steps:      Try to start TCP client with 65536
// Exp Result: CLIENT_ERROR_START_WRONG_PORT
// ====================================================================================================================
TEST_F(General_TcpClient_Test_Start, NegTest_WrongPort_TooBig)
{
    EXPECT_EQ(tcpClient.start("localhost", 65536), CLIENT_ERROR_START_WRONG_PORT);
    EXPECT_EQ(tcpServer.getClientIds().size(), 0);
}

// ====================================================================================================================
// Desc:       Check if client stops properly when server is not running
// Steps:      Try to start TCP client after stopping server
// Exp Result: CLIENT_ERROR_START_CONNECT
// ====================================================================================================================
TEST_F(General_TcpClient_Test_Start, NegTest_ServerNotRunning)
{
    tcpServer.stop();
    EXPECT_EQ(tcpClient.start("localhost", port), CLIENT_ERROR_START_CONNECT);
    EXPECT_EQ(tcpServer.getClientIds().size(), 0);
}

// ====================================================================================================================
// Desc:       Check if client doesn't start if it is already running
// Steps:      Start TCP client and then try to start it again
// Exp Result: -1
// ====================================================================================================================
TEST_F(General_TcpClient_Test_Start, NegTest_AlreadyRunning)
{
    ASSERT_EQ(tcpClient.start("localhost", port), CLIENT_START_OK);
    EXPECT_EQ(tcpClient.start("localhost", port), -1);
    EXPECT_EQ(tcpServer.getClientIds().size(), 1);
}
