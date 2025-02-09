/**
 * @file Server.hpp
 * @author Nils Henrich
 * @brief Base framework for all classes that build a network server based on TCP.
 * This class contains no functionality, but serves as a base framework for the creation of stable servers based on TCP.
 * When compiling with the -DDEBUG flag, the class will print out all received messages to the console.
 * @version 3.2.0
 * @date 2021-12-27
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <cstring>
#include <exception>
#include <atomic>
#include <memory>
#include <functional>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// Debugging output
#ifdef DEVELOP
#define DEBUGINFO "(" << typeid(this).name() << " at " << this << ")::" << __func__
#endif // DEVELOP

namespace tcp
{
    // Return codes
    enum : int
    {
        SERVER_START_OK = 0,                     // Server started successfully
        SERVER_ERROR_START_WRONG_PORT = 10,      // Server could not start because of wrong port number
        SERVER_ERROR_START_SET_CONTEXT = 20,     // Server could not start because of SSL context error
        SERVER_ERROR_START_WRONG_CA_PATH = 30,   // Server could not start because of wrong path to CA cert file
        SERVER_ERROR_START_WRONG_CERT_PATH = 31, // Server could not start because of wrong path to certificate file
        SERVER_ERROR_START_WRONG_KEY_PATH = 32,  // Server could not start because of wrong path to key file
        SERVER_ERROR_START_WRONG_CA = 33,        // Server could not start because of bad CA cert file
        SERVER_ERROR_START_WRONG_CERT = 34,      // Server could not start because of bad certificate file
        SERVER_ERROR_START_WRONG_KEY = 35,       // Server could not start because of bad key file or non matching key with certificate
        SERVER_ERROR_START_CREATE_SOCKET = 40,   // Server could not start because of TCP socket creation error
        SERVER_ERROR_START_SET_SOCKET_OPT = 41,  // Server could not start because of TCP socket option error
        SERVER_ERROR_START_BIND_PORT = 42,       // Server could not start because of TCP socket bind error
        SERVER_ERROR_START_SERVER = 43           // Server could not start because of TCP socket listen error
    };
    /**
     * @brief Exception class for the Server class.
     */
    class Server_error : public ::std::exception
    {
    public:
        Server_error(::std::string msg = "unexpected server error") : msg{msg} {}
        virtual ~Server_error() {}

        const char *what()
        {
            return msg.c_str();
        }

    private:
        const ::std::string msg;

        // Delete default constructor
        Server_error() = delete;

        // Disallow copy
        Server_error(const Server_error &) = delete;
        Server_error &operator=(const Server_error &) = delete;
    };

    /**
     * @brief Class to manage running flag in threads.
     *
     */
    using RunningFlag = ::std::atomic_bool;
    class Server_running_manager
    {
    public:
        Server_running_manager(RunningFlag &flag) : flag{flag} {}
        virtual ~Server_running_manager()
        {
            flag = false;
        }

    private:
        RunningFlag &flag;

        // Delete default constructor
        Server_running_manager() = delete;

        // Disallow copy
        Server_running_manager(const Server_running_manager &) = delete;
        Server_running_manager &operator=(const Server_running_manager &) = delete;
    };

    /**
     * @brief Template class for the Server class.
     * A usable server class must be derived from this class with specific socket type (int for unencrypted TCP, SSL for TLS).
     *
     * @param SocketType
     * @param SocketDeleter
     */
    template <class SocketType, class SocketDeleter = ::std::default_delete<SocketType>>
    class Server
    {
    public:
        /**
         * @brief Constructor for continuous stream forwarding
         */
        Server() : DELIMITER_FOR_FRAGMENTATION{0},
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
        Server(char delimiter, const ::std::string &messageAppend, size_t messageMaxLen) : DELIMITER_FOR_FRAGMENTATION{delimiter},
                                                                                           APPEND_STRING_FOR_FRAGMENTATION{messageAppend},
                                                                                           APPEND_STRING_FOR_FRAGMENTATION_LENGTH{messageAppend.size()},
                                                                                           MAXIMUM_MESSAGE_LENGTH_FOR_FRAGMENTATION{messageMaxLen},
                                                                                           MESSAGE_FRAGMENTATION_ENABLED{true} {} // TODO: Add check if messageAppend is too long (more than messageMaxLen bytes)

        /**
         * @brief Destructor
         */
        virtual ~Server() {}

        /**
         * @brief Starts the server.
         * If server was started successfully (return value SERVER_START_OK), the server can accept new connections and send and receive data.
         * If encryption should be used, the server must be started with the correct path to the CA certificate and the correct path to the certificate and key file.
         *
         * @param port
         * @return int (SERVER_START_OK if successful, see ServerDefines.h for other return values)
         */
        int start(const int port);

        /**
         * @brief Stops the server.
         * When stopping the server, all active connections are closed.
         */
        void stop();

        /**
         * @brief Sends a message to a specific client (Identified by its TCP ID).
         *
         * @param clientId
         * @param msg
         * @return bool (true if successful, false if not)
         */
        bool sendMsg(const int clientId, const ::std::string &msg);

        /**
         * @brief Set worker executed on each incoming message in fragmentation mode
         *
         * @param worker
         */
        void setWorkOnMessage(::std::function<void(const int, const ::std::string)> worker);

        /**
         * @brief Set creator creating a forwarding out stream for each established connection in continuous mode
         *
         * @param creator
         */
        void setCreateForwardStream(::std::function<::std::ostream *(const int)> creator);

        /**
         * @brief Set worker executed on each new established connection
         *
         * @param worker
         */
        void setWorkOnEstablished(::std::function<void(const int)> worker);

        /**
         * @brief Set worker executed on each closed connection
         *
         * @param worker
         */
        void setWorkOnClosed(::std::function<void(const int)> worker);

        /**
         * @brief Get all connected clients identified by ID as list
         *
         * @return vector<int>
         */
        ::std::vector<int> getAllClientIds() const;

        /**
         * @brief Get the IP address of a specific connected client (Identified by its TCP ID).
         *
         * @param clientId
         * @return string
         */
        ::std::string getClientIp(const int clientId) const;

        /**
         * @brief Return if server is running
         *
         * @return bool (true if running, false if not)
         */
        bool isRunning() const;

    protected:
        /**
         * @brief Initializes the server just before starting it.
         * This method is abstract and must be implemented by derived classes.
         *
         * @return int
         */
        virtual int init() = 0;

        /**
         * @brief Initializes a new connection just after accepting it on unencrypted TCP level.
         * The returned socket is used to communicate with the client.
         * This method is abstract and must be implemented by derived classes.
         *
         * @param clientId
         * @return SocketType*
         */
        virtual SocketType *connectionInit(const int clientId) = 0;

        /**
         * @brief Deinitialize a connection just before closing it.
         * This method is abstract and must be implemented by derived classes.
         *
         * @param socket
         */
        virtual void connectionDeinit(SocketType *socket) = 0;

        /**
         * @brief Read raw received data from a specific client (Identified by its TCP ID).
         * This method is expected to return the read raw data as a string with blocking read (Empty string means failure).
         * This method is abstract and must be implemented by derived classes.
         *
         * @param socket
         * @return string
         */
        virtual ::std::string readMsg(SocketType *socket) = 0;

        /**
         * @brief Send raw data to a specific client (Identified by its TCP ID).
         * This method is called by the sendMsg method.
         * This method is abstract and must be implemented by derived classes.
         *
         * @param clientId
         * @param msg
         * @return bool
         */
        virtual bool writeMsg(const int clientId, const ::std::string &msg) = 0;

        // Map to store all active connections with their identifying TCP ID
        ::std::map<int, ::std::unique_ptr<SocketType, SocketDeleter>> activeConnections{};

        // Mutex to protect the activeConnections map
        ::std::mutex activeConnections_m{};

        // Maximum TCP packet size
        const static int MAXIMUM_RECEIVE_PACKAGE_SIZE{16384};

    private:
        /**
         * @brief Listen for new connections requests.
         * This method runs infinitely in a separate thread while the server is running.
         */
        void listenConnection();

        /**
         * @brief Listen for incoming data from a specific connected client (Identified by its TCP ID).
         * This method runs infinitely in a separate thread while the specific client is connected.
         *
         * @param clientId
         */
        void listenMessage(const int clientId, RunningFlag *const recRunning_p);

        // Socket address for the server
        struct sockaddr_in socketAddress
        {
        };

        // TCP socket for the server to accept new connections
        int tcpSocket{0};

        // Thread to accept new connections
        ::std::thread accHandler{};

        // All receiving threads (One per connected client) and their running status
        ::std::map<int, ::std::thread> recHandlers{};
        ::std::map<int, ::std::unique_ptr<RunningFlag>> recHandlersRunning{};

        // Flag to indicate if the server is running
        RunningFlag running{false};

        // Pointer to a function that returns an out stream to forward incoming data to
        ::std::function<::std::ostream *(const int)> generateNewForwardStream{nullptr};
        ::std::map<int, ::std::unique_ptr<::std::ostream>> forwardStreams;

        // Pointer to worker functions on incoming message (for fragmentation mode only), established or closed connection
        ::std::function<void(const int, const ::std::string)> workOnMessage{nullptr};
        ::std::function<void(const int)> workOnEstablished{nullptr};
        ::std::function<void(const int)> workOnClosed{nullptr};

        // Delimiter for the message framing (incoming and outgoing)
        const char DELIMITER_FOR_FRAGMENTATION;

        // Append this string to the end of each outgoing fragmented message
        const ::std::string APPEND_STRING_FOR_FRAGMENTATION;
        const size_t APPEND_STRING_FOR_FRAGMENTATION_LENGTH;

        // Maximum message length (incoming and outgoing)
        const size_t MAXIMUM_MESSAGE_LENGTH_FOR_FRAGMENTATION;

        // Flag if messages shall be fragmented
        const bool MESSAGE_FRAGMENTATION_ENABLED;

        // Disallow copy
        Server(const Server &) = delete;
        Server &operator=(const Server &) = delete;
    };

    // ============================== Implementation of non-abstract methods. ==============================
    // ====================== Must be in header file because of the template class. =======================

    template <class SocketType, class SocketDeleter>
    int Server<SocketType, SocketDeleter>::start(const int port)
    {
        // If the server is already running, return error
        if (running)
        {
#ifdef DEVELOP
            ::std::cerr << DEBUGINFO << ": Server already running" << ::std::endl;
#endif // DEVELOP

            return -1;
        }

        // Check if the port number is valid
        if (1 > port || 65535 < port)
        {
#ifdef DEVELOP
            ::std::cerr << DEBUGINFO << ": The port " << port << " couldn't be used" << ::std::endl;
#endif // DEVELOP

            return SERVER_ERROR_START_WRONG_PORT;
        }

        // Initialize the server and return error if it fails
        int initCode{init()};
        if (initCode)
            return initCode;

        // Create the TCP socket for the server to accept new connections.
        // Return error if it fails
        tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (-1 == tcpSocket)
        {
#ifdef DEVELOP
            ::std::cerr << DEBUGINFO << ": Error when creating TCP socket to listen on" << ::std::endl;
#endif // DEVELOP

            // Stop the server
            stop();

            return SERVER_ERROR_START_CREATE_SOCKET;
        }

        // Set options on the TCP socket for the server to accept new connections.
        // (Reuse address)
        // Return error if it fails
        int opt{0};
        if (setsockopt(tcpSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        {
#ifdef DEVELOP
            ::std::cerr << DEBUGINFO << ": Error when setting TCP socket options" << ::std::endl;
#endif // DEVELOP

            // Stop the server
            stop();

            return SERVER_ERROR_START_SET_SOCKET_OPT;
        }

        // Initialize the socket address for the server.
        memset(&socketAddress, 0, sizeof(socketAddress));
        socketAddress.sin_family = AF_INET;
        socketAddress.sin_addr.s_addr = INADDR_ANY;
        socketAddress.sin_port = htons(port);

        // Bind the TCP socket for the server to accept new connections to the socket address.
        // Return error if it fails
        if (bind(tcpSocket, (struct sockaddr *)&socketAddress, sizeof(socketAddress)))
        {
#ifdef DEVELOP
            ::std::cerr << DEBUGINFO << ": Error when binding server to port " << port << ::std::endl;
#endif // DEVELOP

            // Stop the server
            stop();

            return SERVER_ERROR_START_BIND_PORT;
        }

        // Start listening on the TCP socket for the server to accept new connections.
        if (listen(tcpSocket, SOMAXCONN))
        {
#ifdef DEVELOP
            ::std::cerr << DEBUGINFO << ": Error when starting listening" << ::std::endl;
#endif // DEVELOP

            // Stop the server
            stop();

            return SERVER_ERROR_START_SERVER;
        }

        // Start the thread to accept new connections
        if (accHandler.joinable())
            throw Server_error("Start server thread failed: Thread is already running");
        accHandler = ::std::thread{&Server::listenConnection, this};

        // Server is now running
        running = true;

#ifdef DEVELOP
        ::std::cout << DEBUGINFO << ": Server started on port " << port << ::std::endl;
#endif // DEVELOP

        return initCode;
    }

    template <class SocketType, class SocketDeleter>
    void Server<SocketType, SocketDeleter>::stop()
    {
        // Stop the server
        running = false;

        // Block listening TCP socket to abort all reads
        int shut{shutdown(tcpSocket, SHUT_RDWR)};

        // Wait for the accept thread to finish
        if (accHandler.joinable())
            accHandler.join();

        // If shutdown failed, abort stop here
        if (shut)
            return;

        // Close listening TCP socket
        close(tcpSocket);

#ifdef DEVELOP
        ::std::cout << DEBUGINFO << ": Server stopped" << ::std::endl;
#endif // DEVELOP

        return;
    }

    template <class SocketType, class SocketDeleter>
    bool Server<SocketType, SocketDeleter>::sendMsg(const int clientId, const ::std::string &msg)
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

        // Extend message with start and end characters and send it
        ::std::lock_guard<::std::mutex> lck{activeConnections_m};
        if (activeConnections.find(clientId) != activeConnections.end())
            return writeMsg(clientId, MESSAGE_FRAGMENTATION_ENABLED ? msg + APPEND_STRING_FOR_FRAGMENTATION + ::std::string{DELIMITER_FOR_FRAGMENTATION} : msg);

#ifdef DEVELOP
        ::std::cerr << DEBUGINFO << ": Client " << clientId << " is not connected" << ::std::endl;
#endif // DEVELOP

        return false;
    }

    template <class SocketType, class SocketDeleter>
    void Server<SocketType, SocketDeleter>::setWorkOnMessage(::std::function<void(const int, const ::std::string)> worker)
    {
        workOnMessage = worker;
    }

    template <class SocketType, class SocketDeleter>
    void Server<SocketType, SocketDeleter>::setCreateForwardStream(::std::function<::std::ostream *(const int)> creator)
    {
        generateNewForwardStream = creator;
    }

    template <class SocketType, class SocketDeleter>
    void Server<SocketType, SocketDeleter>::setWorkOnEstablished(::std::function<void(const int)> worker)
    {
        workOnEstablished = worker;
    }

    template <class SocketType, class SocketDeleter>
    void Server<SocketType, SocketDeleter>::setWorkOnClosed(::std::function<void(const int)> worker)
    {
        workOnClosed = worker;
    }

    template <class SocketType, class SocketDeleter>
    std::vector<int> Server<SocketType, SocketDeleter>::getAllClientIds() const
    {
        ::std::vector<int> ret;
        for (auto &v : activeConnections)
            ret.push_back(v.first);
        return ret;
    }

    template <class SocketType, class SocketDeleter>
    std::string Server<SocketType, SocketDeleter>::getClientIp(const int clientId) const
    {
        struct sockaddr_in addr;
        socklen_t addrSize = sizeof(struct sockaddr_in);
        if (getpeername(clientId, (struct sockaddr *)&addr, &addrSize))
        {
#ifdef DEVELOP
            ::std::cerr << DEBUGINFO << ": Error reading client " << clientId << "'s IP address" << ::std::endl;
#endif // DEVELOP

            return "Failed Read!";
        }

        // Convert the IP address to a string and return it
        return ::std::string{inet_ntoa(addr.sin_addr)};
    }

    template <class SocketType, class SocketDeleter>
    void Server<SocketType, SocketDeleter>::listenConnection()
    {
        // Get the size of the socket address for the server (important for connection establishment)
        socklen_t socketAddress_len{sizeof(socketAddress)};

        // Accept new connections while the server is running
        while (running)
        {
            // Wait for a new connection to accept
            const int newConnection{accept(tcpSocket, (struct sockaddr *)&socketAddress, &socketAddress_len)};

            // If new accepted connection ID is -1, the accept failed
            // In this case, continue with accepting the new connections
            if (-1 == newConnection)
                continue;

#ifdef DEVELOP
            ::std::cout << DEBUGINFO << ": New client connected: " << newConnection << ::std::endl;
#endif // DEVELOP

            // Initialize the (so far unencrypted) connection
            SocketType *connection_p{connectionInit(newConnection)};
            if (!connection_p)
                continue;

            // Add connection to active connections
            {
                ::std::lock_guard<::std::mutex> lck{activeConnections_m};
                activeConnections[newConnection] = ::std::unique_ptr<SocketType, SocketDeleter>{connection_p};
            }

            // When a new connection is established, the incoming messages of this connection should be read in a new process
            ::std::unique_ptr<RunningFlag> recRunning{new RunningFlag{true}};
            ::std::thread rec_t{&Server::listenMessage, this, newConnection, recRunning.get()};

            // Get all finished receive handlers
            ::std::vector<int> toRemove;
            for (auto &flag : recHandlersRunning)
            {
                if (!*flag.second.get())
                    toRemove.push_back(flag.first);
            }

            // Remove finished receive handlers
            for (auto &id : toRemove)
            {
                recHandlers[id].join();
                recHandlers.erase(id);
                recHandlersRunning.erase(id);
            }

            // Add new receive handler (Running flag is added inside receive thread)
            recHandlers[newConnection] = ::std::move(rec_t);
            recHandlersRunning[newConnection] = ::std::move(recRunning);
        }

        // Abort receiving for all active connections by shutting down the read channel
        // Complete shutdown and close is done in receive threads
        {
            ::std::lock_guard<::std::mutex> lck{activeConnections_m};
            for (const auto &it : activeConnections)
            {
                shutdown(it.first, SHUT_RD);

#ifdef DEVELOP
                ::std::cout << DEBUGINFO << ": Closed connection to client " << it.first << ::std::endl;
#endif // DEVELOP
            }
        }

        // Wait for all receive processes to finish
        for (auto &it : recHandlers)
            it.second.join();

        return;
    }

    template <class SocketType, class SocketDeleter>
    void Server<SocketType, SocketDeleter>::listenMessage(const int clientId, RunningFlag *const recRunning_p)
    {
        // Mark Thread as running (Add running flag and connect to handler)
        Server_running_manager running_mgr{*recRunning_p};

        // Get connection from map
        SocketType *connection_p;
        {
            ::std::lock_guard<::std::mutex> lck{activeConnections_m};
            if (activeConnections.find(clientId) == activeConnections.end())
                return;
            connection_p = activeConnections[clientId].get();
        }

        // Create continuous stream for this connection
        if (generateNewForwardStream)
            forwardStreams[clientId] = ::std::unique_ptr<::std::ostream>{generateNewForwardStream(clientId)};

        // Run worker for new established connections
        if (workOnEstablished)
            workOnEstablished(clientId);

        // Vectors of running work handlers and their status flags
        ::std::vector<::std::thread> workHandlers;
        ::std::vector<::std::unique_ptr<RunningFlag>> workHandlersRunning;

        // Read incoming messages from this connection as long as the connection is active
        ::std::string buffer;
        while (1)
        {
            // Wait for new incoming message (implemented in derived classes)
            // If message is empty string, the connection is broken
            // BUG: Execution stucks here if server is stopped immediately after client connection
            ::std::string msg{readMsg(connection_p)};
            if (msg.empty())
            {
#ifdef DEVELOP
                ::std::cout << DEBUGINFO << ": Connection to client " << clientId << " broken" << ::std::endl;
#endif // DEVELOP

                {
                    ::std::lock_guard<::std::mutex> lck{activeConnections_m};

                    // Deinitialize the connection
                    connectionDeinit(connection_p);

                    // Block the connection from being used anymore
                    shutdown(clientId, SHUT_RDWR);

                    // Remove connection from active connections
                    activeConnections.erase(clientId);
                }

                // Run code to handle the closed connection
                if (workOnClosed)
                    workOnClosed(clientId);

                // Close the connection
                close(clientId);

                // Wait for all work handlers to finish
                for (auto &it : workHandlers)
                    it.join();

                // Remove continuous stream
                if (forwardStreams.find(clientId) != forwardStreams.end())
                    forwardStreams.erase(clientId);

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
                        ::std::cerr << DEBUGINFO << ": Message from client " << clientId << " is too long" << ::std::endl;
#endif // DEVELOP

                        buffer.clear();
                        continue;
                    }

                    buffer += msg_part;

#ifdef DEVELOP
                    ::std::cout << DEBUGINFO << ": Message from client " << clientId << ": " << buffer << ::std::endl;
#endif // DEVELOP

                    // Run code to handle the message
                    ::std::unique_ptr<RunningFlag> workRunning{new RunningFlag{true}};
                    ::std::thread work_t{[this, clientId](RunningFlag *const workRunning_p, ::std::string buffer)
                                         {
                                             // Mark Thread as running
                                             Server_running_manager running_mgr{*workRunning_p};

                                             // Run code to handle the incoming message
                                             if (workOnMessage)
                                                 workOnMessage(clientId, ::std::move(buffer));

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
                if (forwardStreams.find(clientId) != forwardStreams.end())
                    *forwardStreams[clientId].get() << msg << ::std::flush;
            }
        }
    }

    template <class SocketType, class SocketDeleter>
    bool Server<SocketType, SocketDeleter>::isRunning() const
    {
        return running;
    }
}

#endif // SERVER_HPP_
