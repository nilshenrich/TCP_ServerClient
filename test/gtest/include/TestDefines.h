#ifndef TEST_DEFINES_H_
#define TEST_DEFINES_H_

#include <string>
#include <chrono>

namespace TestConstants
{
    // Wait for TCP/TLS connection
    const ::std::chrono::milliseconds WAITFOR_CONNECT_TCP{10};
    const ::std::chrono::milliseconds WAITFOR_CONNECT_TLS{50};

    // Wait for TCP/TLS connection to be closed
    const ::std::chrono::milliseconds DISCONNECTION_TIMEOUT{10};

    // Wait for TCP/TLS message transfer
    const ::std::chrono::milliseconds WAITFOR_MSG_TCP{100};
    const ::std::chrono::milliseconds WAITFOR_MSG_TLS{100};
    const ::std::chrono::milliseconds WAITFOR_MSG_LONG_TCP{500};
    const ::std::chrono::milliseconds WAITFOR_MSG_LONG_TLS{500};

    // Message max length
    const size_t MAXLEN_MSG_B{::std::numeric_limits<size_t>::max() - 1};
    const size_t MAXLEN_MSG_SHORT_B{8};

    // Multi-client mode
    const int MANYCLIENTS_NUMBER{10};
}

namespace TestApi
{
    // Message from TCP/TLS client (Client ID, Message)
    struct MessageFromClient
    {
        int id;
        ::std::string msg;

        // Overload operator==
        bool operator==(const MessageFromClient &other) const
        {
            return (id == other.id && msg == other.msg);
        }

        // Overload operator<<
        friend ::std::ostream &operator<<(::std::ostream &os, const MessageFromClient &msg)
        {
            os << "id: " << msg.id << ", msg: " << msg.msg;
            return os;
        }
    };
} // namespace TestApi

namespace KeyPaths
{
    const ::std::string rootDir{"../../keys"};
    const ::std::string CaDir{rootDir + "/ca"};
    const ::std::string CaCert{CaDir + "/ca.crt"};
    const ::std::string CaKey{CaDir + "/ca.key"};
    const ::std::string ServerDir{rootDir + "/server"};
    const ::std::string ServerCert{ServerDir + "/server.crt"};
    const ::std::string ServerKey{ServerDir + "/server.key"};
    const ::std::string ClientDir{rootDir + "/client"};
    const ::std::string ClientCert{ClientDir + "/client.crt"};
    const ::std::string ClientKey{ClientDir + "/client.key"};
} // namespace KeyPaths

namespace FakeKeyPaths
{
    const ::std::string rootDir{"../../fakeKeys"};
    const ::std::string CaDir{rootDir + "/ca"};
    const ::std::string CaCert{CaDir + "/ca.crt"};
    const ::std::string CaKey{CaDir + "/ca.key"};
    const ::std::string ServerDir{rootDir + "/server"};
    const ::std::string ServerCert{ServerDir + "/server.crt"};
    const ::std::string ServerKey{ServerDir + "/server.key"};
    const ::std::string ClientDir{rootDir + "/client"};
    const ::std::string ClientCert{ClientDir + "/client.crt"};
    const ::std::string ClientKey{ClientDir + "/client.key"};
} // namespace KeyPaths

namespace SelfSignedKeyPaths
{
    const ::std::string rootDir{"../../selfSignedKeys"};
    const ::std::string CaDir{rootDir + "/ca"};
    const ::std::string CaCert{CaDir + "/ca.crt"};
    const ::std::string CaKey{CaDir + "/ca.key"};
    const ::std::string ServerDir{rootDir + "/server"};
    const ::std::string ServerCert{ServerDir + "/server.crt"};
    const ::std::string ServerKey{ServerDir + "/server.key"};
    const ::std::string ClientDir{rootDir + "/client"};
    const ::std::string ClientCert{ClientDir + "/client.crt"};
    const ::std::string ClientKey{ClientDir + "/client.key"};
} // namespace KeyPaths

namespace SecondKeyPaths
{
    const ::std::string rootDir{"../../secondKeys"};
    const ::std::string CaDir{rootDir + "/ca"};
    const ::std::string CaCert{CaDir + "/ca.crt"};
    const ::std::string CaKey{CaDir + "/ca.key"};
    const ::std::string ServerDir{rootDir + "/server"};
    const ::std::string ServerCert{ServerDir + "/server.crt"};
    const ::std::string ServerKey{ServerDir + "/server.key"};
    const ::std::string ClientDir{rootDir + "/client"};
    const ::std::string ClientCert{ClientDir + "/client.crt"};
    const ::std::string ClientKey{ClientDir + "/client.key"};
} // namespace KeyPaths

namespace CertChainDepth2KeyPaths
{
    const ::std::string rootDir{"../../certChainDepth2Keys"};
    const ::std::string CaDir{rootDir + "/ca"};
    const ::std::string CaCert{CaDir + "/ca.crt"};
    const ::std::string CaKey{CaDir + "/ca.key"};
    const ::std::string ServerDir{rootDir + "/server"};
    const ::std::string ServerCert{ServerDir + "/server.crt"};
    const ::std::string ServerKey{ServerDir + "/server.key"};
    const ::std::string ClientDir{rootDir + "/client"};
    const ::std::string ClientCert{ClientDir + "/client.crt"};
    const ::std::string ClientKey{ClientDir + "/client.key"};
}

#endif // TEST_DEFINES_H_
