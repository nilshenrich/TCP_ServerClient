#ifdef DEVELOP
#include <iostream>
#endif // DEVELOP

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <memory>
#include <sstream>
#include <vector>

#include "../basic/algorithms.hpp"
#include "FtpServer.hpp"

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
                         work_readFile{[](const string, const ios::openmode) -> istream *
                                       { return nullptr; }}, // Default: Return null-stream
                         work_writeFile{[](const string, const ios::openmode) -> ostream *
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
void FtpServer::setWork_readFile(function<istream *(const string, const ios::openmode)> worker) { work_readFile = worker; }
void FtpServer::setWork_writeFile(function<ostream *(const string, const ios::openmode)> worker) { work_writeFile = worker; }

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
    // Illegal characters
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

ios::openmode FtpServer::getStreamOpenMode(const bool direction, const ::std::underlying_type_t<FileTransferType> transferType) const
{
    switch (transferType)
    {
    case ENUM_CLASS_VALUE(FileTransferType::ASCII):
        return (direction == STREAM_DIRECTION_READ) ? STREAM_OPEN_MODE_READ_ASCII : STREAM_OPEN_MODE_WRITE_ASCII;
    case ENUM_CLASS_VALUE(FileTransferType::UNICODE):
        return (direction == STREAM_DIRECTION_READ) ? STREAM_OPEN_MODE_READ_UNICODE : STREAM_OPEN_MODE_WRITE_UNICODE;
    case ENUM_CLASS_VALUE(FileTransferType::BINARY):
        return (direction == STREAM_DIRECTION_READ) ? STREAM_OPEN_MODE_READ_BINARY : STREAM_OPEN_MODE_WRITE_BINARY;
    default:
        throw Server_error("Invalid file transfer type: "s + to_string(transferType));
    }
}

void FtpServer::on_newClient(const int clientId)
{
    {
        unique_ptr<Session> sessionNew{make_unique<Session>()}; // Create new session. Not logged in
        unique_lock<shared_mutex> lck_session{session_m};       // Create: Block simultaneous actions on session map
        activeSessions.insert_or_assign(clientId, move(sessionNew));
    }
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::SUCCESS_WELCOME)) + " Welcome"s);
    return;
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
    return;
}
void FtpServer::on_closed(const int clientId)
{
    // Logout session by erasing it
    unique_lock<shared_mutex> lck_session{session_m}; // Delete: Block simultaneous actions on session map
    activeSessions.erase(clientId);
    return;
}

//////////////////////////////////////////////////
// Worker mehods on incoming messages
//////////////////////////////////////////////////

void FtpServer::on_messageIn(const int clientId, const uint32_t command, const valarray<string> &args, size_t numArgsExp,
                             void (FtpServer::*work)(const int, const uint32_t, const valarray<string> &),
                             const bool mustLoggedIn)
{
    // Check if user is logged in
    {
        bool loggedIn;
        unique_ptr<shared_lock<shared_mutex>> lck_session_modify;
        try
        {
            shared_lock<shared_mutex> lck_session{session_m}; // Read: Allow simultaneous actions on session map
            unique_ptr<Session> &session{activeSessions.at(clientId)};
            lck_session_modify = make_unique<shared_lock<shared_mutex>>(session->modify_m); // Read: Allow simultaneous actions on session data
            loggedIn = session->loggedIn;
        }
        catch (const out_of_range &)
        {
            tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_UNKNOWN_ERROR)) + " Session not found."s);
            return;
        }
        if (mustLoggedIn != loggedIn)
        {
            tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_WRONG_ORDER)) + (mustLoggedIn ? " User not logged in."s : " User already logged in."s));
            return;
        }
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
    return;
}

void FtpServer::on_msg_username(const int clientId, const uint32_t command, const valarray<string> &args)
{
    // Buffer login request. Override possible old session
    const string &username{args[0]};
    {
        unique_ptr<Session> sessionNew{make_unique<Session>(false, username, "/")}; // Not logged in yet
        shared_lock<shared_mutex> lck_session{session_m};                           // Modify: Allow simultaneous actions on session map
        activeSessions.at(clientId) = move(sessionNew);
    }
    // Request fine, require password
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::CONTINUE_PASSWORD_REQUIRED)) + " Password required for user "s + args[0] + "."s);
    return;
}

