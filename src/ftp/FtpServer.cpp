#include "FtpServer.hpp"

using namespace ::std;
using namespace ::tcp;
using namespace ::ftp;

FtpServer::FtpServer() : tcpControl{'\n', MAXIMUM_MESSAGE_LENGTH},
                         work_checkUserCredentials{[](const string, const string) -> bool
                                                   { return false; }}, // Default: Refuse all user credentials
                         work_checkAccessible{[](const string, const string) -> bool
                                              { return false; }}, // Default: Refuse all paths
                         work_listDirectory{[](const string) -> valarray<Item>
                                            { return valarray<Item>{}; }}, // Default: Return empty directory
                         work_readFile{[](const string) -> ifstream
                                       { return ifstream{}; }} // Default: Return null-stream
{
    // Initialize random number generator
    srand((unsigned int)time(nullptr));

    // Link TCP server worker methods to provide FTP server functionality
    tcpControl.setWorkOnEstablished(bind(&FtpServer::on_newClient, this, placeholders::_1));
    tcpControl.setWorkOnMessage(bind(&FtpServer::on_msg, this, placeholders::_1, placeholders::_2));
    tcpControl.setWorkOnClosed(bind(&FtpServer::on_closed, this, placeholders::_1));
}
FtpServer::~FtpServer() { stop(); }

int FtpServer::start() { return tcpControl.start(PORT_CONTROL); }
void FtpServer::stop() { tcpControl.stop(); }

void FtpServer::setWork_checkUserCredentials(function<bool(const string, const string)> worker) { work_checkUserCredentials = worker; }
void FtpServer::setWork_checkAccessible(function<bool(const string, const string)> worker) { work_checkAccessible = worker; }
void FtpServer::setWork_listDirectory(function<valarray<Item>(const string)> worker) { work_listDirectory = worker; }
void FtpServer::setWork_readFile(function<ifstream(const string)> worker) { work_readFile = worker; }

bool FtpServer::isRunning() const { return tcpControl.isRunning(); }

Reqp FtpServer::parseRequest(const string &msg) const
{
    // First word is the command with 3-4 bytes
    // Following words are arguments separated by spaces

    // Get all space positions and end of string
    size_t lenMsg{msg.size()};
    vector<size_t> posSpaces;
    posSpaces.reserve(lenMsg + 1);
    for (size_t i = 0; i < lenMsg; i += 1)
    {
        if (msg[i] == ' ')
        {
            posSpaces.push_back(i);
        }
    }
    size_t numArgs{posSpaces.size()};
    posSpaces.push_back(msg.size());

    // Extract command and arguments from between spaces
    valarray<string> args{numArgs};
    for (size_t i{0}; i < numArgs; i += 1)
    {
        args[i] = msg.substr(posSpaces[i] + 1, posSpaces[i + 1] - posSpaces[i] - 1);
    }
    return Reqp{hashCommand(msg.substr(0, posSpaces[0]).c_str()), args};
}

int FtpServer::getFreePort() const
{
    // First get rabdom number inside port range
    // Then check if port is in use
    //     -> If not, use it
    //     -> If yes, try next number

    int port{rand() % (PORT_RANGE_DATA[1] - PORT_RANGE_DATA[0]) + PORT_RANGE_DATA[0]};
    for (int i{0}; i <= PORT_RANGE_DATA[1] - PORT_RANGE_DATA[0]; i += 1)
    {
        port += 1;
        if (port > PORT_RANGE_DATA[1])
            port = PORT_RANGE_DATA[0];

        int sock{socket(AF_INET, SOCK_STREAM, 0)};
        if (-1 == sock)
            return -1;

        struct sockaddr_in sin;
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons(port);

        if (!bind(sock, (struct sockaddr *)&sin, sizeof(sin)))
        {
            close(sock);
            return port;
        }
    }

    // If we get here, no free port was found. Return -1.
    return -1;
}

void FtpServer::on_newClient(const int clientId)
{
    {
        lock_guard<mutex> lck{session_m};
        session[clientId] = Session{false, "", ""}; // Create new session. Not logged in
    }
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::SUCCESS_WELCOME)) + " Welcome");
}
void FtpServer::on_msg(const int clientId, const string &msg)
{
    Reqp request{parseRequest(msg)};
    switch (request.command)
    {
    case ENUM_CLASS_VALUE(Request::USERNAME):
        on_msg_username(clientId, request.command, request.args);
        break;
    case ENUM_CLASS_VALUE(Request::PASSWORD):
        on_msg_password(clientId, request.command, request.args);
        break;
    case ENUM_CLASS_VALUE(Request::LIST_DIR):
        on_msg_listDirectory(clientId, request.command, request.args);
        break;
    case ENUM_CLASS_VALUE(Request::CHANGE_DIR):
        on_msg_changeDirectory(clientId, request.command, request.args);
        break;
    case ENUM_CLASS_VALUE(Request::GET_DIR):
        on_msg_getDirectory(clientId, request.command, request.args);
        break;
    case ENUM_CLASS_VALUE(Request::FILE_TRANSFER_TYPE):
        on_msg_fileTransferType(clientId, request.command, request.args);
        break;
    case ENUM_CLASS_VALUE(Request::MODE_PASSIVE_ALL):   // Always enter passive mode
    case ENUM_CLASS_VALUE(Request::MODE_PASSIVE_SHORT): // Always enter passive mode
    case ENUM_CLASS_VALUE(Request::MODE_PASSIVE_LONG):  // Always enter passive mode
        on_msg_modePassive(clientId, request.command, request.args);
        break;
    case ENUM_CLASS_VALUE(Request::FILE_DOWNLOAD):
        on_msg_fileDownload(clientId, request.command, request.args);
        break;
    default:
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_NOTIMPLEMENTED)) + " Command not implemented.");
        break;
    }
}
void FtpServer::on_closed(const int clientId)
{
    // Logout session
    lock_guard<mutex> lck{session_m};
    session.erase(clientId);
    return;
}

