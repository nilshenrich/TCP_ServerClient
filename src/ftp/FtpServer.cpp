#include "FtpServer.hpp"

using namespace ::std;
using namespace ::tcp;
using namespace ::ftp;

FtpServer::FtpServer() : tcpControl{'\n', MAXIMUM_MESSAGE_LENGTH},
                         work_checkUserCredentials{[](const string, const string) -> bool
                                                   { return false; }}, // Default: Refuse all user credentials
                         work_checkAccessible{[](const string) -> bool
                                              { return false; }}, // Default: Refuse all paths
                         work_listDirectory{[](const string) -> valarray<Item>
                                            { return valarray<Item>{}; }}, // Default: Return empty directory
                         work_readFile{[](const string) -> ifstream
                                       { return ifstream{}; }} // Default: Return null-stream
{
    // Link TCP server worker methods to provide FTP server functionality
    tcpControl.setWorkOnEstablished(bind(&FtpServer::on_newClient, this, placeholders::_1));
    tcpControl.setWorkOnMessage(bind(&FtpServer::on_msg, this, placeholders::_1, placeholders::_2));
    tcpControl.setWorkOnClosed(bind(&FtpServer::on_closed, this, placeholders::_1));
}
FtpServer::~FtpServer() { stop(); }

int FtpServer::start() { return tcpControl.start(PORT_CONTROL); }
void FtpServer::stop() { tcpControl.stop(); }

void FtpServer::setWork_checkUserCredentials(function<bool(const string, const string)> worker) { work_checkUserCredentials = worker; }
void FtpServer::setWork_checkAccessible(function<bool(const string)> worker) { work_checkAccessible = worker; }
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
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_NOTIMPLEMENTED)) + " Command not implemented");
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
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_UNKNOWN_ERROR)) + " Session not found");
        return;
    }

    // Check num of arguments
    size_t numArgs{args.size()};
    if (numArgs != 1)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_SYNTAX_ARGUMENT)) + " 1 argument expected, but " + to_string(numArgs) + " given");
        return;
    }

    // Buffer login request. Override possible old session
    {
        lock_guard<mutex> lck{session_m};
        session[clientId] = Session{false, args[0], "/"}; // Set username but not logged in
    }
    // Request fine, require password
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::CONTINUE_PASSWORD_REQUIRED)) + " Password required for user " + args[0]);
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
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_UNKNOWN_ERROR)) + " Session not found");
        return;
    }

    // Check num of arguments
    size_t numArgs{args.size()};
    if (numArgs != 1)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_SYNTAX_ARGUMENT)) + " 1 argument expected, but " + to_string(numArgs) + " given");
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
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_LOGIN)) + " Login failed");
        return;
    }

    // Login success
    {
        lock_guard<mutex> lck{session_m};
        session[clientId] = Session{true, move(username), "/"};
    }
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::SUCCESS_LOGIN)) + " Login successful");
    return;
}

void FtpServer::on_msg_listDirectory(const int clientId, const uint32_t command, const valarray<string> &args)
{
    // Check if session exists
    bool connected;
    {
        lock_guard<mutex> lck{session_m};
        connected = session.find(clientId) != session.end();
    }
    if (!connected)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_UNKNOWN_ERROR)) + " Session not found");
        return;
    }

    // Check num of arguments
    size_t numArgs{args.size()};
    if (numArgs != 0)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_SYNTAX_ARGUMENT)) + " 0 arguments expected, but " + to_string(numArgs) + " given");
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
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_LOGIN)) + " Not logged in");
        return;
    }

    // List directory
    // TODO: Open data stream and send dir list
}
