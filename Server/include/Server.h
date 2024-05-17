/**
 * @file Server.h
 * @author Nils Henrich
 * @brief Base framework for all classes that build a network server based on TCP.
 * This class contains no functionality, but serves as a base framework for the creation of stable servers based on TCP.
 * When compiling with the -DDEBUG flag, the class will print out all received messages to the console.
 * @version 1.0
 * @date 2021-12-27
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef SERVER_H
#define SERVER_H

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

#include "ServerDefines.h"

namespace tcp
{
    /**
     * @brief Exception class for the Server class.
     */
    class Server_error : public std::exception
    {
    public:
        Server_error(std::string msg = "unexpected server error") : msg{msg} {}
        virtual ~Server_error() {}

        const char *what()
        {
            return msg.c_str();
        }

    private:
        const std::string msg;

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
    using RunningFlag = std::atomic_bool;
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
    template <class SocketType, class SocketDeleter = std::default_delete<SocketType>>
    class Server
    {
    public:
        /**
         * @brief Constructor for continuous stream forwarding
         */
        Server() : DELIMITER_FOR_FRAGMENTATION{0},
                   MAXIMUM_MESSAGE_LENGTH_FOR_FRAGMENTATION{0},
                   MESSAGE_FRAGMENTATION_ENABLED{false} {}

        /**
         * @brief Constructor for fragmented messages
         *
         * @param delimiter     Character to split messages on
         * @param messageMaxLen Maximum message length
         */
        Server(char delimiter, size_t messageMaxLen) : DELIMITER_FOR_FRAGMENTATION{delimiter},
                                                       MAXIMUM_MESSAGE_LENGTH_FOR_FRAGMENTATION{messageMaxLen},
                                                       MESSAGE_FRAGMENTATION_ENABLED{true} {}

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
         * @param pathToCaCert
         * @param pathToCert
         * @param pathToPrivKey
         * @return int (SERVER_START_OK if successful, see ServerDefines.h for other return values)
         */
        int start(const int port,
                  const char *const pathToCaCert = nullptr,
                  const char *const pathToCert = nullptr,
                  const char *const pathToPrivKey = nullptr);

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
        bool sendMsg(const int clientId, const std::string &msg);

        /**
         * @brief Set worker executed on each incoming message in fragmentation mode
         *
         * @param worker
         */
        void setWorkOnMessage(std::function<void(const int, const std::string)> worker);

        /**
         * @brief Set creator creating a forwarding out stream for each established connection in continuous mode
         *
         * @param creator
         */
        void setCreateForwardStream(std::function<std::ostream *(const int)> creator);

        /**
         * @brief Set worker executed on each new established connection
         *
         * @param worker
         */
        void setWorkOnEstablished(std::function<void(const int)> worker);

        /**
         * @brief Set worker executed on each closed connection
         *
         * @param worker
         */
        void setWorkOnClosed(std::function<void(const int)> worker);

        /**
         * @brief Get all connected clients identified by ID as list
         *
         * @return vector<int>
         */
        std::vector<int> getAllClientIds() const;

        /**
         * @brief Get the IP address of a specific connected client (Identified by its TCP ID).
         *
         * @param clientId
         * @return std::string
         */
        std::string getClientIp(const int clientId) const;

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
         * @param pathToCaCert
         * @param pathToCert
         * @param pathToPrivKey
         * @return int
         */
        virtual int init(const char *const pathToCaCert,
                         const char *const pathToCert,
                         const char *const pathToPrivKey) = 0;

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
         * @return std::string
         */
        virtual std::string readMsg(SocketType *socket) = 0;

        /**
         * @brief Send raw data to a specific client (Identified by its TCP ID).
         * This method is called by the sendMsg method.
         * This method is abstract and must be implemented by derived classes.
         *
         * @param clientId
         * @param msg
         * @return bool
         */
        virtual bool writeMsg(const int clientId, const std::string &msg) = 0;

        // Map to store all active connections with their identifying TCP ID
        std::map<int, std::unique_ptr<SocketType, SocketDeleter>> activeConnections{};

        // Mutex to protect the activeConnections map
        std::mutex activeConnections_m{};

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
        std::thread accHandler{};

        // All receiving threads (One per connected client) and their running status
        std::map<int, std::thread> recHandlers{};
        std::map<int, std::unique_ptr<RunningFlag>> recHandlersRunning{};

        // Flag to indicate if the server is running
        RunningFlag running{false};

        // Pointer to a function that returns an out stream to forward incoming data to
        std::function<std::ostream *(const int)> generateNewForwardStream{nullptr};
        std::map<int, std::unique_ptr<std::ostream>> forwardStreams;

        // Pointer to worker functions on incoming message (for fragmentation mode only), established or closed connection
        std::function<void(const int, const std::string)> workOnMessage{nullptr};
        std::function<void(const int)> workOnEstablished{nullptr};
        std::function<void(const int)> workOnClosed{nullptr};

        // Delimiter for the message framing (incoming and outgoing) (default is '\n')
        const char DELIMITER_FOR_FRAGMENTATION;

        // Maximum message length (incoming and outgoing) (default is 2³² - 2 = 4294967294)
        const size_t MAXIMUM_MESSAGE_LENGTH_FOR_FRAGMENTATION;

        // Flag if messages shall be fragmented
        const bool MESSAGE_FRAGMENTATION_ENABLED;

        // Disallow copy
        Server(const Server &) = delete;
        Server &operator=(const Server &) = delete;
    };

    // ============================== Implementation of non-abstract methods. ==============================
    // ====================== Must be in header file because of the template class. =======================

#include "Server.tpp"
}

#endif // SERVER_H
