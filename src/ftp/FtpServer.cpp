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
    vector<string> words{size_t(4)}; // Max 4 words (command with 3 arguments)
    stringstream ss{msg};
    string buffer;
    while (getline(ss, buffer, ' '))
        words.push_back(buffer);

    vector<string> args{words.begin() + 1, words.end()};
    return Reqp{hashCommand(words[0].c_str()), valarray<string>{args.data(), args.size()}};
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
    {
        size_t numArgs{request.args.size()};
        if (numArgs == 1)
        {
            tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::PASSWORD_REQUIRED)) + " Password required for user " + request.args[0]);
            break;
        }
        tcpControl.sendMsg(clientId, to_string(ENUM_CLASS_VALUE(Response::ERROR_SYNTAX_ARGUMENT)) + " 1 argument expected, but " + to_string(numArgs) + " given");
        break;
    }
    default:
        break;
    }
}
void FtpServer::on_closed(const int clientId)
{
    // TODO: Implement
}
