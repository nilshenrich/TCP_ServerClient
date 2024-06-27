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

#include <functional>
#include <valarray>
#include <fstream>

#include "../basic/TcpServer.hpp"
#include "../basic/TlsServer.hpp"

namespace ftp
{
    // Item type
    enum ItemType
    {
        directory,
        file,
        link
    };

    // Item properties
    struct Item
    {
        ItemType type;
        ::std::string name;
        char permissions[3]; // "rwx"*[user, group, other]
        int nLinks;          // number of links
        int uid;             // user id
        int gid;             // group id
        int size;            // [file] size in bytes | [directory] number of items
        int mtime;           // modification time in UNIX seconds
    };

    class FtpServer
    {
    public:
        /**
         * @brief Basic constructor and destructor
         */
        FtpServer(); // TODO: Add constructor taking path to config file
        virtual ~FtpServer();

        /**
         * @brief Settings for the FTP server
         */

        /**
         * @brief Start the FTP server
         *
         * @return int
         */
        int start();

        /**
         * @brief Stop the FTP server
         */
        void stop();

        /**
         * @brief Link worker methods
         */
        void setWork_checkUserCredentials(::std::function<bool(const ::std::string, const ::std::string)> work);
        void setWork_checkAccessible(::std::function<bool(const ::std::string)> work);
        void setWork_listDirectory(::std::function<::std::valarray<Item>(const ::std::string)> work);
        void setWork_readFile(::std::function<::std::ifstream(const ::std::string)> work);

        /**
         * @brief Return if the FTP server is running
         *
         * @return bool (true if running, false if not)
         */
        bool isRunning() const;

    private:
        // Underlying TCP server
        ::tcp::TcpServer tcpControl; // Fragmented

        // Pointer to functions on incoming message
        ::std::function<bool(const ::std::string, const ::std::string)> work_checkUserCredentials; // Check user credentials: Name, password
        ::std::function<bool(const ::std::string)> work_checkAccessible;                           // Check if path is accessible (directory or file)
        ::std::function<::std::valarray<Item>(const ::std::string)> work_listDirectory;            // List directory content
        ::std::function<::std::ifstream(const ::std::string)> work_readFile;                       // Read file content
    };
}

#endif // FTPSERVER_HPP_
