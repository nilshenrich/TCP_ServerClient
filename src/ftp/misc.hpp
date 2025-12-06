/**
 * @file misc.hpp
 * @author Nils Henrich
 * @brief Miscellaneous classes, structures, collections and functions used for FTP server
 * @version 3.0.0
 * @date 2025-04-19
 *
 * @copyright Copyright (c) 2025
 */

#ifndef MISC_HPP_
#define MISC_HPP_

#include <array>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <memory>
#include <ostream>
#include <string>
#include <valarray>

#include "../basic/TcpServer.hpp"
#include "../basic/TlsServer.hpp"

//////////////////////////////////////////////////
// General definitions
//////////////////////////////////////////////////

// Define getting enum class value as underlying type
#define ENUM_CLASS_VALUE(x) static_cast<::std::underlying_type_t<decltype(x)>>(x)

// Define stream open modes for directions and file transfer types
#define STREAM_OPEN_MODE_READ_ASCII std::ios::in
#define STREAM_OPEN_MODE_READ_UNICODE std::ios::in
#define STREAM_OPEN_MODE_READ_BINARY std::ios::in | std::ios::binary
#define STREAM_OPEN_MODE_WRITE_ASCII std::ios::out
#define STREAM_OPEN_MODE_WRITE_UNICODE std::ios::out
#define STREAM_OPEN_MODE_WRITE_BINARY std::ios::out | std::ios::binary
#define STREAM_DIRECTION_READ true
#define STREAM_DIRECTION_WRITE false

// Dynamic stream buffer size
#define STREAM_DYNAMICOSTREAM_BUFFERSIZE 65536

namespace ftp
{
    //////////////////////////////////////////////////
    // Utility functions
    //////////////////////////////////////////////////

    /**
     * @brief Get unique ID for a command string
     *        This makes it easier to jump in code based on the command
     *        Each command is made of 3-4 bytes, so the ID is just the numeric representation
     *
     * @param command
     * @return uint32
     */
    // TODO: Increase performance by parallelizing byte calculations
    constexpr uint32_t hashCommand(const char *const command)
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

    //////////////////////////////////////////////////
    // Classes
    //////////////////////////////////////////////////

    template <::std::size_t BUFFER_SIZE>
    class DynamicStreambuf : public ::std::streambuf
    {
    public:
        // Default constructor. Stream buffer not set on object creation (null-stream), to be set later via setStreambuf()
        DynamicStreambuf() : p_streambuf{nullptr},
                             buffer{},
                             bufferStatus{1}
        {
            setp(begin(buffer), end(buffer) - 1);
        }

        // Destructor
        virtual ~DynamicStreambuf() {}

        // Redirect the stream buffer to the given stream buffer
        void setStreambuf(::std::streambuf *buf)
        {
            if (bufferStatus == -1)
                throw ::tcp::Server_error("DynamicStreambuf::setStreambuf() - Cannot set stream buffer as buffer is full and in error state.");

            if (p_streambuf)
                throw ::tcp::Server_error("DynamicStreambuf::setStreambuf() - Stream buffer already set.");

            if (!buf)
                throw ::tcp::Server_error("DynamicStreambuf::setStreambuf() - Cannot set null stream buffer.");

            p_streambuf = buf;
            bufferStatus = 0; // Stream buffer successfully set
            sync();           // Send any buffered data to the stream
        }

        // Get the current stream buffer status
        //  0: Stream buffer successfully set -> data buffered and sent
        //  1: Stream buffer not full and not set -> data buffered but not sent
        // -1: Stream buffer full but not set -> data not sent
        int status() const { return bufferStatus; }

    private:
        // Pointer to the stream buffer. This can be changed while usage. This makes this stream buffer dynamic.
        ::std::streambuf *p_streambuf;

        // Buffered data not yet sent to the stream
        ::std::array<char, BUFFER_SIZE> buffer;

        // Status
        //  0: Stream buffer successfully set -> data buffered and sent
        //  1: Stream buffer not full and not set -> data buffered but not sent
        // -1: Stream buffer full but not set -> data not sent
        int bufferStatus;

        // Send buffered data to the stream
        // Output the buffered data to the stream
        // Returns 0 on success, -1 on error
        int sync() override
        {
            if (!p_streambuf)
                return 0;

            p_streambuf->sputn(pbase(), pptr() - pbase());
            setp(begin(buffer), end(buffer) - 1);
            return 0; // Success
        }

        // Buffer full, send data to the stream if existing. If not, throw an error
        int_type overflow(int_type c) override
        {
            // If no stream buffer is set, clear the buffer and return failure code
            if (!p_streambuf)
            {
#ifdef DEVELOP
                ::std::cerr << "DynamicStreambuf::overflow() - No stream buffer set, cannot send data." << ::std::endl;
#endif // DEVELOP

                setp(begin(buffer), end(buffer) - 1);
                bufferStatus = -1; // Buffer full but not set
                return traits_type::eof();
            }

            if (c != traits_type::eof())
            {
                *pptr() = traits_type::to_char_type(c);
                pbump(1);
                sync();
            }
            return c;
        }
    };
    template <::std::size_t BUFFER_SIZE>
    class DynamicOstream : public ::std::ostream
    {
    public:
        // Default constructor
        DynamicOstream() : ::std::ostream{}, streambuf{} { init(&streambuf); }

        // Destructor
        virtual ~DynamicOstream() {}

        // Redirect the stream buffer to the given stream buffer or stream
        void redirect(::std::streambuf *buf)
        {
            streambuf.setStreambuf(buf);
        }
        void redirect(::std::ostream *os)
        {
            streambuf.setStreambuf(os->rdbuf());
        }

