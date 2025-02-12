#ifndef TCP_CLIENT_API_H_
#define TCP_CLIENT_API_H_

#include <string>

#include "TcpClient.hpp"
#include "TestDefines.h"

namespace TestApi
{
    class TcpClientApi_fragmentation
    {
    public:
        TcpClientApi_fragmentation();
        TcpClientApi_fragmentation(const ::std::string &messageAppend);
        TcpClientApi_fragmentation(size_t messageMaxLen);
        TcpClientApi_fragmentation(const ::std::string &messageAppend, size_t messageMaxLen);
        virtual ~TcpClientApi_fragmentation();

        /**
         * @brief Connect to TCP server
         *
         * @param ip IP address of TCP server
         * @param port TCP port of TCP server
         * @return int CLIENT_CONNECT_OK if successful, other if failed
         */
        int start(const ::std::string &ip, const int port);

        /**
         * @brief Disconnect from TCP server
         */
        void stop();

        /**
         * @brief Send message to TCP server
         *
         * @param tcpMsg Message to send
         * @return bool true if successful, false if failed
         */
        bool sendMsg(const ::std::string &tcpMsg);

        /**
         * @brief Get buffered message from TCP server and clear buffer
         *
         * @return vector<string> Vector of buffered messages
         */
        ::std::vector<::std::string> getBufferedMsg();

    private:
        /**
         * @brief Buffer incoming messages
         *
         * @param tcpMsgFromServer Message from server
         */
        void workOnMessage(const ::std::string tcpMsgFromServer);

        // TCP client
        ::tcp::TcpClient tcpClient;

        // Buffered messages
        ::std::vector<::std::string> bufferedMsg;
        ::std::mutex bufferedMsg_m;
    };

    class TcpClientApi_continuous
    {
    public:
        TcpClientApi_continuous();
        virtual ~TcpClientApi_continuous();

        /**
         * @brief Connect to TCP server
         *
         * @param ip IP address of TCP server
         * @param port TCP port of TCP server
         * @return int CLIENT_CONNECT_OK if successful, other if failed
         */
        int start(const ::std::string &ip, const int port);

        /**
         * @brief Disconnect from TCP server
         */
        void stop();

        /**
         * @brief Send message to TCP server
         *
         * @param tcpMsg Message to send
         * @return bool true if successful, false if failed
         */
        bool sendMsg(const ::std::string &tcpMsg);

        /**
         * @brief Get buffered message from TCP server and clear buffer
         *
         * @return string Vector of buffered messages
         */
        ::std::string getBufferedMsg();

    private:
        // Buffered message
        ::std::ostringstream bufferedMsg_os{::std::ios_base::ate};

        // TCP client
        ::tcp::TcpClient tcpClient;
    };

} // namespace TestApi

#endif // TCP_CLIENT_API_H_