//////////////////////////////////////////////////
// Worker mehods on incoming messages
//////////////////////////////////////////////////

void FtpServer::on_msg_username(const int clientId, const uint32_t command, const valarray<string> &args)
{
    // Check if session exists
    bool connected;
    {
        lock_guard<mutex> lck{session_m};
        connected = session.find(clientId) != session.end();
    }
    if (!connected)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_UNKNOWN_ERROR)) + " Session not found.");
        return;
    }

    // Check num of arguments
    size_t numArgs{args.size()};
    if (numArgs != 1)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_SYNTAX_ARGUMENT)) + " 1 argument expected, but " + to_string(numArgs) + " given.");
        return;
    }

    // Buffer login request. Override possible old session
    {
        lock_guard<mutex> lck{session_m};
        session[clientId] = Session{false, args[0], "/"}; // Set username but not logged in
    }
    // Request fine, require password
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::CONTINUE_PASSWORD_REQUIRED)) + " Password required for user " + args[0] + ".");
    return;
}

void FtpServer::on_msg_password(const int clientId, const uint32_t command, const valarray<string> &args)
{
    // Check if session exists
    bool connected;
    {
        lock_guard<mutex> lck{session_m};
        connected = session.find(clientId) != session.end();
    }
    if (!connected)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_UNKNOWN_ERROR)) + " Session not found.");
        return;
    }

    // Check num of arguments
    size_t numArgs{args.size()};
    if (numArgs != 1)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_SYNTAX_ARGUMENT)) + " 1 argument expected, but " + to_string(numArgs) + " given.");
        return;
    }

    // Check user credentials
    string username;
    {
        lock_guard<mutex> lck{session_m};
        username = session[clientId].username;
    }
    if (!work_checkUserCredentials(username, args[0]))
    {
        {
            lock_guard<mutex> lck{session_m};
            session[clientId] = Session{false, "", ""}; // Clear session
        }
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_LOGIN)) + " Login failed.");
        return;
    }

    // Login success
    {
        lock_guard<mutex> lck{session_m};
        session[clientId] = Session{true, move(username), "/"};
    }
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::SUCCESS_LOGIN)) + " Login successful.");
    return;
}

void FtpServer::on_msg_getDirectory(const int clientId, const uint32_t command, const valarray<string> &args)
{
    // Check if session exists
    bool connected;
    {
        lock_guard<mutex> lck{session_m};
        connected = session.find(clientId) != session.end();
    }
    if (!connected)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_UNKNOWN_ERROR)) + " Session not found.");
        return;
    }

    // Check if user is logged in
    bool loggedIn;
    string username;
    string path;
    {
        lock_guard<mutex> lck{session_m};
        loggedIn = session[clientId].loggedIn;
        username = session[clientId].username;
        path = session[clientId].currentpath;
    }
    if (!loggedIn)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_WRONG_ORDER)) + " User not logged in.");
        return;
    }

    // Check num of arguments
    size_t numArgs{args.size()};
    if (numArgs != 0)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_SYNTAX_ARGUMENT)) + " 0 arguments expected, but " + to_string(numArgs) + " given.");
        return;
    }

    // Send current directory path to client
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::CURRENT_PATH)) + " \"" + path + "\" is current directory.");
    return;
}