void FtpServer::on_msg_password(const int clientId, const uint32_t command, const valarray<string> &args)
{
    const string &password{args[0]};
    string response;
    {
        shared_lock<shared_mutex> lck_session{session_m}; // Read: Allow simultaneous actions on session map
        unique_ptr<Session> &session{activeSessions.at(clientId)};
        shared_lock<shared_mutex> lck_session_modify{session->modify_m}; // Read: Allow simultaneous actions on session data
        const string &username{session->username};

        // Check user credentials
        if (work_checkUserCredentials(username, password))
        {
            session.reset(new Session(true, username, "/")); // Set username and logged in
            response = to_string(ENUM_CLASS_VALUE(Response::SUCCESS_LOGIN)) + " Login successful."s;
        }
        else
        {
            session.reset(new Session()); // Clear session
            response = to_string(ENUM_CLASS_VALUE(Response::FAILED_LOGIN)) + " Login failed."s;
        }
    }
    tcpControl.sendMsg(clientId, response);
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
    return;
}

void FtpServer::on_msg_getDirectory(const int clientId, const uint32_t command, const valarray<string> &args)
{
    string response;
    {
        shared_lock<shared_mutex> lck_session{session_m}; // Read: Allow simultaneous actions on session map
        unique_ptr<Session> &session{activeSessions.at(clientId)};
        shared_lock<shared_mutex> lck_session_modify{session->modify_m}; // Read: Allow simultaneous actions on session data
        const string &path{session->currentpath};
        response = to_string(ENUM_CLASS_VALUE(Response::SUCCESS_DIRECTORY)) + " \""s + path + "\" is current directory."s;
    }
    tcpControl.sendMsg(clientId, response);
}

void FtpServer::on_msg_changeDirectory(const int clientId, const uint32_t command, const valarray<string> &args)
{
    const string &path_req{args[0]};
    bool accessible;
    {
        string path_new;
        shared_lock<shared_mutex> lck_session{session_m}; // Read: Allow simultaneous actions on session map
        unique_ptr<Session> &session{activeSessions.at(clientId)};
        shared_lock<shared_mutex> lck_session_modify{session->modify_m}; // Read: Allow simultaneous actions on session data
        const string &username{session->username};
        string &path{session->currentpath};
        if (path_req.empty() || path_req[0] != '/') // Relative path
            path_new = path + "/"s + path_req;      // FIXME: .. is just appended, so the path always grows
        else // Absolute path
            path_new = path_req;

        accessible = work_checkAccessible(username, path_new);
        if (accessible)
        {
            lck_session_modify.unlock();
            unique_lock<shared_mutex> lck_session_modify_unique{session->modify_m}; // Modify: Block simultaneous actions on session data
            path = path_new;                                                        // Set new current path in session
        }
    }

    string response;
    if (accessible)
        response = to_string(ENUM_CLASS_VALUE(Response::SUCCESS_ACTION)) + " Directory successfully changed."s;
    else
        response = to_string(ENUM_CLASS_VALUE(Response::FAILED_FILENOTACCESSIBLE)) + " Requested directory is not accessible."s;
    tcpControl.sendMsg(clientId, response);
}

void FtpServer::on_msg_fileTransferType(const int clientId, const uint32_t command, const valarray<string> &args)
{
    // Get requested transfer type
    const string &arg1{args[0]};
    if (arg1.size() != 1)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_SYNTAX_ARGUMENT)) + " Exactly one character expected as file transfer type."s);
        return;
    }

    // Set file transfer type for user
    const char &transferType{arg1[0]};
    string modename;
    switch (transferType)
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
        shared_lock<shared_mutex> lck_session{session_m}; // Modify: Allow simultaneous actions on session map
        unique_ptr<Session> &session{activeSessions.at(clientId)};
        unique_lock<shared_mutex> lck_session_modify{session->modify_m}; // Modify: Block simultaneous actions on session data
        session->transferType = transferType;
    }
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::OK)) + " Switching to "s + modename + " mode."s);
    return;
}

