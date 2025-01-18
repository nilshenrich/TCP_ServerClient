#ifdef DEVELOP
#include <iostream>
#endif // DEVELOP

#include <sstream>
#include <functional>
#include <fstream>
#include <cstring>
#include <mutex>
#include <memory>
#include <cstdlib>
#include <ctime>

#include "FtpServer.hpp"
#include "../basic/algorithms.hpp"

using namespace ::std;
using namespace ::tcp;
using namespace ::ftp;
namespace fs = ::std::filesystem;

FtpServer::FtpServer() : tcpControl{'\n', "\r", MAXIMUM_MESSAGE_LENGTH},
                         work_checkUserCredentials{[](const string, const string) -> bool
                                                   { return false; }}, // Default: Refuse all user credentials
                         work_checkAccessible{[](const string, const string) -> bool
                                              { return false; }}, // Default: Refuse all paths
                         work_listDirectory{[](const string) -> valarray<Item>
                                            { return valarray<Item>{}; }}, // Default: Return empty directory
                         work_createDirectory{[](const string) -> bool
                                              { return false; }}, // Default: Refuse all directory creations
                         work_readFile{[](const string) -> istream *
                                       { return nullptr; }}, // Default: Return null-stream
                         work_writeFile{[](const string) -> ostream *
                                        { return nullptr; }} // Default: Return null-stream
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
void FtpServer::setWork_createDirectory(function<bool(const string)> worker) { work_createDirectory = worker; }
void FtpServer::setWork_readFile(function<istream *(const string)> worker) { work_readFile = worker; }
void FtpServer::setWork_writeFile(function<ostream *(const string)> worker) { work_writeFile = worker; }

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

string FtpServer::sanitizeRequest(const string &request) const
{
    // Illegale characters
    valarray<char> illegalChars{'\n', '\r'};
    char *iBegin{begin(illegalChars)};
    char *iEnd{end(illegalChars)};

    size_t lenMsg{request.size()};

    // Remove leading and trailing spaces and illegal characters
    string sReturn;
    sReturn.reserve(lenMsg);
    for (size_t i{0}; i < lenMsg; i += 1)
    {
        if ((i == 0 || i == lenMsg - 1) && request[i] == ' ')
            continue;

        if (find(iBegin, iEnd, request[i]) != iEnd)
            continue;

        sReturn += request[i];
    }

    return sReturn;
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
        session[clientId] = Session{}; // Create new session. Not logged in
    }
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::SUCCESS_WELCOME)) + " Welcome"s);
}
void FtpServer::on_msg(const int clientId, const string &msg)
{
    Reqp request{parseRequest(sanitizeRequest(msg))};
    switch (request.command)
    {
    case ENUM_CLASS_VALUE(Request::USERNAME):
        on_messageIn(clientId, request.command, request.args, 1, &FtpServer::on_msg_username, false);
        break;
    case ENUM_CLASS_VALUE(Request::PASSWORD):
        on_messageIn(clientId, request.command, request.args, 1, &FtpServer::on_msg_password, false);
        break;
    case ENUM_CLASS_VALUE(Request::SYSTEMTYPE):
        on_messageIn(clientId, request.command, request.args, 0, &FtpServer::on_msg_getSystemType);
        break;
    case ENUM_CLASS_VALUE(Request::FEATURES_LIST):
        on_messageIn(clientId, request.command, request.args, 0, &FtpServer::on_msg_listFeatures);
        break;
    case ENUM_CLASS_VALUE(Request::DIRECTORY_LIST):
        on_messageIn(clientId, request.command, request.args, 0, &FtpServer::on_msg_listDirectory);
        break;
    case ENUM_CLASS_VALUE(Request::DIRECTORY_CHANGE):
        on_messageIn(clientId, request.command, request.args, 1, &FtpServer::on_msg_changeDirectory);
        break;
    case ENUM_CLASS_VALUE(Request::DIRECTORY_GETCURRENT):
        on_messageIn(clientId, request.command, request.args, 0, &FtpServer::on_msg_getDirectory);
        break;
    case ENUM_CLASS_VALUE(Request::DIRECTORY_CREATE):
        on_messageIn(clientId, request.command, request.args, 1, &FtpServer::on_msg_createDirectory);
        break;
    case ENUM_CLASS_VALUE(Request::FILE_TRANSFER_TYPE):
        on_messageIn(clientId, request.command, request.args, 1, &FtpServer::on_msg_fileTransferType);
        break;
    case ENUM_CLASS_VALUE(Request::MODE_PASSIVE_ALL):   // Always enter passive mode
    case ENUM_CLASS_VALUE(Request::MODE_PASSIVE_SHORT): // Always enter passive mode
    case ENUM_CLASS_VALUE(Request::MODE_PASSIVE_LONG):  // Always enter passive mode
        on_messageIn(clientId, request.command, request.args, 0, &FtpServer::on_msg_modePassive);
        break;
    case ENUM_CLASS_VALUE(Request::FILE_DOWNLOAD):
        on_messageIn(clientId, request.command, request.args, 1, &FtpServer::on_msg_fileDownload);
        break;
    case ENUM_CLASS_VALUE(Request::FILE_UPLOAD):
        on_messageIn(clientId, request.command, request.args, 1, &FtpServer::on_msg_fileUpload);
        break;
    default:
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_NOTIMPLEMENTED)) + " Command not implemented."s);
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

