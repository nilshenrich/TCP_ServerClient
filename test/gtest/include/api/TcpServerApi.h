#ifndef TCP_SERVER_API_H_
#define TCP_SERVER_API_H_

#include <string>

#include "TcpServer.hpp"
#include "TestDefines.h"

namespace TestApi
{
    class TcpServerApi_fragmentation
    {
    public:
        TcpServerApi_fragmentation();
        TcpServerApi_fragmentation(const ::std::string &messageAppend);
        TcpServerApi_fragmentation(size_t messageMaxLen);
        TcpServerApi_fragmentation(const ::std::string &messageAppend, size_t messageMaxLen);
        virtual ~TcpServerApi_fragmentation();

        /**
         * @brief Start TCP server
         *
         * @param port TCP port to listen on
         * @return int SERVER_START_OK if successful, other if failed
         */
        int start(const int port);

        /**
         * @brief Stop TCP server
         */
        void stop();

        /**
         * @brief Send message to TCP client
         *
         * @param tcpClientId TCP client ID
         * @param tcpMsg Message to send
         * @return bool true if successful, false if failed
         */
        bool sendMsg(const int tcpClientId, const ::std::string &tcpMsg);

        /**
         * @brief Get buffered message from TCP clients and clear buffer
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

    private:
        /**
         * @brief Buffer incoming messages
         *
         * @param tcpClientId       Client ID
         * @param tcpMsgFromClient  Message from client
         */
        void workOnMessage(const int tcpClientId, const ::std::string tcpMsgFromClient);

        /**
         * @brief Do nothing on established connection
         *
         * @param tcpClientId
         */
        void workOnEstablished(const int tcpClientId);

        /**
         * @brief Do nothing on closed connection
         *
         * @param tcpClientId ID des Clients
         */
        void workOnClosed(const int tcpClientId);

        // TCP server
        ::tcp::TcpServer tcpServer;

        // Buffered messages
        ::std::vector<MessageFromClient> bufferedMsg;
        ::std::mutex bufferedMsg_m;
    };

    class TcpServerApi_continuous
    {
    public:
        TcpServerApi_continuous();
        virtual ~TcpServerApi_continuous();

        /**
         * @brief Start TCP server
         *
         * @param port TCP port to listen on
         * @return int SERVER_START_OK if successful, other if failed
         */
        int start(const int port);

        /**
         * @brief Stop TCP server
         */
        void stop();

        /**
         * @brief Send message to TCP client
         *
         * @param tcpClientId TCP client ID
         * @param tcpMsg Message to send
         * @return bool true if successful, false if failed
         */
        bool sendMsg(const int tcpClientId, const ::std::string &tcpMsg);

        /**
         * @brief Get buffered message from TCP clients and clear buffer
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
        void workOnClosed(const int tcpClientId);

        // TCP server
        ::tcp::TcpServer tcpServer;

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

#endif // TCP_SERVER_API_H_
