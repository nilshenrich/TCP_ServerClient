/**
 * @file TlsClient.hpp
 * @author Nils Henrich
 * @brief TLS client for encrypted data transfer with authentication.
 * @version 3.2.0
 * @date 2021-12-27
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef TLSCLIENT_HPP_
#define TLSCLIENT_HPP_

#include <limits>
#include <openssl/ssl.h>

#ifdef DEVELOP
#include <openssl/err.h>
#endif // DEVELOP

#include "template/Client.hpp"

namespace tcp
{
    /**
     * @brief Deleter for TLS object
     */
    struct Client_SSL_Deleter
    {
        void operator()(SSL *ssl)
        {
            SSL_free(ssl);
            return;
        }
    };

    /**
     * @brief Class for encrypted TLS client
     */
    class TlsClient : public Client<SSL, Client_SSL_Deleter>
    {
    public:
        /**
         * @brief Constructor for continuous stream forwarding
         *
         * @param os    Stream to forward incoming stream to
         */
        TlsClient(::std::ostream &os = ::std::cout) : Client(os) {}

        /**
         * @brief Constructor for fragmented messages
         *        Default authentication: No self certificates but expect server authentication -> Foreign-authentication
         *
         * @param delimiter     Character to split messages on
         * @param messageAppend String to append to the end of each fragmented message (before the delimiter)
         * @param messageMaxLen Maximum message length (actual message + length of append string) (default is 2³² - 2 = 4294967294)
         */
        TlsClient(char delimiter, const ::std::string &messageAppend = "", size_t messageMaxLen = ::std::numeric_limits<size_t>::max() - 1) : Client(delimiter, messageAppend, messageMaxLen),
                                                                                                                                              CERTIFICATEPATH_CA{},
                                                                                                                                              CERTIFICATEPATH_CERT{},
                                                                                                                                              CERTIFICATEPATH_KEY{},
                                                                                                                                              SERVER_AUTHENTICATION{true} {}

        /**
         * @brief Destructor
         */
        virtual ~TlsClient() { stop(); }

        /**
         * @brief Sets the file paths for the CA certificate, server certificate, and private key.
         *
         * @param pathToCaCert The file path to the CA certificate. Default is an empty string (No CA certificate).
         * @param pathToCert The file path to the server certificate. Default is an empty string (No server certificate).
         * @param pathToPrivKey The file path to the private key. Default is an empty string (No private key).
         */
        void setCertificates(const ::std::string &pathToCaCert = "", const ::std::string &pathToCert = "", const ::std::string &pathToPrivKey = "")
        {
            CERTIFICATEPATH_CA = pathToCaCert;
            CERTIFICATEPATH_CERT = pathToCert;
            CERTIFICATEPATH_KEY = pathToPrivKey;
            return;
        }

        /**
         * @brief Clears the paths of the CA certificate, server certificate, and private key.
         *
         * This function resets the paths of the CA certificate, server certificate, and private key
         * by clearing the strings that store these paths.
         */
        void clearCertificates()
        {
            CERTIFICATEPATH_CA.clear();
            CERTIFICATEPATH_CERT.clear();
            CERTIFICATEPATH_KEY.clear();
            return;
        }

        /**
         * @brief Sets the requirement for server authentication.
         *
         * This function enables or disables the requirement for server authentication
         * during the TLS handshake process. By default, server authentication is required.
         *
         * @param serverAuth A boolean value indicating whether server authentication is required.
         *                   If set to true, server authentication is required. If set to false,
         *                   server authentication is not required. Default is true.
         */
        void requireServerAuthentication(const bool serverAuth = true)
        {
            SERVER_AUTHENTICATION = serverAuth;
            return;
        }

    private:
        /**
         * @brief Initialize the client
         * Load certificates and keys
         *
         * @param pathToCaCert
         * @param pathToCert
         * @param pathToPrivKey
         * @return int
         */
        int init() override final
        {
            // Initialize OpenSSL algorithms
            OpenSSL_add_ssl_algorithms();

#ifdef DEVELOP
            SSL_load_error_strings();
#endif // DEVELOP

            // Set encryption method to latest client side TLS version (Stop client and return with error if failed)
            clientContext.reset(SSL_CTX_new(TLS_client_method()));
            if (!clientContext.get())
            {
#ifdef DEVELOP
                ::std::cerr << DEBUGINFO << ": Error when setting encryption method to latest client side TLS version" << ::std::endl;
#endif // DEVELOP

                stop();
                return CLIENT_ERROR_START_SET_CONTEXT;
            }

            // Get pointer to certificate paths for C-style usage
            const char *const pathToCaCert_p{CERTIFICATEPATH_CA.c_str()};
            const char *const pathToCert_p{CERTIFICATEPATH_CERT.c_str()};
            const char *const pathToPrivKey_p{CERTIFICATEPATH_KEY.c_str()};
            bool validCa{!CERTIFICATEPATH_CA.empty()};
            bool validCert{!(CERTIFICATEPATH_CERT.empty() || CERTIFICATEPATH_KEY.empty())};

            // Valid CA certificate: Load the CA certificate the client should trust. Mandatory for verifying server authentication
            if (validCa)
            {
                // Check if CA certificate file exists
                if (access(pathToCaCert_p, F_OK))
                {
#ifdef DEVELOP
                    ::std::cerr << DEBUGINFO << ": CA certificate file does not exist" << ::std::endl;
#endif // DEVELOP

                    stop();
                    return CLIENT_ERROR_START_WRONG_CA_PATH;
                }

                // Load the CA certificate the client should trust (Stop client and return with error if failed)
                if (1 != SSL_CTX_load_verify_locations(clientContext.get(), pathToCaCert_p, nullptr))
                {
#ifdef DEVELOP
                    ::std::cerr << DEBUGINFO << ": Error when loading the CA certificate the client should trust: " << pathToCaCert_p << ::std::endl;
#endif // DEVELOP

                    stop();
                    return CLIENT_ERROR_START_WRONG_CA;
                }
            }

            // Valid client certificate and private key: Load the client certificate and private key to authenticate the client
            if (validCert)
            {
                // Check if certificate file exists
                if (access(pathToCert_p, F_OK))
                {
#ifdef DEVELOP
                    ::std::cerr << DEBUGINFO << ": Client certificate file does not exist" << ::std::endl;
#endif // DEVELOP

                    stop();
                    return CLIENT_ERROR_START_WRONG_CERT_PATH;
                }

                // Check if private key file exists
                if (access(pathToPrivKey_p, F_OK))
                {
#ifdef DEVELOP
                    ::std::cerr << DEBUGINFO << ": Client private key file does not exist" << ::std::endl;
#endif // DEVELOP

                    stop();
                    return CLIENT_ERROR_START_WRONG_KEY_PATH;
                }

                // Load the client certificate (Stop client and return with error if failed)
                if (1 != SSL_CTX_use_certificate_file(clientContext.get(), pathToCert_p, SSL_FILETYPE_PEM))
                {
#ifdef DEVELOP
                    ::std::cerr << DEBUGINFO << ": Error when loading the client certificate: " << pathToCert_p << ::std::endl;
#endif // DEVELOP

                    stop();
                    return CLIENT_ERROR_START_WRONG_CERT;
                }

                // Load the client private key (Stop client and return with error if failed)
                if (1 != SSL_CTX_use_PrivateKey_file(clientContext.get(), pathToPrivKey_p, SSL_FILETYPE_PEM))
                {
#ifdef DEVELOP
                    ::std::cerr << DEBUGINFO << ": Error when loading the client private key: " << pathToPrivKey_p << ::std::endl;
#endif // DEVELOP

                    stop();
                    return CLIENT_ERROR_START_WRONG_KEY;
                }
            }

            // Set TLS mode: SSL_MODE_AUTO_RETRY
            SSL_CTX_set_mode(clientContext.get(), SSL_MODE_AUTO_RETRY);

            // Force server authentication if defined
            // SSL_VERIFY_NONE set automatically otherwise
            if (SERVER_AUTHENTICATION && validCa)
            {
                SSL_CTX_set_verify(clientContext.get(), SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
                SSL_CTX_set_verify_depth(clientContext.get(), 1); // Server certificate must be issued directly by a trusted CA
            }

            return CLIENT_START_OK;
        }

        /**
         * @brief Initialize the connection
         * Do handshake with the server and return pointer to the TLS context
         *
         * @return SSL*
         */
        SSL *connectionInit() override final
        {
            // Clarification:
            // No need to shutdown/close/free socket as this is already done in stop()
            // If connection initialization fails, client is stoped automatically

            // Set allowed TLS cipher suites (Only TLSv1.3)
            if (!SSL_CTX_set_ciphersuites(clientContext.get(), "TLS_AES_256_GCM_SHA384"))
            {
#ifdef DEVELOP
                ::std::cerr << DEBUGINFO << ": Error when setting cipher suites" << ::std::endl;
#endif // DEVELOP

                return nullptr;
            }

            // Create new TLS channel (Return nullptr if failed)
            SSL *tlsSocket{SSL_new(clientContext.get())};
            if (!tlsSocket)
            {
#ifdef DEVELOP
                ::std::cerr << DEBUGINFO << ": Error when creating new TLS channel" << ::std::endl;
#endif // DEVELOP

                return nullptr;
            }

            // Bind the TLS channel to the TCP socket (Return nullptr if failed)
            if (!SSL_set_fd(tlsSocket, tcpSocket))
            {
#ifdef DEVELOP
                ::std::cerr << DEBUGINFO << ": Error when binding the TLS channel to the TCP socket" << ::std::endl;
#endif // DEVELOP

                SSL_free(tlsSocket);

                return nullptr;
            }

            // Do TLS handshake (Return nullptr if failed)
            if (1 != SSL_connect(tlsSocket))
            {
#ifdef DEVELOP
                ::std::cerr << DEBUGINFO << ": Error when doing TLS handshake" << ::std::endl;

                unsigned long err;
                while ((err = ERR_get_error()))
                {
                    char *err_msg = ERR_error_string(err, NULL);
                    ::std::cerr << DEBUGINFO << ": SSL error (" << err << "): " << err_msg << ::std::endl;
                }
#endif // DEVELOP

                SSL_free(tlsSocket);

                return nullptr;
            }

#ifdef DEVELOP
            ::std::cout << DEBUGINFO << ": Encrypted connection to server established" << ::std::endl;
#endif // DEVELOP

            return tlsSocket;
        }

        /**
         * @brief Deinitialize the connection (Shutdown the TLS connection)
         */
        void connectionDeinit() override final
        {
            // Shutdown the TLS channel. Memory will be freed automatically on deletion
            if (clientSocket.get())
                SSL_shutdown(clientSocket.get());

            return;
        }

        /**
         * @brief Read raw data from the encrypted TLS socket
         *
         * @return string
         */
        ::std::string readMsg() override final
        {
            // Buffer for incoming message
            char buffer[MAXIMUM_RECEIVE_PACKAGE_SIZE]{0};

            // Wait for server to send message
            const int lenMsg{SSL_read(clientSocket.get(), buffer, MAXIMUM_RECEIVE_PACKAGE_SIZE)};

            // Return received message as string (Return empty string if receive failed)
            return ::std::string{buffer, 0 < lenMsg ? static_cast<size_t>(lenMsg) : 0UL};
        }

        /**
         * @brief Write raw data to the encrypted TLS socket
         *
         * @param msg
         * @return true
         * @return false
         */
        bool writeMsg(const ::std::string &msg) override final
        {
#ifdef DEVELOP
            ::std::cout << DEBUGINFO << ": Send to server: " << msg << ::std::endl;
#endif // DEVELOP

            // Get size of message to send
            const int lenMsg{(int)msg.size()};

            // Send message to server (Return false if send failed)
            return SSL_write(clientSocket.get(), msg.c_str(), lenMsg) == lenMsg;
        }

        // TLS context
        ::std::unique_ptr<SSL_CTX, void (*)(SSL_CTX *)> clientContext{nullptr, SSL_CTX_free};

        // Certificate paths
        ::std::string CERTIFICATEPATH_CA;
        ::std::string CERTIFICATEPATH_CERT;
        ::std::string CERTIFICATEPATH_KEY;

        // Require server authentication
        bool SERVER_AUTHENTICATION;

        // Disallow copy
        TlsClient(const TlsClient &) = delete;
        TlsClient &operator=(const TlsClient &) = delete;
    };
}

#endif // TLSCLIENT_HPP_