void FtpServer::on_messageIn(const int clientId, const uint32_t command, const valarray<string> &args, size_t numArgsExp,
                             void (FtpServer::*work)(const int, const uint32_t, const valarray<string> &),
                             const bool mustLoggedIn)
{
    // Check if session exists
    bool connected;
    {
        lock_guard<mutex> lck{session_m};
        connected = session.find(clientId) != session.end();
    }
    if (!connected)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_UNKNOWN_ERROR)) + " Session not found."s);
        return;
    }

    // Check if user is logged in
    bool loggedIn;
    string username;
    {
        lock_guard<mutex> lck{session_m};
        loggedIn = session[clientId].loggedIn;
        username = session[clientId].username;
    }
    if (mustLoggedIn != loggedIn)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_WRONG_ORDER)) + (mustLoggedIn ? " User not logged in."s : " User already logged in."s));
        return;
    }

    // Check num of arguments
    size_t numArgs{args.size()};
    if (numArgsExp != numArgs)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_SYNTAX_ARGUMENT)) + " "s + to_string(numArgsExp) + " arguments expected, but "s + to_string(numArgs) + " given."s);
        return;
    }

    // Call worker method
    (this->*work)(clientId, command, args);
}

void FtpServer::on_msg_username(const int clientId, const uint32_t command, const valarray<string> &args)
{
    // Buffer login request. Override possible old session
    {
        lock_guard<mutex> lck{session_m};
        session[clientId] = Session{false, args[0], "/"}; // Set username but not logged in
    }
    // Request fine, require password
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::CONTINUE_PASSWORD_REQUIRED)) + " Password required for user "s + args[0] + "."s);
    return;
}

void FtpServer::on_msg_password(const int clientId, const uint32_t command, const valarray<string> &args)
{
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
            session[clientId] = Session{}; // Clear session
        }
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_LOGIN)) + " Login failed."s);
        return;
    }

    // Login success
    {
        lock_guard<mutex> lck{session_m};
        session[clientId] = Session{true, username, "/"};
    }
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::SUCCESS_LOGIN)) + " Login successful."s);
    return;
}

void FtpServer::on_msg_getSystemType(const int clientId, const uint32_t command, const valarray<string> &args)
{
#ifdef _WIN32
    string sysType{"WIN32"};
#elif _WIN64
    string sysType{"WINDOWS-NT"};
#elif __linux__
    string sysType{"LINUX"};
#elif __APPLE__ || __MACH__
    string sysType{"MACOS"};
#elif __FreeBSD__
    string sysType{"FREEBSD"};
#elif __unix__ || __unix
    string sysType{"UNIX"};
#else
    string sysType{"UNKNOWN"};
#endif

    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::SUCCESS_SYSTEMTYPE)) + " "s + sysType);
}

void FtpServer::on_msg_getDirectory(const int clientId, const uint32_t command, const valarray<string> &args)
{
    // Get current directory from session
    // BUG[performance]: Session could be deleted since existence check in on_messageIn
    string path;
    {
        lock_guard<mutex> lck{session_m};
        path = session[clientId].currentpath;
    }

    // Send current directory path to client
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::SUCCESS_DIRECTORY)) + " \""s + path + "\" is current directory."s);
    return;
}

void FtpServer::on_msg_changeDirectory(const int clientId, const uint32_t command, const valarray<string> &args)
{
    // Get user and current directory from session
    // BUG[performance]: Session could be deleted since existence check in on_messageIn
    string username;
    string path;
    {
        lock_guard<mutex> lck{session_m};
        username = session[clientId].username;
        path = session[clientId].currentpath;
    }

    // Determine requested absolute path
    string path_req;
    if (args[0].empty() || args[0][0] != '/') // Relative path
    {
        path_req = path + "/"s + args[0];
    }
    else // Absolute path
    {
        path_req = args[0];
    }

    // Check if path is accessible
    if (!work_checkAccessible(username, path_req))
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_FILENOTACCESSIBLE)) + " Requested directory is not accessible."s);
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
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::SUCCESS_ACTION)) + " Directory successfully changed."s);
    return;
}

