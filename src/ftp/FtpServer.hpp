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

#include <string>
#include <valarray>
#include <map>
#include <iomanip>

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

        // Overload operator<<
        friend ::std::ostream &operator<<(::std::ostream &os, const Item &i)
        {
            // Item type
            switch (i.type)
            {
            case ItemType::directory:
                os << "d";
                break;
            case ItemType::link:
                os << "l";
                break;
            case ItemType::file:
            default:
                os << "-";
                break;
            }

            // Item permissions (user, group, other)
            for (int pi{0}; pi < 3; pi += 1)
            {
                const char &p{i.permissions[pi]};
                os << (p & 4 ? "r" : "-"); // read
                os << (p & 2 ? "w" : "-"); // write
                os << (p & 1 ? "x" : "-"); // execute
            }

            // Number of links, owner, group, size
            os.fill(0x20);
            os << ' ' << ::std::setw(4) << i.nLinks;
            os << ' ' << ::std::setw(4) << i.uid;
            os << ' ' << ::std::setw(4) << i.gid;
            os << ' ' << ::std::setw(12) << i.size;

            // Modification time using format: yyyy mmm dd hh:mm
            os.fill('0');
            ::std::time_t time{i.mtime};
            size_t tSize{::std::size("yyyy mmm dd hh:mm")};
            char tBuffer[tSize];
            ::std::strftime(tBuffer, tSize, "%Y %b %d %H:%M", ::std::localtime(&time));
            os << ' ' << tBuffer;

            // Item name
            os << ' ' << i.name;

            return os;
        }
    };

    // Request properties
    struct Reqp
    {
        uint32_t command;
        ::std::valarray<::std::string> args;
    };

    // Session data
    struct Session
    {
        bool loggedIn;
        ::std::string username;
        ::std::string currentpath;
        char mode; // FileTransferType
        ::std::unique_ptr<::tcp::TcpServer> tcpData;

        // Constructors

        // Default: Not logged in
        Session() : loggedIn{false},
                    username{},
                    currentpath{},
                    mode{0},
                    tcpData{nullptr} {}

        // Given logged in, username and current path
        Session(bool loggedIn, const ::std::string &username, const ::std::string &currentpath) : loggedIn{loggedIn},
                                                                                                  username{username},
                                                                                                  currentpath{currentpath},
                                                                                                  mode{0},
                                                                                                  tcpData{nullptr} {}

        // Overload operator<<
        friend ::std::ostream &operator<<(::std::ostream &os, const Session &s)
        {
            os << "{loggedIn: " << s.loggedIn << ", username: " << s.username << ", currentpath: " << s.currentpath << ", mode: " << s.mode << ", has tcpData: " << (s.tcpData ? "yes" : "no") << "}";
            return os;
        }
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
        void setWork_checkAccessible(::std::function<bool(const ::std::string, const ::std::string)> work);
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
            size_t len{::std::min<size_t>(::std::strlen(command), 4)};

            uint32_t id{0};
            for (size_t i = 0; i < len; i += 1)
            {
                char c{command[i]};
                id |= static_cast<uint32_t>(c * (c >= 0x20)) << (24 - (i * 8));
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
         * @brief Get free random TCP port for data within range
         *        Return -1 if no free port found
         *
         * @return int
         */
        int getFreePort() const;

        // Constants
        const size_t MAXIMUM_MESSAGE_LENGTH{4096};
        const int PORT_CONTROL{21};
        const int PORT_RANGE_DATA[2]{1024, 65535};

        // Underlying TCP server. Control and data
        ::tcp::TcpServer tcpControl; // Fragmented

        // Active user sessions. Key is client ID, value is username
        ::std::map<int, Session> session{}; // Open sessions

        // Thread safety
        ::std::mutex session_m{}; // Mutex for session
        ::std::mutex tcpPort_m{}; // Mutex for TCP port availability

        // Pointer to functions on incoming message
        ::std::function<bool(const ::std::string, const ::std::string)> work_checkUserCredentials; // Check user credentials: name, password
        ::std::function<bool(const ::std::string, const ::std::string)> work_checkAccessible;      // Check if path is accessible (directory or file) for user: username, path
        ::std::function<::std::valarray<Item>(const ::std::string)> work_listDirectory;            // List directory content: path
        ::std::function<::std::ifstream(const ::std::string)> work_readFile;                       // Read file content: path

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
        void on_messageIn(const int clientId, const uint32_t command, const ::std::valarray<::std::string> &args, size_t numArgsExp,
                          void (FtpServer::*work)(const int, const uint32_t, const ::std::valarray<::std::string> &),
                          const bool mustLoggedIn = true);

        // Worker methods
        void on_msg_username(const int clientId, const uint32_t command, const ::std::valarray<::std::string> &args);
        void on_msg_password(const int clientId, const uint32_t command, const ::std::valarray<::std::string> &args);
        void on_msg_getSystemType(const int clientId, const uint32_t command, const ::std::valarray<::std::string> &args);
        void on_msg_listDirectory(const int clientId, const uint32_t command, const ::std::valarray<::std::string> &args);
        void on_msg_changeDirectory(const int clientId, const uint32_t command, const ::std::valarray<::std::string> &args);
        void on_msg_getDirectory(const int clientId, const uint32_t command, const ::std::valarray<::std::string> &args);
        void on_msg_fileTransferType(const int clientId, const uint32_t command, const ::std::valarray<::std::string> &args);
        void on_msg_modePassive(const int clientId, const uint32_t command, const ::std::valarray<::std::string> &args);
        void on_msg_fileDownload(const int clientId, const uint32_t command, const ::std::valarray<::std::string> &args);
    };

    // Hashed request keywords
    enum class Request : uint32_t
    {
        SYSTEMTYPE = FtpServer::hashCommand("SYST"),
        USERNAME = FtpServer::hashCommand("USER"),
        PASSWORD = FtpServer::hashCommand("PASS"),
        LIST_DIR = FtpServer::hashCommand("LIST"),
        CHANGE_DIR = FtpServer::hashCommand("CWD"),
        GET_DIR = FtpServer::hashCommand("PWD"),
        FILE_TRANSFER_TYPE = FtpServer::hashCommand("TYPE"),
        MODE_PASSIVE_ALL = FtpServer::hashCommand("EPSV"),   // For both IPv4 and IPv6
        MODE_PASSIVE_SHORT = FtpServer::hashCommand("PASV"), // Only for IPv4
        MODE_PASSIVE_LONG = FtpServer::hashCommand("LPSV"),  // Only for IPv6
        FILE_DOWNLOAD = FtpServer::hashCommand("RETR"),
    };

    // Response codes
    enum class Response : int
    {
        SUCCESS_DATA_OPEN = 150,
        OK = 200,
        SUCCESS_SYSTEMTYPE = 215,
        SUCCESS_WELCOME = 220,
        SUCCESS_DATA_CLOSE = 226,
        SUCCESS_PASSIVE_ALL = 229,
        SUCCESS_PASSIVE_SHORT = 227,
        SUCCESS_PASSIVE_LONG = 228,
        SUCCESS_LOGIN = 230,
        SUCCESS_ACTION = 250,
        CURRENT_PATH = 257,
        CONTINUE_PASSWORD_REQUIRED = 331,
        FAILED_OPEN_DATACONN = 425,
        FAILED_LOGIN = 430,
        FAILED_FILENOTACCESSIBLE = 450,
        FAILED_UNKNOWN_ERROR = 451,
        ERROR_SYNTAX_COMMAND = 500,
        ERROR_SYNTAX_ARGUMENT = 501,
        ERROR_NOTIMPLEMENTED = 502,
        ERROR_WRONG_ORDER = 503,
        ERROR_ARGUMENT_NOTSUPPORTED = 504,
        ERROR_LOGIN = 530,
    };

    // File transfer types (EBCDIC not supported)
    enum class FileTransferType : char
    {
        ASCII = 'A',
        BINARY = 'I',
        UNICODE = 'U',
        INVALID = 0, // TODO: Check for invalid whenever used and throw error
    };
}

#endif // FTPSERVER_HPP_