        // Get the stream buffer
        DynamicStreambuf<BUFFER_SIZE> *rdbuf() const
        {
            return const_cast<DynamicStreambuf<BUFFER_SIZE> *>(&streambuf);
        }

    private:
        DynamicStreambuf<BUFFER_SIZE> streambuf;
    };

    //////////////////////////////////////////////////
    // Types for file transfer
    //////////////////////////////////////////////////

    // Item type
    enum class ItemType
    {
        directory,
        file,
        link
    };

    // File transfer types (EBCDIC not supported)
    enum class FileTransferType : char
    {
        ASCII = 'A',
        BINARY = 'I',
        UNICODE = 'U',
        INVALID = 0,
    };

    //////////////////////////////////////////////////
    // Small utility structures used for type bundling across the FTP server implementation
    //////////////////////////////////////////////////

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
        bool loggedIn;                                                       // Is user logged in?
        ::std::string username;                                              // Username
        ::std::string currentpath;                                           // Always absolute from user home
        char transferType;                                                   // FileTransferType
        ::std::unique_ptr<::tcp::TcpServer> tcpData;                         // Data server for file transfer
        int dataClientId;                                                    // Client ID for data connection
        DynamicOstream<STREAM_DYNAMICOSTREAM_BUFFERSIZE> *incomingStreamFwd; // Forward incoming data to this stream (file upload) - Memory managed outside of session by Server
        ::std::mutex established_m;                                          // Mutex to wait for data connection to be established
        ::std::mutex processed_m;                                            // Mutex to wait for data transfer to be processed
        ::std::mutex closed_m;                                               // Mutex to wait for data connection to be closed
        ::std::shared_mutex modify_m;                                        // Mutex to protect session data modification

        // Constructors

        // Default: Not logged in
        Session() : Session{false, ::std::string{}, ::std::string{}} {}

        // Given logged in, username and current path
        Session(bool loggedIn, const ::std::string &username, const ::std::string &currentpath) : loggedIn{loggedIn},
                                                                                                  username{username},
                                                                                                  currentpath{currentpath},
                                                                                                  transferType{0},
                                                                                                  tcpData{nullptr},
                                                                                                  dataClientId{-1},
                                                                                                  incomingStreamFwd{nullptr},
                                                                                                  established_m{},
                                                                                                  processed_m{},
                                                                                                  closed_m{},
                                                                                                  modify_m{} {}

        // Overload operator<<
        friend ::std::ostream &operator<<(::std::ostream &os, const Session &s)
        {
            os << "{loggedIn: " << s.loggedIn
               << ", username: " << s.username
               << ", currentpath: " << s.currentpath
               << ", transferType: " << s.transferType
               << ", has tcpData: " << (s.tcpData ? "yes" : "no")
               << ", forward stream set: " << (s.incomingStreamFwd && s.incomingStreamFwd->rdbuf() ? "yes" : "no") << "}";
            return os;
        }

        // Delete copy constructor, move constructor and assignment operator, as unique_ptr and mutex are not copyable
        Session(const Session &) = delete;
        Session(Session &&) = delete;
        Session &operator=(const Session &) = delete;
    };

    //////////////////////////////////////////////////
    // FTP command and response codes
    //////////////////////////////////////////////////

    // Hashed request keywords
    // https://en.wikipedia.org/wiki/List_of_FTP_commands
    enum class Request : uint32_t
    {
        SYSTEMTYPE = hashCommand("SYST"),          // System type of server (e.g. UNIX Type: L8)
        USERNAME = hashCommand("USER"),            // Username for login
        PASSWORD = hashCommand("PASS"),            // Password for login
        DIRECTORY_GETCURRENT = hashCommand("PWD"), // Get current directory path
        FEATURES_LIST = hashCommand("FEAT"),       // List of features supported by server
        DIRECTORY_LIST = hashCommand("LIST"),      // List directory content
        DIRECTORY_CHANGE = hashCommand("CWD"),     // Change directory
        DIRECTORY_CREATE = hashCommand("MKD"),     // Create directory
        FILE_TRANSFER_TYPE = hashCommand("TYPE"),  // Set file transfer type
        MODE_PASSIVE_ALL = hashCommand("EPSV"),    // Enter passive mode (For both IPv4 and IPv6)
        MODE_PASSIVE_SHORT = hashCommand("PASV"),  // Enter passive mode (For IPv4 only)
        MODE_PASSIVE_LONG = hashCommand("LPSV"),   // Enter passive mode (For IPv6 only)
        FILE_DOWNLOAD = hashCommand("RETR"),       // Download file
        FILE_UPLOAD = hashCommand("STOR"),         // Upload file
    };

    // Response codes
    // https://en.wikipedia.org/wiki/List_of_FTP_server_return_codes
    enum class Response : int
    {
        SUCCESS_DATA_OPEN = 150,
        OK = 200,
        SUCCESS_STATUS = 211,
        SUCCESS_SYSTEMTYPE = 215,
        SUCCESS_WELCOME = 220,
        SUCCESS_DATA_CLOSE = 226,
        SUCCESS_PASSIVE_ALL = 229,
        SUCCESS_PASSIVE_SHORT = 227,
        SUCCESS_PASSIVE_LONG = 228,
        SUCCESS_LOGIN = 230,
        SUCCESS_ACTION = 250,
        SUCCESS_DIRECTORY = 257,
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
}
#endif // MISC_HPP_