void FtpServer::on_msg_fileTransferType(const int clientId, const uint32_t command, const valarray<string> &args)
{
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
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_ARGUMENT_NOTSUPPORTED)) + " Unsupported file transfer type."s);
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
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::OK)) + " Switching to "s + modename + " mode."s);
}

void FtpServer::on_msg_modePassive(const int clientId, const uint32_t command, const valarray<string> &args)
{
    // Get server IP address the client is connected to
    string myIp{tcpControl.getServerIp(clientId)};

    // Check IP type matches command
    // IPv4: [\d\.]+
    // IPv6: [0-9a-f:]+
    if (myIp.find_first_not_of("0123456789.") == string::npos && command == ENUM_CLASS_VALUE(Request::MODE_PASSIVE_LONG))
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_ARGUMENT_NOTSUPPORTED)) + " Extended mode not supported for IPv4."s);
        return;
    }
    if (myIp.find_first_not_of("0123456789abcdef:") == string::npos && command == ENUM_CLASS_VALUE(Request::MODE_PASSIVE_SHORT))
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_ARGUMENT_NOTSUPPORTED)) + " Short mode not supported for IPv6."s);
        return;
    }

    // Open data server on free port within range
    int port;
    unique_ptr<TcpServer> dataServer;
    {
        lock_guard<mutex> lck{tcpPort_m};
        port = getFreePort();
        if (port == -1)
        {
            tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_OPEN_DATACONN)) + " No free port."s);
            return;
        }

        // Create new data server and start listening on free port
        // Each session could have multiple data connections open at the same time (For transferring multiple files in parallel)
        dataServer.reset(new TcpServer()); // Continuous mode
        // FIXME: Don't start here directly but on concrete data transfer request
        if (dataServer->start(port) != SERVER_START_OK)
        {
            tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_OPEN_DATACONN)) + " Failed to open data connection."s);
            return;
        }
    }

    // Add data server to session and inform client
    // BUG[performance]: Session could be deleted since existence check in on_messageIn
    {
        lock_guard<mutex> lck{session_m};
        session[clientId].tcpData = move(dataServer); // FIXME: Unqualified move
    }
    string msg;
    Response responseCode;
    switch (command)
    {
    case ENUM_CLASS_VALUE(Request::MODE_PASSIVE_ALL):
        responseCode = Response::SUCCESS_PASSIVE_ALL;
        msg = "Entering Extended Passive Mode (|||"s + to_string(port) + "|)."s;
        break;
    case ENUM_CLASS_VALUE(Request::MODE_PASSIVE_SHORT):
        responseCode = Response::SUCCESS_PASSIVE_SHORT;
        algorithms::replace_allC(myIp, '.', ',');
        msg = "Entering Passive Mode ("s + myIp + ","s + to_string(port / 256) + ","s + to_string(port % 256) + ")."s;
        break;
    case ENUM_CLASS_VALUE(Request::MODE_PASSIVE_LONG):
        responseCode = Response::SUCCESS_PASSIVE_LONG;
        msg = "Entering Long Passive Mode ("s + myIp + ", "s + to_string(port) + ")."s;
        break;
    default: // Code never comes here
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_ARGUMENT_NOTSUPPORTED)) + " Unsupported passive mode."s);
        return;
    }

    // Inform client of new data server
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(responseCode)) + " "s + msg);
}

void FtpServer::on_msg_listDirectory(const int clientId, const uint32_t command, const valarray<string> &args)
{
    // Get user, current directory and data server from session
    // BUG[performance]: Session could be deleted since existence check in on_messageIn
    string username;
    string path;
    unique_ptr<TcpServer> dataServer;
    {
        lock_guard<mutex> lck{session_m};
        username = session[clientId].username;
        path = session[clientId].currentpath;
        dataServer = move(session[clientId].tcpData); // Remove data server from session as should be closed after this action // FIXME: Unqualified move
    }

    // Get directory list into string
    ostringstream msg;
    valarray<Item> items = work_listDirectory(username);
    size_t numItems{items.size()};
    for (size_t i{0}; i < numItems; i += 1)
    {
        msg << items[i] << endl;
    }

    // Wait here for data server to accept connection
    // FIXME: Not ideal performance
    // FIXME: Works just if no other data connections are open
    // FIXME: Add timeout
    vector<int> dataClients;
    while ((dataClients = dataServer->getAllClientIds()).empty())
        this_thread::sleep_for(chrono::milliseconds(10));

    // Send directory list to client
    // TODO: Send in chunks while retrieving, not all at once
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::SUCCESS_DATA_OPEN)) + " Here comes the directory listing."s);
    dataServer->sendMsg(dataClients[0], msg.str());
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::SUCCESS_DATA_CLOSE)) + " Directory send OK."s);
}

