#ifndef TLS_CLIENT_API_H_
#define TLS_CLIENT_API_H_

#include <string>

#include "TlsClient.hpp"
#include "TestDefines.h"

namespace TestApi
{
    class TlsClientApi_fragmentation
    {
    public:
        TlsClientApi_fragmentation();
        TlsClientApi_fragmentation(const ::std::string &messageAppend);
        TlsClientApi_fragmentation(size_t messageMaxLen);
        TlsClientApi_fragmentation(const ::std::string &messageAppend, size_t messageMaxLen);
        virtual ~TlsClientApi_fragmentation();

        /**
         * @brief Connect to TLS server
         *
         * @param ip IP address of TLS server
         * @param port TLS port of TLS server
         * @param pathToCaCert Path to CA certificate. Default is proper self-signed test certificate
         * @param pathToServerCert Path to server certificate. Default is proper self-signed test certificate
         * @param pathToServerKey Path to server key. Default is proper self-signed test certificate
         * @param serverAuth Flag if server authentication is required. Default is true
         * @return int TLSCLIENT_CONNECT_OK if successful, other if failed
         */
        int start(const ::std::string &ip, const int port, const ::std::string pathToCaCert = KeyPaths::CaCert, const ::std::string pathToClientCert = KeyPaths::ClientCert, const ::std::string pathToClientKey = KeyPaths::ClientKey, const bool serverAuth = true);

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

        /**
         * @brief Get specific subject part as string of the certificate of the connected server
         *
         * @param subjPart Subject part to get
         * @return string
         */
        ::std::string getSubjPartFromServerCert(const int subjPart);

    private:
        /**
         * @brief Buffer incoming messages
         * @param tlsMsgFromClient Message from server
         */
        void workOnMessage(const ::std::string tlsMsgFromServer);

        // TCP client
        ::tcp::TlsClient tlsClient;

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
         * @param pathToCaCert Path to CA certificate. Default is proper self-signed test certificate
         * @param pathToServerCert Path to server certificate. Default is proper self-signed test certificate
         * @param pathToServerKey Path to server key. Default is proper self-signed test certificate
         * @param serverAuth Flag if server authentication is required. Default is true
         * @return int TLSCLIENT_CONNECT_OK if successful, other if failed
         */
        int start(const ::std::string &ip, const int port, const ::std::string pathToCaCert = KeyPaths::CaCert, const ::std::string pathToClientCert = KeyPaths::ClientCert, const ::std::string pathToClientKey = KeyPaths::ClientKey, const bool serverAuth = true);

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

        /**
         * @brief Get specific subject part as string of the certificate of the connected server
         *
         * @param subjPart Subject part to get
         * @return string
         */
        ::std::string getSubjPartFromServerCert(const int subjPart);

    private:
        // Buffered messages
        ::std::ostringstream bufferedMsg_os{::std::ios_base::ate};

        // TCP client
        ::tcp::TlsClient tlsClient;
    };

} // namespace TestApi

#endif // TLS_CLIENT_API_H_
