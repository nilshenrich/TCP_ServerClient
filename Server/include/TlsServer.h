/**
 * @file TlsServer.h
 * @author Nils Henrich
 * @brief TLS server for encrypted data transfer with authentication.
 * @version 1.0
 * @date 2021-12-27
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef TLSSERVER_H
#define TLSSERVER_H

#include <limits>
#include <openssl/ssl.h>

#include "Server.h"

namespace tcp
{
   // Deleter for SSL objects
   struct Server_SSL_Deleter
   {
      void operator()(SSL *ssl)
      {
         SSL_free(ssl);
         return;
      }
   };

   class TlsServer : public Server<SSL, Server_SSL_Deleter>
   {
   public:
      /**
       * @brief Constructor for continuous stream forwarding
       */
      TlsServer();

      /**
       * @brief Constructor for fragmented messages
       *
       * @param delimiter     Character to split messages on
       * @param messageMaxLen Maximum message length
       */
      TlsServer(char delimiter, size_t messageMaxLen = std::numeric_limits<size_t>::max() - 1);

      /**
       * @brief Destructor
       */
      virtual ~TlsServer();

      /**
       * @brief Get specific subject part as string of the certificate of a specific connected client (Identified by its TCP ID).
       *
       * @param clientId
       * @param tlsSocket
       * @param subjPart
       * @return std::string
       */
      std::string getSubjPartFromClientCert(const int clientId, const SSL *tlsSocket, const int subjPart);

   private:
      /**
       * @brief Initialize the server (Setup enryyption settings).
       * Setting: Encryption via TLS, force client authentication, authenticate self with certificate.
       * Checking certificate validity with CA certificate.
       *
       * @param pathToCaCert
       * @param pathToCert
       * @param pathToPrivKey
       * @return int
       */
      int init(const char *const pathToCaCert,
               const char *const pathToCert,
               const char *const pathToPrivKey) override final;

      /**
       * @brief Initialize connection to a specific client (Identified by its TCP ID) (Do TLS handshake).
       *
       * @param clientId
       * @return SSL*
       */
      SSL *connectionInit(const int clientId) override final;

      /**
       * @brief Deinitialize connection to a specific client (Identified by its TCP ID) (Do TLS shutdown).
       *
       * @param socket
       */
      void connectionDeinit(SSL *socket) override final;

      /**
       * @brief Read data from a specific client (Identified by its TCP ID).
       * This method blocks until data is available.
       * If no data is available, it returns an empty string.
       *
       * @param socket
       * @return std::string
       */
      std::string readMsg(SSL *socket) override final;

      /**
       * @brief Send raw data to a specific client (Identified by its TCP ID).
       *
       * @param clientId
       * @param msg
       * @return true
       * @return false
       */
      bool writeMsg(const int clientId, const std::string &msg) override final;

      // TLS context of the server
      std::unique_ptr<SSL_CTX, void (*)(SSL_CTX *)> serverContext{nullptr, SSL_CTX_free};

      // Disallow copy
      TlsServer(const TlsServer &) = delete;
      TlsServer &operator=(const TlsServer &) = delete;
   };
}

#endif // TLSSERVER_H
