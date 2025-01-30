/**
 * @file TcpClient.hpp
 * @author Nils Henrich
 * @brief TCP client for unencrypted data transfer without authentication.
 * @version 3.1.0
 * @date 2021-12-27
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef TCPCLIENT_HPP_
#define TCPCLIENT_HPP_

#include <limits>

#include "template/Client.hpp"

namespace tcp
{
    class TcpClient : public Client<int>
    {
    public:
        /**
         * @brief Constructor for continuous stream forwarding
         *
         * @param os    Stream to forward incoming stream to
         */
        TcpClient(::std::ostream &os = ::std::cout) : Client(os) {}

        /**
         * @brief Constructor for fragmented messages
         *
         * @param delimiter     Character to split messages on
         * @param messageAppend String to append to the end of each fragmented message (before the delimiter)
         * @param messageMaxLen Maximum message length (actual message + length of append string) (default is 2³² - 2 = 4294967294)
         */
        TcpClient(char delimiter, const ::std::string &messageAppend = "", size_t messageMaxLen = ::std::numeric_limits<size_t>::max() - 1) : Client(delimiter, messageAppend, messageMaxLen) {}

        /**
         * @brief Destructor
         */
        virtual ~TcpClient() { stop(); }

    private:
        /**
         * @brief Initialize the client
         * Do nothing for TCP client
         *
         * @return int
         */
        int init() override final { return 0; }

        /**
         * @brief Initialize the connection
         * Just return pointer to the TCP socket
         *
         * @return int*
         */
        int *connectionInit() override final { return new int{tcpSocket}; }

        /**
         * @brief Deinitialize the connection (Do nothing)
         */
        void connectionDeinit() override final {}

        /**
         * @brief Read raw data from the unencrypted TCP socket
         *
         * @return string
         */
        ::std::string readMsg() override final
        {
            // Buffer to store the data received from the server
            char buffer[MAXIMUM_RECEIVE_PACKAGE_SIZE]{0};

            // Wait for the server to send data
            ssize_t lenMsg{recv(tcpSocket, buffer, MAXIMUM_RECEIVE_PACKAGE_SIZE, 0)};

            // Return the received message as a string (Empty string if receive failed)
            return ::std::string{buffer, 0 < lenMsg ? static_cast<size_t>(lenMsg) : 0UL};
        }

        /**
         * @brief Send raw data to the unencrypted TCP socket
         *
         * @param msg
         * @return true
         * @return bool
         */
        bool writeMsg(const ::std::string &msg) override final
        {
#ifdef DEVELOP
            ::std::cout << DEBUGINFO << ": Send to server: " << msg << ::std::endl;
#endif // DEVELOP

            const size_t lenMsg{msg.size()};
            return send(tcpSocket, msg.c_str(), lenMsg, 0) == (ssize_t)lenMsg;
        }

        // Disallow copy
        TcpClient(const TcpClient &) = delete;
        TcpClient &operator=(const TcpClient &) = delete;
    };
}

#endif // TCPCLIENT_HPP_
