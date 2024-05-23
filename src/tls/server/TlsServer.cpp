#include "TlsServer.h"

using namespace tcp_serverclient;
using namespace std;

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