void FtpServer::on_msg_fileDownload(const int clientId, const uint32_t command, const valarray<string> &args)
{
    // Get user, current directory and data server from session
    // BUG[performance]: Session could be deleted since existence check in on_messageIn
    string username;
    string path;
    unique_ptr<TcpServer> dataServer;
    {
        lock_guard<mutex> lck{session_m};
        username = session[clientId].username;
        path = session[clientId].currentpath;
        dataServer = move(session[clientId].tcpData); // Remove data server from session as should be closed after this action // FIXME: Unqualified move
    }

    // Get stream to file that should be downloaded
    path += "/"s + args[0];
    istream *is{work_readFile(path)};

    // Wait here for data server to accept connection
    // FIXME: Not ideal performance
    // FIXME: Works just if no other data connections are open
    // FIXME: Add timeout
    vector<int> dataClients;
    while ((dataClients = dataServer->getAllClientIds()).empty())
        this_thread::sleep_for(chrono::milliseconds(10));

    // Send file content to client
    // TODO: Send in chunks while retrieving, not all at once
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::SUCCESS_DATA_OPEN)) + " Here comes the content of file "s + args[0] + "."s);
    string chunk{string(FILETRANSFER_CHUNKSIZE, 0)};
    while (!is->eof())
    {
        is->read(chunk.data(), FILETRANSFER_CHUNKSIZE);
        dataServer->sendMsg(dataClients[0], chunk.substr(0, is->gcount()));
    }
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::SUCCESS_DATA_CLOSE)) + " File send OK."s);
}

void FtpServer::on_msg_listFeatures(const int clientId, const uint32_t command, const valarray<string> &args)
{
    // Send feature list to client
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::SUCCESS_STATUS)) + "-Features:"s);
    for (const string &feature : features)
    {
        tcpControl.sendMsg(clientId, " "s + feature);
    }
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::SUCCESS_STATUS)) + " End of features."s);
    return;
}

void FtpServer::on_msg_createDirectory(const int clientId, const uint32_t command, const valarray<string> &args)
{
    // Get user and current directory from session
    // BUG[performance]: Session could be deleted since existence check in on_messageIn
    string username;
    string path;
    {
        lock_guard<mutex> lck{session_m};
        username = session[clientId].username;
        path = session[clientId].currentpath;
    }

    // Determine requested absolute path
    string path_req;
    if (args[0].empty() || args[0][0] != '/') // Relative path
    {
        path_req = path + "/"s + args[0];
    }
    else // Absolute path
    {
        path_req = args[0];
    }

    // Check if path is accessible
    if (!work_checkAccessible(username, path_req))
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_FILENOTACCESSIBLE)) + " Requested directory is not accessible."s);
        return;
    }

    // Create directory and send positive feedback
    if (!work_createDirectory(path_req))
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_FILENOTACCESSIBLE)) + " Failed to create directory."s);
        return;
    }
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::SUCCESS_DIRECTORY)) + " \""s + path_req + "\" created."s);
    return;
}

void FtpServer::on_msg_fileUpload(const int clientId, const uint32_t command, const valarray<string> &args)
{
    // Get user, current directory and data server from session
    // BUG[performance]: Session could be deleted since existence check in on_messageIn
    string username;
    string path;
    unique_ptr<TcpServer> dataServer;
    {
        lock_guard<mutex> lck{session_m};
        username = session[clientId].username;
        path = session[clientId].currentpath;
        dataServer = move(session[clientId].tcpData); // Remove data server from session as should be closed after this action // FIXME: Unqualified move
    }

    // Get stream to file that should be uploaded
    path += "/"s + args[0];
    ostream *os{work_writeFile(path)};

    // Forward all received data to file writer
    // BUG: Data server already running and might not be ready to accept data
    dataServer->setCreateForwardStream([os](const int clientID) -> ostream *
                                       { return os; });

    // FIXME: Block until data transfer is finished and then send success message
    return;
}
