/**
 * @file Client.hpp
 * @author Nils Henrich
 * @brief Base framework for all classes that build a network client based on TCP.
 * This class contains no functionality, but serves a base framework for the creation of stable clients based on TCP.
 * When compiling with the -DDEBUG flag, the class will print out all received messages to the console.
 * @version 3.1.0
 * @date 2021-12-28
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <thread>
#include <memory>
#include <exception>
#include <atomic>
#include <functional>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

// Debugging output
#ifdef DEVELOP
#define DEBUGINFO "(" << typeid(this).name() << " at " << this << ")::" << __func__
#endif // DEVELOP

namespace tcp
{
    // Return codes
    enum : int
    {
        CLIENT_START_OK = 0,                     // Client started successfully
        CLIENT_ERROR_START_WRONG_PORT = 10,      // Client could not start because of wrong port number
        CLIENT_ERROR_START_SET_CONTEXT = 20,     // Client could not start because of SSL context error
        CLIENT_ERROR_START_WRONG_CA_PATH = 30,   // Client could not start because of wrong path to CA cert file
        CLIENT_ERROR_START_WRONG_CERT_PATH = 31, // Client could not start because of wrong path to certifcate file
        CLIENT_ERROR_START_WRONG_KEY_PATH = 32,  // Client could not start because of wrong path to key file
        CLIENT_ERROR_START_WRONG_CA = 33,        // Client could not start because of bad CA cert file
        CLIENT_ERROR_START_WRONG_CERT = 34,      // Client could not start because of bad certificate file
        CLIENT_ERROR_START_WRONG_KEY = 35,       // Client could not start because of bad key file or non matching key with certificate
        CLIENT_ERROR_START_CREATE_SOCKET = 40,   // Client could not start because of TCP socket creation error
        CLIENT_ERROR_START_SET_SOCKET_OPT = 41,  // Client could not start because of TCP socket options error
        CLIENT_ERROR_START_CONNECT = 50,         // Client could not start because of TCP socket connection error
        CLIENT_ERROR_START_CONNECT_INIT = 60,    // Client could not start because of an error while initializing the connection
    };
    /**
     * @brief Stream that actually does nothing
     */
    class NullBuffer : public ::std::streambuf
    {
    public:
        int overflow(int c) override final
        {
            return c;
        }
    };

    /**
     * @brief Exception class for the Client class.
     */
    class Client_error : public ::std::exception
    {
    public:
        Client_error(::std::string msg = "unexpected client error") : msg{msg} {}
        virtual ~Client_error() {}

        const char *what()
        {
            return msg.c_str();
        }

    private:
        const ::std::string msg;

        // Delete default constructor
        Client_error() = delete;

        // Disallow copy
        Client_error(const Client_error &) = delete;
        Client_error &operator=(const Client_error &) = delete;
    };

    /**
     * @brief Class to manage running flag in threads.
     *
     */
    using RunningFlag = ::std::atomic_bool;
    class Client_running_manager
    {
    public:
        Client_running_manager(RunningFlag &flag) : flag{flag} {}
        virtual ~Client_running_manager()
        {
            flag = false;
        }

    private:
        RunningFlag &flag;

        // Delete default constructor
        Client_running_manager() = delete;

        // Disallow copy
        Client_running_manager(const Client_running_manager &) = delete;
        Client_running_manager &operator=(const Client_running_manager &) = delete;
    };

    /**
     * @brief Template class for the Client class.
     *
     * @param SocketType
     * @param SocketDeleter
     */
    template <class SocketType, class SocketDeleter = ::std::default_delete<SocketType>>
    class Client
    {
    public:
        /**
         * @brief Constructor for continuous stream forwarding
         *
         * @param os    Stream to forward incoming stream to
         */
        Client(::std::ostream &os) : CONTINUOUS_OUTPUT_STREAM{os},
                                     DELIMITER_FOR_FRAGMENTATION{0},
                                     APPEND_STRING_FOR_FRAGMENTATION{0},
                                     APPEND_STRING_FOR_FRAGMENTATION_LENGTH{0},
                                     MAXIMUM_MESSAGE_LENGTH_FOR_FRAGMENTATION{0},
                                     MESSAGE_FRAGMENTATION_ENABLED{false} {}

        /**
         * @brief Constructor for fragmented messages
         *
         * @param delimiter     Character to split messages on
         * @param messageAppend String to append to the end of each fragmented message (before the delimiter)
         * @param messageMaxLen Maximum message length (actual message + length of append string)
         */
        Client(char delimiter, const ::std::string &messageAppend, size_t messageMaxLen) : CONTINUOUS_OUTPUT_STREAM{nullstream},
                                                                                           DELIMITER_FOR_FRAGMENTATION{delimiter},
                                                                                           APPEND_STRING_FOR_FRAGMENTATION{messageAppend},
                                                                                           APPEND_STRING_FOR_FRAGMENTATION_LENGTH{messageAppend.size()},
                                                                                           MAXIMUM_MESSAGE_LENGTH_FOR_FRAGMENTATION{messageMaxLen},
                                                                                           MESSAGE_FRAGMENTATION_ENABLED{true} {} // TODO: Add check if messageAppend is too long (more than messageMaxLen bytes)

        virtual ~Client() {}

        /**
         * @brief Start the client and connects to the server.
         * If connection to server succeeds, this method returns CLIENT_START_OK, otherwise it returns an error code.
         *
         * @param serverIp
         * @param serverPort
         * @return int
         */
        int start(const ::std::string &serverIp, const int serverPort);

        /**
         * @brief Stop the client and disconnects from the server.
         */
        void stop();

        /**
         * @brief Send a message to the server if connected.
         *
         * @param msg
         * @return true
         * @return false
         */
        bool sendMsg(const ::std::string &msg);

        /**
         * @brief Set worker executed on each incoming message in fragmentation mode
         *
         * @param worker
         */
        void setWorkOnMessage(::std::function<void(const ::std::string)> worker);

        /**
         * @brief Return if client is running
         *
         * @return bool (true if running, false if not)
         */
        bool isRunning() const;

    protected:
        /**
         * @brief Initialize the client.
         * This method is abstract and must be implemented by derived classes.
         *
         * @return int
         */
        virtual int init() = 0;

        /**
         * @brief Initialize the connection to the server.
         * This method is abstract and must be implemented by derived classes.
         *
         * @return SocketType*
         */
        virtual SocketType *connectionInit() = 0;

        /**
         * @brief Deinitialize the connection to the server.
         * This method is abstract and must be implemented by derived classes.
         */
        virtual void connectionDeinit() = 0;

        /**
         * @brief Read raw received data from the server connection.
         * This method is expected to return the received data as a string with blocking read (Empty string means failure).
         * This method is abstract and must be implemented by derived classes.
         *
         * @return string
         */
        virtual ::std::string readMsg() = 0;

        /**
         * @brief Write raw data to the server connection.
         * This method is expected to return true if the data was written successfully, otherwise false.
         * This method is abstract and must be implemented by derived classes.
         *
         * @param msg
         * @return true
         * @return false
         */
        virtual bool writeMsg(const ::std::string &msg) = 0;

        // Client sockets (TCP and user defined)
        int tcpSocket;
        ::std::unique_ptr<SocketType, SocketDeleter> clientSocket{nullptr};

        // Maximum package size for receiving data
        const static int MAXIMUM_RECEIVE_PACKAGE_SIZE{16384};

    private:
        /**
         * @brief Read incoming data from the server connection.
         * This method runs infinitely until the client is stopped.
         */
        void receive();

        // Flag to indicate if the client is running
        RunningFlag running{false};

        // Client socket address
        struct sockaddr_in socketAddress
        {
        };

        // Thread for receiving data from the server
        ::std::thread recHandler{};

        // All working threads and their running status
        ::std::vector<::std::thread> workHandlers;
        ::std::vector<::std::unique_ptr<RunningFlag>> workHandlersRunning;

        // Pointer to worker function for incoming messages (for fragmentation mode only)
        ::std::function<void(const ::std::string)> workOnMessage{nullptr};

        // Out stream to forward continuous input stream to
        ::std::ostream &CONTINUOUS_OUTPUT_STREAM;

        // Delimiter for the message framing (incoming and outgoing) (default is '\n')
        const char DELIMITER_FOR_FRAGMENTATION;

        // Append this string to the end of each outgoing fragmented message
        const ::std::string APPEND_STRING_FOR_FRAGMENTATION;
        const size_t APPEND_STRING_FOR_FRAGMENTATION_LENGTH;

        // Maximum message length (incoming and outgoing)
        const size_t MAXIMUM_MESSAGE_LENGTH_FOR_FRAGMENTATION;

        // Flag if messages shall be fragmented
        const bool MESSAGE_FRAGMENTATION_ENABLED;

        // Buffer/Stream doing nothing
        NullBuffer nullbuffer;
        ::std::ostream nullstream{&nullbuffer};

        // Disallow copy
        Client() = delete;
        Client(const Client &) = delete;
        Client &operator=(const Client &) = delete;
    };

    // ============================== Implementation of non-abstract methods. ==============================
    // ====================== Must be in header file because of the template class. =======================

    template <class SocketType, class SocketDeleter>
    int Client<SocketType, SocketDeleter>::start(const ::std::string &serverIp, const int serverPort)
    {
        // Check if client is already running
        // If so, return with error
        if (running)
        {
#ifdef DEVELOP
            ::std::cerr << DEBUGINFO << ": Client already running" << ::std::endl;
#endif // DEVELOP

            return -1;
        }

        // Check if the port number is valid
        if (1 > serverPort || 65535 < serverPort)
        {
#ifdef DEVELOP
            ::std::cerr << DEBUGINFO << ": The port " << serverPort << " couldn't be used" << ::std::endl;
#endif // DEVELOP

            return CLIENT_ERROR_START_WRONG_PORT;
        }

        // Initialize the client
        // If initialization fails, return with error
        int initCode{init()};
        if (initCode)
            return initCode;

        // Create the client tcp socket and connect to the server
        // If socket creation fails, stop client and return with error
        tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (-1 == tcpSocket)
        {
#ifdef DEVELOP
            ::std::cerr << DEBUGINFO << ": Error while creating client TCP socket" << ::std::endl;
#endif // DEVELOP

            stop();
            return CLIENT_ERROR_START_CREATE_SOCKET;
        }

        // Set the socket options
        // If setting fails, stop client and return with error
        int opt{0};
        if (setsockopt(tcpSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        {
#ifdef DEVELOP
            ::std::cerr << DEBUGINFO << ": Error while setting TCP socket options" << ::std::endl;
#endif // DEVELOP

            stop();
            return CLIENT_ERROR_START_SET_SOCKET_OPT;
        }

        // Initialize the socket address
        struct hostent *serverHost;
        serverHost = gethostbyname(serverIp.c_str());
        memset(&socketAddress, 0, sizeof(socketAddress));
        socketAddress.sin_family = AF_INET;
        socketAddress.sin_addr.s_addr = *(uint32_t *)serverHost->h_addr_list[0];
        socketAddress.sin_port = htons(serverPort);

        // Connect to the server via unencrypted TCP
        // If connection fails, stop client and return with error
        if (connect(tcpSocket, (struct sockaddr *)&socketAddress, sizeof(socketAddress)))
        {
#ifdef DEVELOP
            ::std::cerr << DEBUGINFO << ": Error while connecting to server" << ::std::endl;
#endif // DEVELOP

            stop();
            return CLIENT_ERROR_START_CONNECT;
        }

        // Initialize the TCP connection to the server
        // If initialization fails, stop client and return with error
        clientSocket.reset(connectionInit());
        if (!clientSocket.get())
        {
            stop();
            return CLIENT_ERROR_START_CONNECT_INIT;
        }

        // Receive incoming data from the server infinitely in the background while the client is running
        // If background task already exists, return with error
        if (recHandler.joinable())
            throw Client_error("Error while starting background receive task. Background task already exists");
        recHandler = ::std::thread{&Client::receive, this};

        // Client is now running
        running = true;

#ifdef DEVELOP
        ::std::cout << DEBUGINFO << ": Client started" << ::std::endl;
#endif // DEVELOP

        return CLIENT_START_OK;
    }

    template <class SocketType, class SocketDeleter>
    void Client<SocketType, SocketDeleter>::stop()
    {
        // Stop the client
        running = false;

        // Block the TCP socket to abort receiving process
        connectionDeinit();
        int shut{shutdown(tcpSocket, SHUT_RDWR)};

        // Wait for the background receive thread to finish
        if (recHandler.joinable())
            recHandler.join();

        // If shutdown failed, abort stop here
        if (shut)
            return;

        // Close the TCP socket
        close(tcpSocket);

#ifdef DEVELOP
        ::std::cout << DEBUGINFO << ": Client stopped" << ::std::endl;
#endif // DEVELOP

        return;
    }

    template <class SocketType, class SocketDeleter>
    bool Client<SocketType, SocketDeleter>::sendMsg(const ::std::string &msg)
    {
        if (MESSAGE_FRAGMENTATION_ENABLED)
        {
            // Check if message doesn't contain delimiter
            if (msg.find(DELIMITER_FOR_FRAGMENTATION) != ::std::string::npos)
            {
#ifdef DEVELOP
                ::std::cerr << DEBUGINFO << ": Message contains delimiter" << ::std::endl;
#endif // DEVELOP

                return false;
            }

            // Check if message is too long
            if (msg.length() > MAXIMUM_MESSAGE_LENGTH_FOR_FRAGMENTATION + APPEND_STRING_FOR_FRAGMENTATION_LENGTH)
            {
#ifdef DEVELOP
                ::std::cerr << DEBUGINFO << ": Message is too long" << ::std::endl;
#endif // DEVELOP

                return false;
            }
        }

        // Send the message to the server with leading and trailing characters to indicate the message length
        if (running)
            return writeMsg(MESSAGE_FRAGMENTATION_ENABLED ? msg + APPEND_STRING_FOR_FRAGMENTATION + ::std::string{DELIMITER_FOR_FRAGMENTATION} : msg);

#ifdef DEVELOP
        ::std::cerr << DEBUGINFO << ": Client not running" << ::std::endl;
#endif // DEVELOP

        return false;
    }

    template <class SocketType, class SocketDeleter>
    void Client<SocketType, SocketDeleter>::setWorkOnMessage(::std::function<void(const ::std::string)> worker)
    {
        workOnMessage = worker;
    }

    template <class SocketType, class SocketDeleter>
    void Client<SocketType, SocketDeleter>::receive()
    {
        // Do receive loop until client is stopped
        ::std::string buffer;
        while (1)
        {
            // Wait for incoming data from the server
            // This method blocks until data is received
            // An empty string is returned if the connection is crashed
            ::std::string msg{readMsg()};
            if (msg.empty())
            {
#ifdef DEVELOP
                ::std::cout << DEBUGINFO << ": Connection to server lost" << ::std::endl;
#endif // DEVELOP

                // Stop the client
                running = false;

                // Wait for all work handlers to finish
                for (auto &it : workHandlers)
                    it.join();

                // Block the TCP socket to abort receiving process
                // If shutdown failed, abort stop here
                connectionDeinit();
                if (shutdown(tcpSocket, SHUT_RDWR))
                    return;

                // Close the TCP socket
                close(tcpSocket);

                return;
            }

            // If stream shall be fragmented ...
            if (MESSAGE_FRAGMENTATION_ENABLED)
            {
                // Get raw message separated by delimiter
                // If delimiter is found, the message is split into two parts
                size_t delimiter_pos{msg.find(DELIMITER_FOR_FRAGMENTATION)};
                while (::std::string::npos != delimiter_pos)
                {
                    ::std::string msg_part{msg.substr(0, delimiter_pos)};
                    msg = msg.substr(delimiter_pos + 1);
                    delimiter_pos = msg.find(DELIMITER_FOR_FRAGMENTATION);

                    // Check if the message is too long
                    if (buffer.size() + msg_part.size() > MAXIMUM_MESSAGE_LENGTH_FOR_FRAGMENTATION)
                    {
#ifdef DEVELOP
                        ::std::cerr << DEBUGINFO << ": Message from server is too long" << ::std::endl;
#endif // DEVELOP

                        buffer.clear();
                        continue;
                    }

                    buffer += msg_part;

#ifdef DEVELOP
                    ::std::cout << DEBUGINFO << ": Received message from server: " << buffer << ::std::endl;
#endif // DEVELOP

                    ::std::unique_ptr<RunningFlag> workRunning{new RunningFlag{true}};
                    ::std::thread work_t{[this](RunningFlag *workRunning_p, ::std::string buffer)
                                         {
                                             // Mark thread as running
                                             Client_running_manager running_mgr{*workRunning_p};

                                             // Run code to handle the incoming message
                                             if (workOnMessage)
                                                 workOnMessage(::std::move(buffer));

                                             return;
                                         },
                                         workRunning.get(), ::std::move(buffer)};

                    // Remove all finished work handlers from the vector
                    size_t workHandlers_s{workHandlersRunning.size()};
                    for (size_t i{0}; i < workHandlers_s; i += 1)
                    {
                        if (!*workHandlersRunning[i].get())
                        {
                            workHandlers[i].join();
                            workHandlers.erase(workHandlers.begin() + i);
                            workHandlersRunning.erase(workHandlersRunning.begin() + i);
                            i -= 1;
                            workHandlers_s -= 1;
                        }
                    }

                    workHandlers.push_back(::std::move(work_t));
                    workHandlersRunning.push_back(::std::move(workRunning));
                }
                buffer += msg;
            }

            // If stream shall be forwarded to continuous out stream ...
            else
            {
                // Just forward incoming message to output stream
                CONTINUOUS_OUTPUT_STREAM << msg << ::std::flush;
            }
        }
    }

    template <class SocketType, class SocketDeleter>
    bool Client<SocketType, SocketDeleter>::isRunning() const
    {
        return running;
    }
}

#endif // CLIENT_HPP_
