#ifndef TLS_SERVER_API_H_
#define TLS_SERVER_API_H_

#include <string>

#include "TlsServer.hpp"
#include "TestDefines.h"

namespace TestApi
{
    class TlsServerApi_fragmentation
    {
    public:
        TlsServerApi_fragmentation();
        TlsServerApi_fragmentation(const ::std::string &messageAppend);
        TlsServerApi_fragmentation(size_t messageMaxLen);
        TlsServerApi_fragmentation(const ::std::string &messageAppend, size_t messageMaxLen);
        virtual ~TlsServerApi_fragmentation();

        /**
         * @brief Start TLS server
         *
         * @param port TLS port to listen on
         * @param pathToCaCert Path to CA certificate. Default is proper self-signed test certificate
         * @param pathToServerCert Path to server certificate. Default is proper self-signed test certificate
         * @param pathToServerKey Path to server key. Default is proper self-signed test certificate
         * @param clientAuth Flag if client authentication is required. Default is true
         * @return int SERVER_START_OK if successful, other if failed
         */
        int start(const int port, const ::std::string pathToCaCert = KeyPaths::CaCert, const ::std::string pathToServerCert = KeyPaths::ServerCert, const ::std::string pathToServerKey = KeyPaths::ServerKey, const bool clientAuth = true);

        /**
         * @brief Stop TLS server
         */
        void stop();

        /**
         * @brief Send message to TLS client
         *
         * @param tlsClientId TLS client ID
         * @param tlsMsg Message to send
         * @return bool true if successful, false if failed
         */
        bool sendMsg(const int tlsClientId, const ::std::string &tlsMsg);

        /**
         * @brief Get buffered message from TLS clients and clear buffer
         *
         * @return vector<MessageFromClient> Vector of buffered messages
         */
        ::std::vector<MessageFromClient> getBufferedMsg();

        /**
         * @brief Get IDs of all connected clients
         *
         * @return vector<int> Vector of client IDs
         */
        ::std::vector<int> getClientIds();

        /**
         * @brief Get the IP of a specific connected client
         *
         * @param clientId ID of the client
         * @return string
         */
        ::std::string getClientIp(const int clientId) const;

        /**
         * @brief Get specific subject part as string of the certificate of a specific connected client
         *
         * @param clientId ID of the client
         * @param subjPart Subject part to get
         * @return string
         */
        ::std::string getSubjPartFromClientCert(const int clientId, const int subjPart);

    private:
        /**
         * @brief Buffer incoming messages
         *
         * @param tlsClientId       Client ID
         * @param tlsMsgFromClient  Message from client
         */
        void workOnMessage(const int tlsClientId, const ::std::string tlsMsgFromClient);

        /**
         * @brief Do nothing on established connection
         *
         * @param tcpClientId
         */
        void workOnEstablished(const int tcpClientId);

        /**
         * @brief Do nothing on established connection
         *
         * @param tcpClientId ID des Clients
         */
        void workOnClosed(const int tlsClientId);

        // TLS server
        ::tcp::TlsServer tlsServer;

        // Buffered messages
        ::std::vector<MessageFromClient> bufferedMsg;
        ::std::mutex bufferedMsg_m;
    };

    class TlsServerApi_continuous
    {
    public:
        TlsServerApi_continuous();
        virtual ~TlsServerApi_continuous();

        /**
         * @brief Start TLS server
         *
         * @param port TLS port to listen on
         * @param pathToCaCert Path to CA certificate. Default is proper self-signed test certificate
         * @param pathToServerCert Path to server certificate. Default is proper self-signed test certificate
         * @param pathToServerKey Path to server key. Default is proper self-signed test certificate
         * @param clientAuth Flag if client authentication is required. Default is true
         * @return int SERVER_START_OK if successful, other if failed
         */
        int start(const int port, const ::std::string pathToCaCert = KeyPaths::CaCert, const ::std::string pathToServerCert = KeyPaths::ServerCert, const ::std::string pathToServerKey = KeyPaths::ServerKey, const bool clientAuth = true);

        /**
         * @brief Stop TLS server
         */
        void stop();

        /**
         * @brief Send message to TLS client
         *
         * @param tlsClientId TLS client ID
         * @param tlsMsg Message to send
         * @return bool true if successful, false if failed
         */
        bool sendMsg(const int tlsClientId, const ::std::string &tlsMsg);

        /**
         * @brief Get buffered message from TLS clients and clear buffer
         *
         * @return map<int, string> Vector of buffered messages
         */
        ::std::map<int, ::std::string> getBufferedMsg();

        /**
         * @brief Get IDs of all connected clients
         *
         * @return vector<int> Vector of client IDs
         */
        ::std::vector<int> getClientIds();

        /**
         * @brief Get the IP of a specific connected client
         *
         * @param clientId ID of the client
         * @return string
         */
        ::std::string getClientIp(const int clientId) const;

        /**
         * @brief Get specific subject part as string of the certificate of a specific connected client
         *
         * @param clientId ID of the client
         * @param subjPart Subject part to get
         * @return string
         */
        ::std::string getSubjPartFromClientCert(const int clientId, const int subjPart);

    private:
        /**
         * @brief Do nothing on established connection
         *
         * @param tcpClientId
         */
        void workOnEstablished(const int tcpClientId);

        /**
         * @brief Do nothing on established connection
         *
         * @param tcpClientId ID des Clients
         */
        void workOnClosed(const int tlsClientId);

        // TLS server
        ::tcp::TlsServer tlsServer;

        /**
         * @brief Generate an output stream to a string for each client
         *
         * @param clientId
         * @return ostringstream*
         */
        ::std::ostringstream *generateContinuousStream(int clientId);

        // Buffered messages
        ::std::map<int, ::std::ostringstream *> bufferedMsg;
    };

} // namespace TestApi

#endif // TLS_SERVER_API_H_
