/**
 * @file FtpServer.hpp
 * @author Nils Henrich
 * @brief FTP server for unencrypted and encrypted data transfer based on TCP/TLS server.
 * @details https://datatracker.ietf.org/doc/html/rfc959
 * @version 3.0.0
 * @date 2024-06-26
 *
 * @copyright Copyright (c) 2024
 */

#ifndef FTPSERVER_HPP_
#define FTPSERVER_HPP_

#include <functional>
#include <istream>
#include <map>
#include <mutex>
#include <ostream>
#include <shared_mutex>
#include <string>
#include <type_traits>
#include <valarray>

#include "../basic/TcpServer.hpp"
#include "../basic/TlsServer.hpp"
#include "misc.hpp"

namespace ftp
{
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
        void setWork_checkUserCredentials(::std::function<bool(const ::std::string &, const ::std::string &)> work);                              // See description of work_checkUserCredentials
        void setWork_checkAccessible(::std::function<bool(const ::std::string &, const ::std::string &)> work);                                   // See description of work_checkAccessible
        void setWork_listDirectory(::std::function<::std::valarray<Item>(const ::std::string &)> work);                                           // See description of work_listDirectory
        void setWork_createDirectory(::std::function<bool(const ::std::string &, const ::std::string &)> work);                                   // See description of work_createDirectory
        void setWork_readFile(::std::function<::std::istream *(const ::std::string &, const ::std::string &, const ::std::ios::openmode)> work);  // See description of work_readFile
        void setWork_writeFile(::std::function<::std::ostream *(const ::std::string &, const ::std::string &, const ::std::ios::openmode)> work); // See description of work_writeFile

        /**
         * @brief Return if the FTP server is running (means if underlying TCP server is running)
         *
         * @return bool (true if running, false if not)
         */
        bool isRunning() const;

    private:
        /**
         * @brief Worker methods for incoming messages
         */
        void on_newClient(const int clientId);
        void on_msg(const int clientId, const ::std::string &msg);
        void on_closed(const int clientId);

        /**
         * @brief Cut out command and arguments from incoming message
         *        <command> <arguments>, ...
         *
         * @param msg
         * @return Reqp
         */
        Reqp parseRequest(const ::std::string &msg) const;

        /**
         * @brief Remove illegal characters from a request string
         *        - \n
         *        - \r
         *        - \t
         *
         * @param request
         * @return string
         */
        ::std::string sanitizeRequest(const ::std::string &request) const;

        /**
         * @brief Determine the stream open mode based on direction and file transfer type
         *
         * @param direction Stream direction (read/write)
         * @param transferType FTP file transfer type
         * @return ::std::ios::openmode
         */
        ::std::ios::openmode getStreamOpenMode(const bool direction, const ::std::underlying_type_t<FileTransferType> transferType) const;

        // Constants
        const size_t MAXIMUM_MESSAGE_LENGTH{4096};
        const int PORT_CONTROL{21};
        const int PORT_RANGE_DATA[2]{1024, 65535};

        // Underlying TCP server. Control and data
        ::tcp::TcpServer tcpControl; // Fragmented

        // Active user sessions. Key is client ID, value is username
        ::std::map<int, ::std::unique_ptr<Session>> activeSessions{}; // Open sessions // INFO: Needs to be pointer as Session contains unique_ptr and mutex which are not copyable or movable

        // Thread safety
        // Rules for session map access:
        // - Read:
        //     - [allowed] simultaneous reads
        //     - [allowed] simultaneous modifications (Each item is managed by its own mutex)
        //     - [allowed] simultaneous creates (Blocked here due to ease of implementation)
        //     - [blocked] simultaneous deletes (Automatically done by unique lock on delete)
        //     -> Allows all parallel actions
        // - Modify:
        //     - [allowed] simultaneous reads (Each item is managed by its own mutex)
        //     - [allowed] simultaneous modifications (Each item is managed by its own mutex)
        //     - [allowed] simultaneous creates (Blocked here due to ease of implementation)
        //     - [blocked] simultaneous deletes (Automatically done by unique lock on delete)
        //     -> Allows all parallel actions
        // - Create:
        //     - [allowed] simultaneous reads (Blocked here due to ease of implementation)
        //     - [allowed] simultaneous modifications (Blocked here due to ease of implementation)
        //     - [blocked] simultaneous creates
        //     - [blocked] simultaneous deletes
        //     -> Blocks all other actions
        // - Delete:
        //     - [blocked] simultaneous reads
        //     - [blocked] simultaneous modifications
        //     - [blocked] simultaneous creates
        //     - [blocked] simultaneous deletes
        //     -> Blocks all other actions
        ::std::shared_mutex session_m{}; // Mutex for blocking actions on session map: read, modify, create, delete
        ::std::mutex tcpPort_m{};        // Mutex for TCP port availability

