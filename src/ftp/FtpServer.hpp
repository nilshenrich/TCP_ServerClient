/**
 * @file FtpServer.hpp
 * @author Nils Henrich
 * @brief FTP server for unencrypted and encrypted data transfer based on TCP/TLS server.
 * @version 3.0.0
 * @date 2024-06-26
 *
 * @copyright Copyright (c) 2024
 */

#ifndef FTPSERVER_HPP_
#define FTPSERVER_HPP_

#include "../basic/TcpServer.hpp"
#include "../basic/TlsServer.hpp"

namespace ftp
{
    class FtpServer
    {
    public:
    private:
        // Underlying TCP and TLS servers
        ::tcp::TcpServer tcpServer;
        ::tcp::TlsServer tlsServer;
    };
}

#endif // FTPSERVER_HPP_
