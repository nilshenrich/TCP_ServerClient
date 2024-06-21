/**
 * @file TcpServer.hpp
 * @author Nils Henrich
 * @brief TCP server for unencrypted data transfer without authentication.
 * @version 2.0
 * @date 2021-12-27
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef TCPSERVER_HPP_
#define TCPSERVER_HPP_

#include <limits>

#include "template/Server.hpp"

namespace tcp
{
   class TcpServer : public Server<int>
   {
   public:
      /**
       * @brief Constructor for continuous stream forwarding
       */
      TcpServer() : Server{} {}

      /**
       * @brief Constructor for fragmented messages
       *
       * @param delimiter     Character to split messages on
       * @param messageMaxLen Maximum message length
       */
      TcpServer(char delimiter, size_t messageMaxLen = ::std::numeric_limits<size_t>::max() - 1) : Server{delimiter, messageMaxLen} {}

      /**
       * @brief Destructor
       */
      virtual ~TcpServer() { stop(); }

   private:
      /**
       * @brief Initialize the server (Do nothing. Just return 0).
       *
       * @return int
       */
      int init(const char *const = nullptr,
               const char *const = nullptr,
               const char *const = nullptr) override final { return SERVER_START_OK; }

      /**
       * @brief Initialize connection to a specific client (Identified by its TCP ID) (Do nothing. Just return pointer to TCP ID).
       *
       * @param clientId
       * @return int*
       */
      int *connectionInit(const int clientId) override final { return new int{clientId}; }

      /**
       * @brief Deinitialize connection to a specific client (Identified by its TCP ID) (Do nothing.).
       *
       * @param socket
       */
      void connectionDeinit(int *socket) override final {}

      /**
       * @brief Read data from a specific client (Identified by its TCP ID).
       * This method blocks until data is available.
       * If no data is available, it returns an empty string.
       *
       * @param socket
       * @return string
       */
      ::std::string readMsg(int *socket) override final
      {
         // Buffer for received data.
         char buffer[MAXIMUM_RECEIVE_PACKAGE_SIZE]{0};

         // Wait for data to be available.
         ssize_t lenMsg{recv(*socket, buffer, MAXIMUM_RECEIVE_PACKAGE_SIZE, 0)};

         // Return received data as string (or empty string if no data is available).
         return ::std::string{buffer, 0 < lenMsg ? static_cast<size_t>(lenMsg) : 0UL};
      }

      /**
       * @brief Send raw data to a specific client (Identified by its TCP ID).
       * Send message over unencrypted TCP connection.
       *
       * @param clientId
       * @param msg
       * @return true
       * @return false
       */
      bool writeMsg(const int clientId, const ::std::string &msg) override final
      {
#ifdef DEVELOP
         ::std::cout << typeid(this).name() << "::" << __func__ << ": Send to client " << clientId << ": " << msg << endl;
#endif // DEVELOP

         const size_t lenMsg{msg.size()};
         return send(clientId, msg.c_str(), lenMsg, 0) == (ssize_t)lenMsg;
      }

      // Disallow copy
      TcpServer(const TcpServer &) = delete;
      TcpServer &operator=(const TcpServer &) = delete;
   };
}

#endif // TCPSERVER_HPP_
