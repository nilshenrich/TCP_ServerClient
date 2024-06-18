/**
 * @file TcpClient.hpp
 * @author Nils Henrich
 * @brief TCP client for unencrypted data transfer without authentication.
 * @version 2.0
 * @date 2021-12-27
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef TCPCLIENT_HPP_
#define TCPCLIENT_HPP_

#include <limits>

#include "template/Client.hpp"

namespace tcp_serverclient
{
    class TcpClient : public Client<int>
    {
    public:
        /**
         * @brief Constructor for continuous stream forwarding
         *
         * @param os                                Stream to forward incoming stream to
         */
        TcpClient(::std::ostream &os = ::std::cout);

        /**
         * @brief Constructor for fragmented messages
         *
         * @param delimiter                         Character to split messages on
         * @param messageMaxLen                     Maximum message length
         */
        TcpClient(char delimiter, size_t messageMaxLen = ::std::numeric_limits<size_t>::max() - 1);

        /**
         * @brief Destructor
         */
        virtual ~TcpClient();

    private:
        /**
         * @brief Initialize the client
         * Do nothing for TCP client
         *
         * @return int
         */
        int init(const char *const,
                 const char *const,
                 const char *const) override final;

        /**
         * @brief Initialize the connection
         * Just return pointer to the TCP socket
         *
         * @return int*
         */
        int *connectionInit() override final;

        /**
         * @brief Deinitialize the connection (Do nothing)
         */
        void connectionDeinit() override final;

        /**
         * @brief Read raw data from the unencrypted TCP socket
         *
         * @return string
         */
        ::std::string readMsg() override final;

        /**
         * @brief Send raw data to the unencrypted TCP socket
         *
         * @param msg
         * @return true
         * @return bool
         */
        bool writeMsg(const ::std::string &msg) override final;

        // Disallow copy
        TcpClient(const TcpClient &) = delete;
        TcpClient &operator=(const TcpClient &) = delete;
    };

    // ============================== Implementation of methods. ==============================

    TcpClient::TcpClient(::std::ostream &os) : Client(os) {}
    TcpClient::TcpClient(char delimiter, size_t messageMaxLen) : Client(delimiter, messageMaxLen) {}

    TcpClient::~TcpClient()
    {
        stop();
    }

    int TcpClient::init(const char *const,
                        const char *const,
                        const char *const)
    {
        return 0;
    }

    int *TcpClient::connectionInit()
    {
        return new int{tcpSocket};
    }

    void TcpClient::connectionDeinit()
    {
        return;
    }

    ::std::string TcpClient::readMsg()
    {
        // Buffer to store the data received from the server
        char buffer[MAXIMUM_RECEIVE_PACKAGE_SIZE]{0};

        // Wait for the server to send data
        ssize_t lenMsg{recv(tcpSocket, buffer, MAXIMUM_RECEIVE_PACKAGE_SIZE, 0)};

        // Return the received message as a string (Empty string if receive failed)
        return ::std::string{buffer, 0 < lenMsg ? static_cast<size_t>(lenMsg) : 0UL};
    }

    bool TcpClient::writeMsg(const ::std::string &msg)
    {
#ifdef DEVELOP
        ::std::cout << typeid(this).name() << "::" << __func__ << ": Send to server: " << msg << endl;
#endif // DEVELOP

        const size_t lenMsg{msg.size()};
        return send(tcpSocket, msg.c_str(), lenMsg, 0) == (ssize_t)lenMsg;
    }
}

#endif // TCPCLIENT_HPP_
