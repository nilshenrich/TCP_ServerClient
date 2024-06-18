#ifndef TLS_CLIENT_API_H_
#define TLS_CLIENT_API_H_

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <mutex>

#include "TlsClient.hpp"
#include "TestDefines.h"

namespace TestApi
{
    class TlsClientApi_fragmentation
    {
    public:
        TlsClientApi_fragmentation(size_t messageMaxLen = TestConstants::MAXLEN_MSG_B);
        virtual ~TlsClientApi_fragmentation();

        /**
         * @brief Connect to TLS server
         *
         * @param ip IP address of TLS server
         * @param port TLS port of TLS server
         * @return int TLSCLIENT_CONNECT_OK if successful, other if failed
         */
        int start(const ::std::string &ip, const int port, const ::std::string pathToCaCert = KeyPaths::CaCert, const ::std::string pathToClientCert = KeyPaths::ClientCert, const ::std::string pathToClientKey = KeyPaths::ClientKey);

        /**
         * @brief Disconnect from TLS server
         */
        void stop();

        /**
         * @brief Send message to TLS server
         *
         * @param tcpMsg Message to send
         * @return bool true if successful, false if failed
         */
        bool sendMsg(const ::std::string &tcpMsg);

        /**
         * @brief Get buffered message from TLS server and clear buffer
         *
         * @return vector<string> Vector of buffered messages
         */
        ::std::vector<::std::string> getBufferedMsg();

    private:
        /**
         * @brief Buffer incoming messages
         * @param tlsMsgFromClient Message from server
         */
        void workOnMessage(const ::std::string tlsMsgFromServer);

        // TCP client
        ::tcp_serverclient::TlsClient tlsClient;

        // Buffered messages
        ::std::vector<::std::string> bufferedMsg;
        ::std::mutex bufferedMsg_m;
    };

    class TlsClientApi_continuous
    {
    public:
        TlsClientApi_continuous();
        virtual ~TlsClientApi_continuous();

        /**
         * @brief Connect to TLS server
         *
         * @param ip IP address of TLS server
         * @param port TLS port of TLS server
         * @return int TLSCLIENT_CONNECT_OK if successful, other if failed
         */
        int start(const ::std::string &ip, const int port, const ::std::string pathToCaCert = KeyPaths::CaCert, const ::std::string pathToClientCert = KeyPaths::ClientCert, const ::std::string pathToClientKey = KeyPaths::ClientKey);

        /**
         * @brief Disconnect from TLS server
         */
        void stop();

        /**
         * @brief Send message to TLS server
         *
         * @param tcpMsg Message to send
         * @return bool true if successful, false if failed
         */
        bool sendMsg(const ::std::string &tcpMsg);

        /**
         * @brief Get buffered message from TLS server and clear buffer
         *
         * @return vector<string> Vector of buffered messages
         */
        ::std::string getBufferedMsg();

    private:
        // TCP client
        ::tcp_serverclient::TlsClient tlsClient;

        // Buffered messages
        ::std::ostringstream bufferedMsg_os{::std::ios_base::ate};
    };

} // namespace TestApi

#endif // TLS_CLIENT_API_H_