void FtpServer::on_msg_modePassive(const int clientId, const uint32_t command, const valarray<string> &args)
{
    // Get server IP address the client is connected to
    string myIp;
    try
    {
        myIp = tcpControl.getServerIp(clientId);
    }
    catch (Server_error &e)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_UNKNOWN_ERROR)) + " Failed to determine control connection details."s);
        return;
    }

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

    // Get file transfer type from session
    underlying_type_t<FileTransferType> transferType;
    {
        shared_lock<shared_mutex> lck_session{session_m}; // Read: Allow simultaneous actions on session map
        shared_lock<shared_mutex> lck_session_modify{activeSessions.at(clientId)->modify_m}; // Read: Allow simultaneous actions on session data
        unique_ptr<Session> &session{activeSessions.at(clientId)};
        transferType = session->transferType;
    }

    // If not transfer type is specified, return with error code
    if (transferType == ENUM_CLASS_VALUE(FileTransferType::INVALID))
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_WRONG_ORDER)) + " File transfer type must be specified first."s);
        return;
    }

    // Open data server on free port within range
    int port;
    {
        lock_guard<mutex> lck_port{tcpPort_m};
        try
        {
            port = algorithms::getFreePort(PORT_RANGE_DATA[0], PORT_RANGE_DATA[1]);
        }
        catch (const Error &e)
        {
            tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_OPEN_DATACONN)) + " No free TCP port."s);
            return;
        }

        // Create new data server and start listening on free port
        // All incoming data is forwarded to stream to temporary buffer
        shared_lock<shared_mutex> lck_session{session_m}; // Modify: Allow simultaneous actions on session map
        unique_ptr<Session> &session{activeSessions.at(clientId)};
        shared_lock<shared_mutex> lck_session_modify{session->modify_m}; // Read: Allow simultaneous actions on session data
        int *p_dataClientId{&session->dataClientId};
        DynamicOstream<STREAM_DYNAMICOSTREAM_BUFFERSIZE> **pp_incomingStreamFwd{&session->incomingStreamFwd};
        shared_mutex *p_session_m{&session_m};
        mutex *p_established_m{&session->established_m};
        mutex *p_processed_m{&session->processed_m};
        mutex *p_closed_m{&session->closed_m};
        lck_session_modify.unlock();
        unique_lock<shared_mutex> lck_session_modify_unique{session->modify_m}; // Modify: Block simultaneous actions on session data
        session->tcpData.reset(nullptr);                            // Clear old data server if existing
        session->dataClientId = -1;                                 // Reset data client ID
        unique_ptr<TcpServer> dataServer{make_unique<TcpServer>()}; // Create new data server in continuous mode
        dataServer->setCreateForwardStream([pp_incomingStreamFwd, p_established_m, p_session_m, p_dataClientId](const int dataClientId) -> DynamicOstream<STREAM_DYNAMICOSTREAM_BUFFERSIZE> *
                                           {
                                               shared_lock<shared_mutex> lck_session{*p_session_m}; // Modify: Allow simultaneous actions on session map
                                               *p_dataClientId = dataClientId;
                                               *pp_incomingStreamFwd = new DynamicOstream<STREAM_DYNAMICOSTREAM_BUFFERSIZE>();
                                               p_established_m->unlock();
                                               return *pp_incomingStreamFwd; //
                                           });
        dataServer->setWorkOnClosed([p_processed_m, p_closed_m, p_session_m](const int dataClientId) -> void
                                    {
                                        shared_lock<shared_mutex> lck_session{*p_session_m}; // Modify: Allow simultaneous actions on session map
                                        p_processed_m->lock();                               // Wait here until data transfer is processed
                                        p_closed_m->unlock();                                //
                                    });
        if (dataServer->start(port, 1) != SERVER_START_OK)
        {
            tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::FAILED_OPEN_DATACONN)) + " Failed to open data connection."s);
            return;
        }
        bool _;                          // Dummy variable to suppress unused variable warning
        _ = p_established_m->try_lock(); // Lock mutex until stream is created in lambda
        _ = p_processed_m->try_lock();   // Lock mutex until data transfer is processed in lambda
        _ = p_closed_m->try_lock();      // Lock mutex until connection is closed in lambda
        session->tcpData = move(dataServer);
    }

    string responseMessage;
    Response responseCode;
    switch (command)
    {
    case ENUM_CLASS_VALUE(Request::MODE_PASSIVE_ALL):
        responseCode = Response::SUCCESS_PASSIVE_ALL;
        responseMessage = "Entering Extended Passive Mode (|||"s + to_string(port) + "|)."s;
        break;
    case ENUM_CLASS_VALUE(Request::MODE_PASSIVE_SHORT):
        responseCode = Response::SUCCESS_PASSIVE_SHORT;
        algorithms::replace_allC(myIp, '.', ',');
        responseMessage = "Entering Passive Mode ("s + myIp + ","s + to_string(port / 256) + ","s + to_string(port % 256) + ")."s;
        break;
    case ENUM_CLASS_VALUE(Request::MODE_PASSIVE_LONG):
        responseCode = Response::SUCCESS_PASSIVE_LONG;
        responseMessage = "Entering Long Passive Mode ("s + myIp + ", "s + to_string(port) + ")."s;
        break;
    default: // Code never comes here
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_ARGUMENT_NOTSUPPORTED)) + " Unsupported passive mode."s);
        return;
    }

    // Inform client of new data server
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(responseCode)) + " "s + responseMessage);
    return;
}

