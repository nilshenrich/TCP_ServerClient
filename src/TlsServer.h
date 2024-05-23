/**
 * @file TlsServer.h
 * @author Nils Henrich
 * @brief TLS server for encrypted data transfer with authentication.
 * @version 2.0
 * @date 2021-12-27
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef TLSSERVER_H
#define TLSSERVER_H

#include <limits>
#include <openssl/ssl.h>

#include "../../general/server/Server.h"

namespace tcp_serverclient
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
      TlsServer(char delimiter, size_t messageMaxLen = ::std::numeric_limits<size_t>::max() - 1);

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
       * @return string
       */
      ::std::string getSubjPartFromClientCert(const int clientId, const SSL *tlsSocket, const int subjPart);

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
       * @return string
       */
      ::std::string readMsg(SSL *socket) override final;

      /**
       * @brief Send raw data to a specific client (Identified by its TCP ID).
       *
       * @param clientId
       * @param msg
       * @return true
       * @return false
       */
      bool writeMsg(const int clientId, const ::std::string &msg) override final;

      // TLS context of the server
      ::std::unique_ptr<SSL_CTX, void (*)(SSL_CTX *)> serverContext{nullptr, SSL_CTX_free};

      // Disallow copy
      TlsServer(const TlsServer &) = delete;
      TlsServer &operator=(const TlsServer &) = delete;
   };

   // ============================== Implementation of methods. ==============================

   TlsServer::TlsServer() : Server{} {}
   TlsServer::TlsServer(char delimiter, size_t messageMaxLen) : Server{delimiter, messageMaxLen} {}

   TlsServer::~TlsServer()
   {
      stop();
   }

   int TlsServer::init(const char *const pathToCaCert,
                       const char *const pathToCert,
                       const char *const pathToPrivKey)
   {
      // Initialize OpenSSL library (Needed for encryption and authentication)
      OpenSSL_add_ssl_algorithms();

      // Set encrytion method (Latest version of TLS server side)
      // Stop server and return error if it fails
      serverContext.reset(SSL_CTX_new(TLS_server_method()));
      if (!serverContext.get())
      {
#ifdef DEVELOP
         cerr << typeid(this).name() << "::" << __func__ << ": Error when setting encryption method" << endl;
#endif // DEVELOP

         stop();
         return SERVER_ERROR_START_SET_CONTEXT;
      }

      // Check if CA certificate file exists
      if (access(pathToCaCert, F_OK))
      {
#ifdef DEVELOP
         cerr << typeid(this).name() << "::" << __func__ << ": CA certificate file does not exist" << endl;
#endif // DEVELOP

         stop();
         return SERVER_ERROR_START_WRONG_CA_PATH;
      }

      // Check if certificate file exists
      if (access(pathToCert, F_OK))
      {
#ifdef DEVELOP
         cerr << typeid(this).name() << "::" << __func__ << ": Server certificate file does not exist" << endl;
#endif // DEVELOP

         stop();
         return SERVER_ERROR_START_WRONG_CERT_PATH;
      }

      // Check if private key file exists
      if (access(pathToPrivKey, F_OK))
      {
#ifdef DEVELOP
         cerr << typeid(this).name() << "::" << __func__ << ": Server private key file does not exist" << endl;
#endif // DEVELOP

         stop();
         return SERVER_ERROR_START_WRONG_KEY_PATH;
      }

      // Load CA certificate
      // Stop server and return error if it fails
      if (1 != SSL_CTX_load_verify_locations(serverContext.get(), pathToCaCert, nullptr))
      {
#ifdef DEVELOP
         cerr << typeid(this).name() << "::" << __func__ << ": Error when reading CA certificate \"" << pathToCaCert << "\"" << endl;
#endif // DEVELOP

         stop();
         return SERVER_ERROR_START_WRONG_CA;
      }

      // Set CA certificate as verification certificate to verify client certificate
      SSL_CTX_set_client_CA_list(serverContext.get(), SSL_load_client_CA_file(pathToCaCert));

      // Load server certificate
      // Stop server and return error if it fails
      if (1 != SSL_CTX_use_certificate_file(serverContext.get(), pathToCert, SSL_FILETYPE_PEM))
      {
#ifdef DEVELOP
         cerr << typeid(this).name() << "::" << __func__ << ": Error when loading server certificate \"" << pathToCert << "\"" << endl;
#endif // DEVELOP

         stop();
         return SERVER_ERROR_START_WRONG_CERT;
      }

      // Load server private key (Includes check with certificate)
      // Stop server and return error if it fails
      if (1 != SSL_CTX_use_PrivateKey_file(serverContext.get(), pathToPrivKey, SSL_FILETYPE_PEM))
      {
#ifdef DEVELOP
         cerr << typeid(this).name() << "::" << __func__ << ": Error when loading server private key \"" << pathToPrivKey << "\"" << endl;
#endif // DEVELOP

         stop();
         return SERVER_ERROR_START_WRONG_KEY;
      }

      // Set TLS mode (Auto retry)
      SSL_CTX_set_mode(serverContext.get(), SSL_MODE_AUTO_RETRY);

      // Force client authentication
      SSL_CTX_set_verify(serverContext.get(), SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT | SSL_VERIFY_CLIENT_ONCE, NULL);

      // Check client certificate (CA certificate must be direct issuer)
      SSL_CTX_set_verify_depth(serverContext.get(), 1);

      return SERVER_START_OK;
   }

   SSL *TlsServer::connectionInit(const int clientId)
   {
      // Set allowed TLS cipher suites (Only TLSv1.3)
      if (!SSL_CTX_set_ciphersuites(serverContext.get(), "TLS_AES_256_GCM_SHA384"))
      {
#ifdef DEVELOP
         cerr << typeid(this).name() << "::" << __func__ << ": Error when setting TLS cipher suites" << endl;
#endif // DEVELOP

         shutdown(clientId, SHUT_RDWR);
         close(clientId);

         return nullptr;
      }

      // Create new TLS channel
      // Close connection and return nullptr if it fails
      SSL *tlsSocket{SSL_new(serverContext.get())};
      if (!tlsSocket)
      {
#ifdef DEVELOP
         cerr << typeid(this).name() << "::" << __func__ << ": Error when creating TLS channel" << endl;
#endif // DEVELOP

         shutdown(clientId, SHUT_RDWR);
         close(clientId);
         SSL_free(tlsSocket);

         return nullptr;
      }

      // Assign clients TCP socket to TLS channel
      // Close connection and return nullptr if it fails
      if (!SSL_set_fd(tlsSocket, clientId))
      {
#ifdef DEVELOP
         cerr << typeid(this).name() << "::" << __func__ << ": Error when assigning clients TCP socket to TLS channel" << endl;
#endif // DEVELOP

         SSL_shutdown(tlsSocket);
         shutdown(clientId, SHUT_RDWR);
         close(clientId);
         SSL_free(tlsSocket);

         return nullptr;
      }

      // Do TLS handshake
      // Close connection and return nullptr if it fails
      if (1 != SSL_accept(tlsSocket))
      {
#ifdef DEVELOP
         cerr << typeid(this).name() << "::" << __func__ << ": Error when doing TLS handshake" << endl;
#endif // DEVELOP

         SSL_shutdown(tlsSocket);
         shutdown(clientId, SHUT_RDWR);
         close(clientId);
         SSL_free(tlsSocket);

         return nullptr;
      }

#ifdef DEVELOP
      cout << typeid(this).name() << "::" << __func__ << ": New connection established to client: " << clientId << endl;
#endif // DEVELOP

      return tlsSocket;
   }

   bool TlsServer::writeMsg(const int clientId, const string &msg)
   {
      // Convert string to char array (Do here for peromance reasons)
      const char *const buffer = msg.c_str();

      // Get length of message to send
      const int lenMsg{(int)msg.size()};

#ifdef DEVELOP
      cout << typeid(this).name() << "::" << __func__ << ": Send to client " << clientId << ": " << msg << endl;
#endif // DEVELOP

      // Get TLS channel for client to send message to
      SSL *socket{activeConnections[clientId].get()};

      // Send message to client
      // Return false if it fails
      return SSL_write(socket, buffer, lenMsg) == lenMsg;
   }

   string TlsServer::readMsg(SSL *socket)
   {
      // Buffer for incoming message
      char buffer[MAXIMUM_RECEIVE_PACKAGE_SIZE]{0};

      // Wait for message from client
      const int lenMsg{SSL_read(socket, buffer, MAXIMUM_RECEIVE_PACKAGE_SIZE)};

      // Return message as string if it was received successfully (Return empty string if it fails)
      return string{buffer, 0 < lenMsg ? static_cast<size_t>(lenMsg) : 0UL};
   }

   void TlsServer::connectionDeinit(SSL *socket)
   {
      // Shutdown TLS channel. Memory will be freed automatically on deletion
      SSL_shutdown(socket);
      return;
   }

   string TlsServer::getSubjPartFromClientCert(const int clientId, const SSL *tlsSocket, const int subjPart)
   {
      char buf[256]{0};

      // If TLS socket is null, get socket from list of connected clients
      if (!tlsSocket)
      {
         lock_guard<mutex> lck{activeConnections_m};
         if (activeConnections.find(clientId) == activeConnections.end())
         {
#ifdef DEVELOP
            cerr << typeid(this).name() << "::" << __func__ << ": No connected client " << clientId << endl;
#endif // DEVELOP

            return "";
         }

         tlsSocket = activeConnections[clientId].get();
      }

      // Read client certificate from TLS channel
      unique_ptr<X509, void (*)(X509 *)> remoteCert{SSL_get_peer_certificate(tlsSocket), X509_free};

      // Get wholw subject part from client certificate
      X509_NAME *remoteCertSubject{X509_get_subject_name(remoteCert.get())};

      // Get specific part from subject
      X509_NAME_get_text_by_NID(remoteCertSubject, subjPart, buf, 256);

      // Return subject part as string
      return string(buf);
   }
}

#endif // TLSSERVER_H
