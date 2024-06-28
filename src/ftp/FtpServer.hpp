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

// Define getting enum class value as underlying type
#define ENUM_CLASS_VALUE(x) static_cast<::std::underlying_type_t<decltype(x)>>(x)

namespace ftp
{
    // Item type
    enum class ItemType
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
         * @brief Return if the FTP server is running (means if underlying TCP server is running)
         *
         * @return bool (true if running, false if not)
         */
        bool isRunning() const;

        /**
         * @brief Get unique ID for a command string
         *        This makes it easier to jump in code based on the command
         *        Each command is made of 3-4 bytes, so the ID is just the numeric representation
         *
         * @param command
         * @return uint32
         */
        static constexpr uint32_t hashCommand(const char *const command)
        {
            int len{::std::strlen(command)};

            uint32_t id{0};
            for (int i = 0; i < len; i += 1)
            {
                id |= static_cast<uint32_t>(command[i]) << (24 - (i * 8));
            }
            return id;
        }

    private:
        /**
         * @brief Worker methods for incoming messages
         */
        void on_newClient(const int clientId);
        void on_msg(const int clientId, const ::std::string &msg);
        void on_closed(const int clientId);

        /**
         * @brief Cut out command from incoming message
         *        <command> <parameters>
         *
         * @param msg
         * @return ::std::string
         */
        ::std::string getCommand(const ::std::string &msg) const;

        // Constants
        const size_t MAXIMUM_MESSAGE_LENGTH{4096};
        const int PORT_CONTROL{21};

        // Underlying TCP server
        ::tcp::TcpServer tcpControl; // Fragmented

        // Pointer to functions on incoming message
        ::std::function<bool(const ::std::string, const ::std::string)> work_checkUserCredentials; // Check user credentials: Name, password
        ::std::function<bool(const ::std::string)> work_checkAccessible;                           // Check if path is accessible (directory or file)
        ::std::function<::std::valarray<Item>(const ::std::string)> work_listDirectory;            // List directory content
        ::std::function<::std::ifstream(const ::std::string)> work_readFile;                       // Read file content
    };

    // Hashed request keywords
    enum class Request : uint32_t
    {
        USERNAME = FtpServer::hashCommand("USER")
    };

    // Response codes
    enum class Response : int
    {
        WELCOME = 220,
        PASSWORD_REQUIRED = 331
    };
}

#endif // FTPSERVER_HPP_
