/**
 * @file TcpServer.h
 * @author Nils Henrich
 * @brief TCP server for unencrypted data transfer without authentication.
 * @version 2.0
 * @date 2021-12-27
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <limits>

#include "Server.h"

namespace tcp
{
   class TcpServer : public Server<int>
   {
   public:
      /**
       * @brief Constructor for continuous stream forwarding
       */
      TcpServer();

      /**
       * @brief Constructor for fragmented messages
       *
       * @param delimiter     Character to split messages on
       * @param messageMaxLen Maximum message length
       */
      TcpServer(char delimiter, size_t messageMaxLen = ::std::numeric_limits<size_t>::max() - 1);

      /**
       * @brief Destructor
       */
      virtual ~TcpServer();

   private:
      /**
       * @brief Initialize the server (Do nothing. Just return 0).
       *
       * @return int
       */
      int init(const char *const = nullptr,
               const char *const = nullptr,
               const char *const = nullptr) override final;

      /**
       * @brief Initialize connection to a specific client (Identified by its TCP ID) (Do nothing. Just return pointer to TCP ID).
       *
       * @param clientId
       * @return int*
       */
      int *connectionInit(const int clientId) override final;

      /**
       * @brief Deinitialize connection to a specific client (Identified by its TCP ID) (Do nothing.).
       *
       * @param socket
       */
      void connectionDeinit(int *socket) override final;

      /**
       * @brief Read data from a specific client (Identified by its TCP ID).
       * This method blocks until data is available.
       * If no data is available, it returns an empty string.
       *
       * @param socket
       * @return string
       */
      ::std::string readMsg(int *socket) override final;

      /**
       * @brief Send raw data to a specific client (Identified by its TCP ID).
       * Send message over unencrypted TCP connection.
       *
       * @param clientId
       * @param msg
       * @return true
       * @return false
       */
      bool writeMsg(const int clientId, const ::std::string &msg) override final;

      // Disallow copy
      TcpServer(const TcpServer &) = delete;
      TcpServer &operator=(const TcpServer &) = delete;
   };
}

#endif // TCPSERVER_H
