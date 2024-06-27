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
    // TODO: Link TCP server worker methods to provide FTP server functionality
}
FtpServer::~FtpServer() { stop(); }

int FtpServer::start() { return tcpControl.start(PORT_CONTROL); }
void FtpServer::stop() { tcpControl.stop(); }

void FtpServer::setWork_checkUserCredentials(function<bool(const string, const string)> worker) { work_checkUserCredentials = worker; }
void FtpServer::setWork_checkAccessible(function<bool(const string)> worker) { work_checkAccessible = worker; }
void FtpServer::setWork_listDirectory(function<valarray<Item>(const string)> worker) { work_listDirectory = worker; }
void FtpServer::setWork_readFile(function<ifstream(const string)> worker) { work_readFile = worker; }

bool FtpServer::isRunning() const { return tcpControl.isRunning(); }