void FtpServer::on_msg_listDirectory(const int clientId, const uint32_t command, const valarray<string> &args)
{
    // Get user, current directory and data server from session
    string username;
    string path;
    unique_ptr<TcpServer> dataServer;
    int dataClientId;
    mutex *p_processed_m;
    unique_ptr<unique_lock<shared_mutex>> lck_session_modify_unique;
    {
        shared_lock<shared_mutex> lck_session{session_m}; // Read: Allow simultaneous actions on session map
        unique_ptr<Session> &session{activeSessions.at(clientId)};
        shared_lock<shared_mutex> lck_session_modify{session->modify_m}; // Read: Allow simultaneous actions on session data
        session->established_m.lock();
        username = session->username;
        path = session->currentpath;
        dataClientId = session->dataClientId;
        p_processed_m = &session->processed_m;
        lck_session_modify.unlock();
        lck_session_modify_unique = make_unique<unique_lock<shared_mutex>>(session->modify_m); // Modify: Block simultaneous actions on session data
        dataServer = move(session->tcpData);                                                   // Remove data server from session as should be closed after this action
        session->dataClientId = -1;                                                            // Reset data client ID in session
    }

    // Check data server exists and is running
    if (!(dataServer && dataServer->isRunning())) // INFO: If left evaluated false, right will not be evaluated at all
    {
        p_processed_m->unlock();
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_WRONG_ORDER)) + " Data connection must be opened first via PASV"s);
        return;
    }

    // Get directory list into string
    ostringstream msg;
    valarray<Item> items = work_listDirectory(username);
    size_t numItems{items.size()};
    for (size_t i{0}; i < numItems; i += 1)
    {
        msg << items[i] << endl;
    }

    // Send directory list to client
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::SUCCESS_DATA_OPEN)) + " Here comes the directory listing."s);
    dataServer->sendMsg(dataClientId, msg.str());
    p_processed_m->unlock();
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::SUCCESS_DATA_CLOSE)) + " Directory send OK."s);
    return; // Close data connection by deleting the data server. Disconnect to be done by transfer master (server in this case)
}

void FtpServer::on_msg_fileDownload(const int clientId, const uint32_t command, const valarray<string> &args)
{
    // Get user, current directory and data server from session
    string username;
    string path;
    underlying_type_t<FileTransferType> transferType;
    unique_ptr<TcpServer> dataServer;
    int dataClientId;
    mutex *p_processed_m;
    unique_ptr<shared_lock<shared_mutex>> lck_session_modify;
    {
        shared_lock<shared_mutex> lck_session{session_m}; // Read: Allow simultaneous actions on session map
        unique_ptr<Session> &session{activeSessions.at(clientId)};
        lck_session_modify = make_unique<shared_lock<shared_mutex>>(session->modify_m); // Read: Allow simultaneous actions on session data
        session->established_m.lock();
        username = session->username;
        path = session->currentpath;
        transferType = session->transferType; // No check needed as already done in on_msg_modePassive
        dataClientId = session->dataClientId;
        p_processed_m = &session->processed_m;
        lck_session_modify->unlock();
        unique_lock<shared_mutex> lck_session_modify_unique{session->modify_m}; // Modify: Block simultaneous actions on session data
        dataServer = move(session->tcpData);                                    // Remove data server from session as should be closed after this action
        session->dataClientId = -1;                                             // Reset data client ID in session
        lck_session_modify_unique.unlock();
        lck_session_modify->lock();
    }

    // Check data server exists and is running
    if (!(dataServer && dataServer->isRunning())) // INFO: If left evaluated false, right will not be evaluated at all
    {
        p_processed_m->unlock();
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_WRONG_ORDER)) + " Data connection must be opened first via PASV"s);
        return;
    }

    // Get stream to file that should be downloaded
    unique_ptr<istream> is{work_readFile(path + "/"s + args[0], getStreamOpenMode(STREAM_DIRECTION_READ, transferType))};

    // Send file content to client
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::SUCCESS_DATA_OPEN)) + " Here comes the content of file "s + args[0] + "."s);
    string chunk{string(FILETRANSFER_CHUNKSIZE, 0)};
    while (!is->eof())
    {
        is->read(chunk.data(), FILETRANSFER_CHUNKSIZE);
        dataServer->sendMsg(dataClientId, chunk.substr(0, is->gcount()));
    }
    p_processed_m->unlock();
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::SUCCESS_DATA_CLOSE)) + " File send OK."s);
    return; // Close data connection by deleting the data server. Disconnect to be done by transfer master (server in this case)
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
    const string &path_req{args[0]};
    bool accessible;
    bool success;
    {
        string path_new;
        shared_lock<shared_mutex> lck_session{session_m}; // Read: Allow simultaneous actions on session map
        unique_ptr<Session> &session{activeSessions.at(clientId)};
        shared_lock<shared_mutex> lck_session_modify{session->modify_m}; // Read: Allow simultaneous actions on session data
        const string &username{session->username};
        const string &path{session->currentpath};

        // Determine requested absolute path
        if (path_req.empty() || path_req[0] != '/') // Relative path
            path_new = path + "/"s + path_req;
        else // Absolute path
            path_new = path_req;

        accessible = work_checkAccessible(username, path_new);
        if (accessible)
            success = work_createDirectory(path_new);
        // If not accessible, value of success is irrelevant
    }

    string response;
    if (!accessible)
        response = to_string(ENUM_CLASS_VALUE(Response::FAILED_FILENOTACCESSIBLE)) + " Requested directory is not accessible."s;
    else if (!success)
        response = to_string(ENUM_CLASS_VALUE(Response::FAILED_FILENOTACCESSIBLE)) + " Failed to create directory."s;
    else
        response = to_string(ENUM_CLASS_VALUE(Response::SUCCESS_DIRECTORY)) + " \""s + path_req + "\" created."s;

    tcpControl.sendMsg(clientId, response);
    return;
}

