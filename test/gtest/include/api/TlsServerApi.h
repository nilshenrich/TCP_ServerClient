#ifndef TLS_SERVER_API_H_
#define TLS_SERVER_API_H_

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <mutex>

#include "TlsServer.h"
#include "TestDefines.h"

namespace TestApi
{
    class TlsServerApi_fragmentation
    {
    public:
        TlsServerApi_fragmentation(size_t messageMaxLen = TestConstants::MAXLEN_MSG_B);
        virtual ~TlsServerApi_fragmentation();

        /**
         * @brief Start TLS server
         *
         * @param port TLS port to listen on
         * @return int NETWORKLISTENER_START_OK if successful, other if failed
         */
        int start(const int port, const std::string pathToCaCert = KeyPaths::CaCert, const std::string pathToListenerCert = KeyPaths::ListenerCert, const std::string pathToListenerKey = KeyPaths::ListenerKey);

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
        bool sendMsg(const int tlsClientId, const std::string &tlsMsg);

        /**
         * @brief Get buffered message from TLS clients and clear buffer
         *
         * @return std::vector<MessageFromClient> Vector of buffered messages
         */
        std::vector<MessageFromClient> getBufferedMsg();

        /**
         * @brief Get IDs of all connected clients
         *
         * @return std::vector<int> Vector of client IDs
         */
        std::vector<int> getClientIds();

    private:
        /**
         * @brief Buffer incoming messages
         *
         * @param tlsClientId       Client ID
         * @param tlsMsgFromClient  Message from client
         */
        void workOnMessage(const int tlsClientId, const std::string tlsMsgFromClient);

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
        networking::TlsServer tlsServer;

        // Buffered messages
        std::vector<MessageFromClient> bufferedMsg;
        std::mutex bufferedMsg_m;
    };

    class TlsServerApi_forwarding
    {
    public:
        TlsServerApi_forwarding();
        virtual ~TlsServerApi_forwarding();

        /**
         * @brief Start TLS server
         *
         * @param port TLS port to listen on
         * @return int NETWORKLISTENER_START_OK if successful, other if failed
         */
        int start(const int port, const std::string pathToCaCert = KeyPaths::CaCert, const std::string pathToListenerCert = KeyPaths::ListenerCert, const std::string pathToListenerKey = KeyPaths::ListenerKey);

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
        bool sendMsg(const int tlsClientId, const std::string &tlsMsg);

        /**
         * @brief Get buffered message from TLS clients and clear buffer
         *
         * @return std::map<int, std::string> Vector of buffered messages
         */
        std::map<int, std::string> getBufferedMsg();

        /**
         * @brief Get IDs of all connected clients
         *
         * @return std::vector<int> Vector of client IDs
         */
        std::vector<int> getClientIds();

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
        networking::TlsServer tlsServer;

        /**
         * @brief Generate an output stream to a string for each client
         *
         * @param clientId
         * @return std::ostringstream*
         */
        std::ostringstream *generateForwardingStream(int clientId);

        // Buffered messages
        std::map<int, std::ostringstream *> bufferedMsg;
    };

} // namespace TestApi

#endif // TLS_SERVER_API_H_
