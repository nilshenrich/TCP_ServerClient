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
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::WELCOME)) + " Welcome");
}
void FtpServer::on_msg(const int clientId, const string &msg)
{
    Reqp request{parseRequest(msg)};
    switch (request.command)
    {
    case ENUM_CLASS_VALUE(Request::USERNAME):
        on_msg_USER(clientId, request.command, request.args);
        break;
    default:
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_NOTIMPLEMENTED)) + " Command not implemented");
        break;
    }
}
void FtpServer::on_closed(const int clientId)
{
    // Logout session
    users_loggedIn.erase(clientId);
    users_requested.erase(clientId);

    return;
}

//////////////////////////////////////////////////
// Worker mehods on incoming messages
//////////////////////////////////////////////////

void FtpServer::on_msg_USER(const int clientId, const uint32_t command, const valarray<string> &args)
{
    // Check num of arguments
    size_t numArgs{args.size()};
    if (numArgs != 1)
    {
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_SYNTAX_ARGUMENT)) + " 1 argument expected, but " + to_string(numArgs) + " given");
        return;
    }

    // TODO: Check user is not logged in already

    // Request fine, require password
    tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::PASSWORD_REQUIRED)) + " Password required for user " + args[0]);
    return;
}
