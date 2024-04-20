#include "general/TcpServer_Test_Start.h"

using namespace std;
using namespace Test;
using namespace tcp;

General_TcpServer_Test_Start::General_TcpServer_Test_Start() {}
General_TcpServer_Test_Start::~General_TcpServer_Test_Start() {}

void General_TcpServer_Test_Start::SetUp()
{
    // Get free TCP port
    port = HelperFunctions::getFreePort();
    ASSERT_NE(port, -1) << "No free port found";
    return;
}

void General_TcpServer_Test_Start::TearDown()
{
    // Stop TCP server
    tcpServer.stop();

    // Check if no pipe error occurred
    EXPECT_FALSE(HelperFunctions::getAndResetPipeError()) << "Pipe error occurred!";

    return;
}

// ====================================================================================================================
// Desc:       Check if the TCP server starts properly
// Steps:      Start TCP server with correct parameters
// Exp Result: NETWORKLISTENER_START_OK
// ====================================================================================================================
TEST_F(General_TcpServer_Test_Start, PosTest)
{
    EXPECT_EQ(tcpServer.start(port), NETWORKLISTENER_START_OK);
}

// ====================================================================================================================
// Desc:       Check if server doesn't accept negative port number
// Steps:      Try to start TCP server with -1
// Exp Result: NETWORKLISTENER_ERROR_START_WRONG_PORT
// ====================================================================================================================
TEST_F(General_TcpServer_Test_Start, NegTest_WrongPort_Negative)
{
    EXPECT_EQ(tcpServer.start(-1), NETWORKLISTENER_ERROR_START_WRONG_PORT);
}

// ====================================================================================================================
// Desc:       Check if server doesn't accept too big port number
// Steps:      Try to start TCP server with 65536
// Exp Result: NETWORKLISTENER_ERROR_START_WRONG_PORT
// ====================================================================================================================
TEST_F(General_TcpServer_Test_Start, NegTest_WrongPort_TooBig)
{
    EXPECT_EQ(tcpServer.start(65536), NETWORKLISTENER_ERROR_START_WRONG_PORT);
}

// ====================================================================================================================
// Desc:       Check if server doesn't start if it is already running having no active connections
// Steps:      Start TCP server and then try to start it again on free port
// Exp Result: -1
// ====================================================================================================================
TEST_F(General_TcpServer_Test_Start, NegTest_AlreadyRunning_NoConnections)
{
    ASSERT_EQ(tcpServer.start(port), NETWORKLISTENER_START_OK);
    EXPECT_EQ(tcpServer.start(HelperFunctions::getFreePort()), -1);
}

// ====================================================================================================================
// Desc:       Check if server doesn't start if it is already running having 1 active connection
// Steps:      Start TCP server, connect a client and then try to start it again on free port
// Exp Result: -1
// ====================================================================================================================
TEST_F(General_TcpServer_Test_Start, NegTest_AlreadyRunning_OneConnection)
{
    ASSERT_EQ(tcpServer.start(port), NETWORKLISTENER_START_OK);
    TestApi::TcpClientApi_fragmentation tcpClient{};
    ASSERT_EQ(tcpClient.start("localhost", port), NETWORKCLIENT_START_OK);
    EXPECT_EQ(tcpServer.start(HelperFunctions::getFreePort()), -1);
}

// ====================================================================================================================
// Desc:       Check if server doesn't start if port is already in use
// Steps:      Start TCP server and try starting another instance with same port
// Exp Result: NETWORKLISTENER_ERROR_START_BIND_PORT
// ====================================================================================================================
TEST_F(General_TcpServer_Test_Start, NegTest_PortInUse)
{
    ASSERT_EQ(tcpServer.start(port), NETWORKLISTENER_START_OK);
    TestApi::TcpServerApi_fragmentation tcpServer2{};
    EXPECT_EQ(tcpServer2.start(port), NETWORKLISTENER_ERROR_START_BIND_PORT);
}