void FtpServer::on_msg_changeDirectory(const int clientId, const uint32_t command, const valarray<string> &args)
{
    // Check if session exists
    bool connected;
    {
        lock_guard<mutex> lck{session_m};
        connected = session.find(clientId) != session.end();
    }
    if (!connected)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_UNKNOWN_ERROR)) + " Session not found.");
        return;
    }

    // Check if user is logged in
    bool loggedIn;
    string username;
    string path;
    {
        lock_guard<mutex> lck{session_m};
        loggedIn = session[clientId].loggedIn;
        username = session[clientId].username;
        path = session[clientId].currentpath;
    }
    if (!loggedIn)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_WRONG_ORDER)) + " User not logged in.");
        return;
    }

    // Check num of arguments
    size_t numArgs{args.size()};
    if (numArgs != 1)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_SYNTAX_ARGUMENT)) + " 1 argument expected, but " + to_string(numArgs) + " given.");
        return;
    }

    // Determine requeted absolute path
    string path_req;
    if (args[0].empty() || args[0][0] != '/') // Relative path
    {
        path_req = path + "/" + args[0];
    }
    else // Absolute path
    {
        path_req = args[0];
    }

    // Check if path is accessible
    if (!work_checkAccessible(username, path_req))
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_FILENOTACCESSIBLE)) + " Requested directory is not accessible.");
        return;
    }

    // Change directory and send positive feedback
    {
        lock_guard<mutex> lck{session_m};
        if (session.find(clientId) == session.end()) // If session doesn't exist anymore, abort process
        {
            return;
        }
        session[clientId].currentpath = path_req;
    }
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::SUCCESS_ACTION)) + " Directory successfully changed.");
    return;
}

void FtpServer::on_msg_fileTransferType(const int clientId, const uint32_t command, const valarray<string> &args)
{
    // Check if session exists
    bool connected;
    {
        lock_guard<mutex> lck{session_m};
        connected = session.find(clientId) != session.end();
    }
    if (!connected)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_UNKNOWN_ERROR)) + " Session not found.");
        return;
    }

    // Check if user is logged in
    bool loggedIn;
    {
        lock_guard<mutex> lck{session_m};
        loggedIn = session[clientId].loggedIn;
    }
    if (!loggedIn)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_WRONG_ORDER)) + " User not logged in.");
        return;
    }

    // Check num of arguments
    size_t numArgs{args.size()};
    if (numArgs != 1)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_SYNTAX_ARGUMENT)) + " 1 argument expected, but " + to_string(numArgs) + " given.");
        return;
    }

    // Set file transfer type for user
    string modename;
    switch (args[0][0])
    {
    case ENUM_CLASS_VALUE(FileTransferType::ASCII):
        modename = "ASCII";
        break;
    case ENUM_CLASS_VALUE(FileTransferType::BINARY):
        modename = "BINARY";
        break;
    case ENUM_CLASS_VALUE(FileTransferType::UNICODE):
        modename = "UTF-8";
        break;
    default:
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_ARGUMENT_NOTSUPPORTED)) + " Unsupported file transfer type.");
        return;
    }

    {
        lock_guard<mutex> lck{session_m};
        if (session.find(clientId) == session.end()) // If session doesn't exist anymore, abort process
        {
            return;
        }
        session[clientId].mode = args[0][0];
    }
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::OK)) + " Switching to " + modename + " mode.");
}

void FtpServer::on_msg_modePassive(const int clientId, const uint32_t command, const valarray<string> &args)
{
    // Check if session exists
    bool connected;
    {
        lock_guard<mutex> lck{session_m};
        connected = session.find(clientId) != session.end();
    }
    if (!connected)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_UNKNOWN_ERROR)) + " Session not found.");
        return;
    }

    // Check if user is logged in
    bool loggedIn;
    {
        lock_guard<mutex> lck{session_m};
        loggedIn = session[clientId].loggedIn;
    }
    if (!loggedIn)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_WRONG_ORDER)) + " User not logged in.");
        return;
    }

    // Check num of arguments
    size_t numArgs{args.size()};
    if (numArgs != 0)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_SYNTAX_ARGUMENT)) + " 0 arguments expected, but " + to_string(numArgs) + " given.");
        return;
    }

    // Type of passive mode
    string modename;
    string myIp{tcpControl.getServerIp(clientId)};
    switch (command)
    {
    case ENUM_CLASS_VALUE(Request::MODE_PASSIVE_ALL):
        modename = "Extended";
        break;
    case ENUM_CLASS_VALUE(Request::MODE_PASSIVE_SHORT):
        modename = "";
        break;
    case ENUM_CLASS_VALUE(Request::MODE_PASSIVE_LONG):
        modename = "Long";
        break;
    default:
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_ARGUMENT_NOTSUPPORTED)) + " Unsupported mode.");
        return;
    }

    // Get server IP address the client is connected to
    string serverIp{tcpControl.getServerIp(clientId)};

    // Check IP type matches command
    // IPv4: [\d\.]+
    // IPv6: [0-9a-f:]+
    if (serverIp.find_first_not_of("0123456789.") == string::npos && command == ENUM_CLASS_VALUE(Request::MODE_PASSIVE_LONG))
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_ARGUMENT_NOTSUPPORTED)) + " " + modename + " mode not supported for IPv4.");
        return;
    }
    if (serverIp.find_first_not_of("0123456789abcdef:") == string::npos && command == ENUM_CLASS_VALUE(Request::MODE_PASSIVE_SHORT))
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_ARGUMENT_NOTSUPPORTED)) + " " + modename + " mode not supported for IPv6.");
        return;
    }

    // Open data server on free port within range
    int port{getFreePort()};
    if (port == -1)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_OPEN_DATACONN)) + " No free port.");
        return;
    }

    // TODO: Create new data server on port and tell client about it
}