        // Pointer to functions on incoming message
        ::std::function<bool(const ::std::string &, const ::std::string &)> work_checkUserCredentials;                              // Check user credentials: name, password -> bool
        ::std::function<bool(const ::std::string &, const ::std::string &)> work_checkAccessible;                                   // Check if path is accessible (directory or file) for user: username, path (Absolute from user home) -> bool
        ::std::function<::std::valarray<Item>(const ::std::string &)> work_listDirectory;                                           // List content of current directory: username -> items
        ::std::function<bool(const ::std::string &, const ::std::string &)> work_createDirectory;                                   // Create directory: username, path (Absolute from user home) -> bool
        ::std::function<::std::istream *(const ::std::string &, const ::std::string &, const ::std::ios::openmode)> work_readFile;  // Read file content: username, path (Absolute from user home), open mode -> reading stream
        ::std::function<::std::ostream *(const ::std::string &, const ::std::string &, const ::std::ios::openmode)> work_writeFile; // Write content to file: username, path (Absolute from user home), open mode -> writing stream

        //////////////////////////////////////////////////
        // Worker methods on incoming messages
        //////////////////////////////////////////////////

        /**
         * @brief General template for all worker methods
         *        1. Perform checks:
         *          - Check if session exists
         *          - Check if user is logged in (default)
         *          - Check num of arguments
         *        2. Process request
         *
         * @param clientId      TCP session ID
         * @param command       FTP request command
         * @param args          FTP request arguments
         * @param numArgsExp Number of arguments
         * @param work          Worker method
         * @param mustLoggedIn  Must user be logged in? Default: true
         */
        void on_messageIn(const int clientId, const uint32_t command,
                          void (FtpServer::*work)(const int, const uint32_t, const ::std::string &),
                          const ::std::string &arg = ::std::string{},
                          const bool hasArg = false,
                          const bool mustLoggedIn = true);

        // Worker methods
        void on_msg_username(const int clientId, const uint32_t command, const ::std::string &arg);
        void on_msg_password(const int clientId, const uint32_t command, const ::std::string &arg);
        void on_msg_getSystemType(const int clientId, const uint32_t command, const ::std::string &arg);
        void on_msg_listFeatures(const int clientId, const uint32_t command, const ::std::string &arg);
        void on_msg_listDirectory(const int clientId, const uint32_t command, const ::std::string &arg);
        void on_msg_changeDirectory(const int clientId, const uint32_t command, const ::std::string &arg);
        void on_msg_getDirectory(const int clientId, const uint32_t command, const ::std::string &arg);
        void on_msg_createDirectory(const int clientId, const uint32_t command, const ::std::string &arg);
        void on_msg_fileTransferType(const int clientId, const uint32_t command, const ::std::string &arg);
        void on_msg_modePassive(const int clientId, const uint32_t command, const ::std::string &arg);
        void on_msg_fileDownload(const int clientId, const uint32_t command, const ::std::string &arg);
        void on_msg_fileUpload(const int clientId, const uint32_t command, const ::std::string &arg);

        //////////////////////////////////////////////////
        // Constants
        //////////////////////////////////////////////////

        // Features the server supports
        // To be responded to client as feature list
        const ::std::valarray<::std::string> features{"PASV"};

        // Chunk size for file transfer in bytes
        const size_t FILETRANSFER_CHUNKSIZE{65536};
    };
}

#endif // FTPSERVER_HPP_