void FtpServer::on_msg_fileUpload(const int clientId, const uint32_t command, const valarray<string> &args)
{
    // Get user, current directory and data server from session
    string username;
    string path;
    underlying_type_t<FileTransferType> transferType;
    unique_ptr<TcpServer> dataServer;
    DynamicOstream<STREAM_DYNAMICOSTREAM_BUFFERSIZE> *incomingStreamFwd;
    mutex *p_processed_m;
    mutex *p_closed_m;
    unique_ptr<shared_lock<shared_mutex>> lck_session_modify;
    {
        shared_lock<shared_mutex> lck_session{session_m}; // Read: Allow simultaneous actions on session map
        unique_ptr<Session> &session{activeSessions.at(clientId)};
        lck_session_modify = make_unique<shared_lock<shared_mutex>>(session->modify_m); // Read: Allow simultaneous actions on session data
        session->established_m.lock();
        username = session->username;
        path = session->currentpath;
        transferType = session->transferType; // No check needed as already done in on_msg_modePassive
        p_processed_m = &session->processed_m;
        p_closed_m = &session->closed_m;
        lck_session_modify->unlock();
        unique_lock<shared_mutex> lck_session_modify_unique{session->modify_m}; // Modify: Block simultaneous actions on session data
        dataServer = move(session->tcpData);                                    // Remove data server from session as should be closed after this action
        incomingStreamFwd = session->incomingStreamFwd;                         // Remove stream from session as should be closed after this action
        lck_session_modify_unique.unlock();
        lck_session_modify->lock();
    }

    // Check data server exists and is running
    if (!(dataServer && dataServer->isRunning())) // INFO: If left evaluated false, right will not be evaluated at all
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_WRONG_ORDER)) + " Data connection must be opened first via PASV"s);
        p_processed_m->unlock(); // Clean up
        return;
    }

    // Get stream to file that should be uploaded and redirect data server output to file stream
    unique_ptr<ostream> outgoingStream{work_writeFile(path + "/"s + args[0], getStreamOpenMode(STREAM_DIRECTION_WRITE, transferType))};
    incomingStreamFwd->redirect(outgoingStream.get());
    p_processed_m->unlock(); // Allow data processing to start

    // Data server is now ready to accept data
    // Data will be written to temporary file and moved to final destination after upload is complete
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::SUCCESS_DATA_OPEN)) + " Ready to receive data."s);
    p_closed_m->lock(); // Wait here until data server has closed connection and all data is received. Disconnect to be done by transfer master (client in this case)

    // Client has disconnected from data server when reaching this point
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::SUCCESS_DATA_CLOSE)) + " File upload OK."s);
    return;
}
