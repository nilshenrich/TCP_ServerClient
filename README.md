# TCP_ServerClient

Installable packages for TCP based client-server communication.\
TLS encryption is supported with either server authentication, client authentication or both (two-way-authentication).

<details>
<summary>Table of contents</summary>

- [TCP\_ServerClient](#tcp_serverclient)
  - [Summary](#summary)
    - [Limitations](#limitations)
  - [System requirements](#system-requirements)
  - [Installation](#installation)
  - [Usage](#usage)
    - [Example](#example)
    - [Include in custom projects](#include-in-custom-projects)
    - [Message modes](#message-modes)
    - [Server](#server)
    - [Client](#client)
  - [Start return codes](#start-return-codes)
    - [Server](#server-1)
    - [Client](#client-1)
  - [Known issues](#known-issues)
    - [Pipe error if client sends immediately after exiting start](#pipe-error-if-client-sends-immediately-after-exiting-start)
    - [Test execution gets stuck after starting TLS server](#test-execution-gets-stuck-after-starting-tls-server)
    - [Segmentation fault on full test execution](#segmentation-fault-on-full-test-execution)
  
</details>

## Summary

This project provides installable C++ libraries for setting up TCP based client-server communications.\
Such a connection can be:
1. Unencrypted plain text TCP
1. Encrypted with TLS (TLSv1.3 and authentication):
   1. Server authentication (one-way-authentication)
   1. Client authentication (one-way-authentication)
   1. Both, server and client authentication (two-way-authentication)

Both, the server and the client can send and receive messages asynchronously. To send a message to an established connection, a send method can be called any time. To work on incoming messages, a receive method can be defined to be called immediately in a separate thread.

A regular TCP package has max maximum length of 65536 bytes. Under the hood a TCP package size is limited to 16384 for this library, but it is possible to send and receive messages of any size thanks to built in fragmentation and reassembling.\
A connection can be established for one of the following two modes:
* Fragmented:\
  This mode sends and receives whole text messages of a finite size.\
  A receive worker method can be defined dealing with a string variable containing the message.\
  To separate messages, a delimiter must be defined. Please make sure that the delimiter is not part of any message.
* Continuous:\
  This mode sends and receives a continuous stream of data.\
  Instead of working on a single string variable, a receive worker method deals with an incoming stream.

### Limitations

There are some size limitations for this library:
* Maximum allowed connections a server can handle simultaneously: 4096\
  *This limitation is only applied to a single server instance. It is possible to create an application with multiple servers with a maximum of 4096 connections each.*
* Maximum allowed message length:\
  *This number depends on the CPU architecture and available memory you are using.\
  For checking, just call the method `max_size()` on any variable of type string. For most modern systems, this value is such high that it can be treated as infinity (On a regular 64bit system, this number is 2<sup>64</sup>-1).*

## System requirements

This library is developed on a debian based system, so this manual is specific to it.\
Nevertheless, it only depends on the C++17 standard, the standard library and the OpenSSL C++ library, so it should be possible to use it on other systems.

For the hardware I'm not giving any limitations. It is usable on low level hardware as well as on high performance systems.

## Installation

The project has no pre-compilable parts. The server and client parts are header-only libraries.\
To use the library in your project, just include the header files.\
To install the library on your system, follow these steps:

1. Install necessary third party software

    ```console
    sudo apt install build-essential libssl-dev
    ```

    * **build-essential** contains the GNU C++ compiler as well as the GNU C++ standard library.
    * **libssl-dev** contains additional C++ libraries for SSL/TLS encryption.

    For creating self-signed certificates and running the tests or examples, the **openssl** tool is required:

    ```console
    sudo apt install openssl
    ```

2. Install the library on your system

    ```console
    sudo make install
    ```

## Usage

To see a basic example that shows you all functionality, please build and run the [example](./example) project:

### Example

1. Install cmake to compile the example

    ```console
    sudo apt install cmake
    ```

    The other third party software mentioned in [Installation](#installation) is needed as well.

2. Build both applications

    ```console
    mkdir build
    cd build
    cmake ..
    make
    ```

    Building the example project automatically installs the library on your system and creates self-signed certificates for the TLS encryption.

3. Start the server and client and follow instructions on the console

    Open a terminal and start the server
    ```console
    ./server
    ```

    Open a second terminal and start the client
    ```console
    ./client
    ```

### Include in custom projects

Installing the project copies the library headers on your system.\
To use it in your project, include the header files from the sub-folder **tcp**:

```cpp
#include <tcp/TcpServer.h>
#include <tcp/TcpClient.h>
#include <tcp/TlsServer.h>
#include <tcp/TlsClient.h>

using namespace std;
using namespace tcp; // The entire library is in the namespace tcp
```

The following linker flags are mandatory to be set depending on what sub-features are used:

* **-lssl**     if the TLS encryption is used

### Message modes

For both, an unencrypted TCP and encrypted TLS connection, one of two modes can be selected for exchanging messages.

#### Fragmented

In the fragmented mode, all messages are text packages with a finite length. When receiving a message, the message is buffered in a string variable that can be processed in the receive worker method. To separate messages, a delimiter must be defined to separate individual messages on the network stream. Please make sure that the delimiter is not part of any message.

#### Continuous

In the continuous mode, a continuous stream of data is sent and received. To work on incoming data, an outgoing stream must be defined. For a client, that holds just one active connection to a server, a pointer to an outgoing stream must be defined. For a server, that holds multiple connections, a method returning a pointer to an outgoing stream must be defined based on the sending client ID. Please keep in mind that returning a pointer to an existing stream will crash the program. For the server, the stream must be generated with the **new** keyword in the definition method.

### Server

The following examples are done for a TCP server, but they can be used for a TLS server as well.

#### Create instance

```cpp
TcpServer server; // Constructor with no arguments gives a server in continuous mode
TcpServer server{'|'}; // Constructor with delimiter argument gives a server in fragmented mode
TcpServer server{'|', "--message end--"}; // In fragmented mode, a custom text (string) can be defined that is appended at the end of each outgoing message
TcpServer server{'|', 4096}; // In fragmented mode, the maximum message length (for sending and receiving) can be set
```

#### Define and link worker methods

Worker methods can be defined for the following events:

* New connection established
* New message received
* Connection closed

```cpp
// Worker for established connection to a client
void worker_established(int clientId)
{
    // Do stuff immediately after establishing a new connection
    // (clientId could be changed if needed)
}

// Worker for closed connection to client
void worker_closed(int clientId)
{
    // Do stuff after closing connection
    // (clientId could be changed if needed)
}

// Worker for incoming message (Only used in fragmentation-mode)
void worker_message(int clientId, string msg)
{
    // Do stuff with message
    // (clientId and msg could be changed if needed)
}

// Output stream generator
ofstream *generator_outStream(int clientId)
{
    // This method is called when establishing a new connection, even before the worker_established method
    // Stream must be generated with new
    // This example uses file stream but any other ostream could be used
    // (clientId could be changed if needed)
    return new ofstream{"FileForClient_"s + to_string(clientId)};
}

// Link worker functions using the following linker methods:
tcpServer.setWorkOnEstablished(&worker_established)
tcpServer.setWorkOnClosed(&worker_closed)
tcpServer.setWorkOnMessage(&worker_message)
tcpServer.setCreateForwardStream(&generator_outStream)
```

A worker method can be linked to the server on several ways:
*Using worker_established as example here, this works for all other worker functions similarly.*

1. Standalone function
    ```cpp
    void worker_established(int clientId)
    {
        // Do stuff immediately after establishing a new connection
        // (clientId could be changed if needed)
    }
    TcpServer tcpServer;
    tcpServer.setWorkOnEstablished(&worker_established);
    ```

2. Member function of **this** instance
   
    ```cpp
    class ExampleClass
    {
    public:
        ExampleClass()
        {
            tcpServer.setWorkOnEstablished(::std::bind(&ExampleClass::classMember, this, ::std::placeholders::_1));
        }
        virtual ~ExampleClass() {}

    private:
        // TCP server as class member
        TcpServer tcpServer{};

        void classMember(const int clientId)
        {
            // Some code
        }
    };
    ```

    The **bind** function is used to get the function reference to a method from an object, in this case ```this```. For each attribute of the passed function, a placeholder with increasing number must be passed.

3. Member function of foreign class

    ```cpp
    class ExampleClass
    {
    public:
        ExampleClass() {}
        virtual ~ExampleClass() {}

    private:
        void classMember(const int clientId)
        {
            // Some code
        }
    };

    // Create object
    ExampleClass exampleClass;

    // TCP server outside from class
    TcpServer tcpServer;
    tcpServer.setWorkOnEstablished(::std::bind(&ExampleClass::classMember, exampleClass, ::std::placeholders::_1));
    ```

4. Lambda function

    ```cpp
    TcpServer tcpServer;
    tcpServer.setWorkOnEstablished([](const int clientId)
    {
        // Some code
    });
    ```

#### Server methods

The following methods are the same for all kinds of servers (TCP or TLS in fragmented or continuous mode):

1. start():

    The **start**-method is used to start a TCP or TLS server. When this method returns 0, the server runs in the background. If the return value is other that 0, please see [Defines.h](Server/include/Defines.h) or [Start return codes - server](#server-1) for definition of error codes.

    ```cpp
    TcpServer tcpServer;
    tcpServer.start(8081);
    ```

2. stop():

    The **stop**-method stops a running server.

    ```cpp
    tcpServer.stop();
    ```

3. sendMsg():

    The **sendMsg**-method sends a message to a connected client (over TCP or TLS). If the return value is **true**, the sending was successful, if it is **false**, not.

    ```cpp
    tcpServer.sendMsg(4, "example message over TCP");
    ```

4. TlsServer::setCertificates():

    The **setCertificates**-method is only available for **TlsServer** and is used to set the certificates for the server.\
    The first argument takes a path to the CA certificate file, used to authenticate the client as issuer in 1. place.\
    The second and third arguments take the path to the server certificate and the server key file to authenticate itself.

    ```cpp
    tlsServer.setCertificates("ca_cert.pem", "server_cert.pem", "server_key.pem")
    ```

5. TlsServer::clearCertificates():

    The **clearCertificates**-method is only available for **TlsServer** and is used to clear the certificates for the server.\
    This method is useful when the server should run without TLS encryption.

    ```cpp
    tlsServer.clearCertificates();
    ```

6. TlsServer::requireClientAuthentication():

    The **requireClientAuthentication**-method is only available for **TlsServer** and is used to require client authentication.\
    If the passed value is **true**, the client must authenticate itself with a certificate issued by the CA, if it is **false**, the client can connect without a or self signed certificate.

    ```cpp
    tlsServer.requireClientAuthentication(true);
    ```

7. getClientIp():

    The **getClientIp**-method returns the IP address of a connected client (TCP or TLS) identified by its TCP ID. If no client with this ID is connected, the string **"Failed Read!"** is returned.

8. TlsServer::getSubjPartFromClientCert():

    The **getSubjPartFromClientCert**-method only exists for **TlsServer** and returns a given subject part of the client's certificate identified by its TCP ID or its tlsSocket (SSL*). If the tlsSocket parameter is*nullptr*, the client is identified by its TCP ID, otherwise it is identified by the given tlsSocket parameter.

    The subject of a certificate contains information about the certificate owner. Here is a list of all subject parts and how to get them using **getSubjPartFromClientCert()**:

    - **NID_countryName**: Country code (Germany = DE)
    - **NID_stateOrProvinceName**: State or province name (e.g. Baden-Württemberg)
    - **NID_localityName**: City name (e.g. Stuttgart)
    - **NID_organizationName**: Organization name
    - **NID_organizationalUnitName**: Organizational unit name
    - **NID_commonName**: Name of owner

    For example

    ```cpp
    tlsServer.getSubjPartFromClientCert(4, nullptr, NID_localityName);
    ```

    will return "Stuttgart" if this is the client's city name.

9. isRunning():

    The **isRunning**-method returns the running flag of the server.\
    **True** means: *The server is running*\
    **False** means: *The server is not running*

### Client

The following examples are done for a TCP client, but they can be used for a TLS client as well.

#### Create instance

```cpp
myStream ofstream("MyFile.txt");

TcpClient client; // Constructor with no arguments gives a client in continuous mode forwarding to stdout
TcpClient client{myStream}; // Constructor with stream argument gives a client in continuous mode forwarding to a file
TcpClient client{'|'}; // Constructor with delimiter argument gives a client in fragmented mode
TcpClient client{'|', "--message end--"}; // In fragmented mode, a custom text (string) can be defined that is appended at the end of each outgoing message
TcpClient client{'|', 4096}; // In fragmented mode, the maximum message length (for sending and receiving) can be set
```

#### Define and link worker methods

The only worker function a client provides is working on received messages in fragmented mode.

```cpp
// Worker for incoming message (Only used in fragmentation-mode)
void worker_message(string msg)
{
    // Do stuff with message
}
TcpClient tcpClient;
tcpClient.setWorkOnMessage(&worker_message);
```

This function can be linked to client similarly to server via standalone, member or lambda function.

#### Client methods

1. start():

    The **start**-method is used to start a TCP or TLS client. When this method returns 0, the client runs in the background. If the return value is other that 0, please see [Defines.h](Client/include/Defines.h) or [Start return codes - client](#client-1) for definition of error codes.

    ```cpp
    TcpClient tcpClient;
    tcpClient.start("serverHost", 8081);
    ```

2. stop():

    The **stop**-method stops a running client.

    ```cpp
    tcpClient.stop();
    ```

3. TlsClient::setCertificates():
   
    The **setCertificates**-method is only available for **TlsClient** and is used to set the certificates for the client.\
    The first argument takes a path to the CA certificate file, used to authenticate the server as issuer in 1. place.\
    The second and third arguments take the path to the client certificate and the client key file to authenticate itself.

    ```cpp
    tlsClient.setCertificates("ca_cert.pem", "client_cert.pem", "client_key.pem")
    ```

4. TlsClient::clearCertificates():

    The **clearCertificates**-method is only available for **TlsClient** and is used to clear the certificates for the client.\
    This method is useful when the client should run without TLS encryption.

    ```cpp
    tlsClient.clearCertificates();
    ```

5. TlsClient::requireServerAuthentication():
    
        The **requireServerAuthentication**-method is only available for **TlsClient** and is used to require server authentication.\
        If the passed value is **true**, the server must authenticate itself with a certificate issued by the CA, if it is **false**, the server can connect without a certificate.
    
        ```cpp
        tlsClient.requireServerAuthentication(true);
        ```

6. sendMsg():

    The **sendMsg**-method sends a message to the server (over TCP or TLS). If the return value is **true**, the sending was successful, if it is **false**, not.

    ```cpp
    tcpClient.sendMsg("example message over TCP");
    ```

7. isRunning():

    The **isRunning**-method returns the running flag of the client.\
    **True** means: *The client is running*\
    **False** means: *The client is not running*

## Start return codes

When calling the **start**-method, on server or client, an ineger value is returned. 0 always means success and the server/client is now running in the background until the **stop**-method is called. Other values indicate the following errors errors (see [Defines.h](Server/include/Defines.h) for server and [Defines.h](Client/include/Defines.h) for client):

### Server

* **10**: Server could not start because of wrong port number
* **20**: Server could not start because of SSL context error
* **30**: Server could not start because of wrong path to CA cert file
* **31**: Server could not start because of wrong path to certificate file
* **32**: Server could not start because of wrong path to key file
* **33**: Server could not start because of bad CA cert file
* **34**: Server could not start because of bad certificate file
* **35**: Server could not start because of bad key file or non matching key with certificate
* **40**: Server could not start because of TCP socket creation error
* **41**: Server could not start because of TCP socket option error
* **42**: Server could not start because of TCP socket bind error
* **43**: Server could not start because of TCP socket listen error

### Client

* **10**: Client could not start because of wrong port number
* **20**: Client could not start because of SSL context error
* **30**: Client could not start because of wrong path to CA cert file
* **31**: Client could not start because of wrong path to certifcate file
* **32**: Client could not start because of wrong path to key file
* **33**: Client could not start because of bad CA cert file
* **34**: Client could not start because of bad certificate file
* **35**: Client could not start because of bad key file or non matching key with certificate
* **40**: Client could not start because of TCP socket creation error
* **41**: Client could not start because of TCP socket options error
* **50**: Client could not start because of TCP socket connection error
* **60**: Client could not start because of an error while initializing the connection

## Known issues

### [Pipe error if client sends immediately after exiting start](https://github.com/nilshenrich/TCP_ServerClient/issues/1)

When a client sends a message to the server immediately after the TlsServer::start() method returned, the server program throws a pipe error.

Waiting for a short time after connecting to server will fix it on client side.

To prevent a program crash on server side, a pipe error can simply be ignored:

```cpp
// Handle pipe error
void signal_handler(int signum)
{
    // Ignore pipe error
    if (signum == SIGPIPE)
    {
        ::std::cout << "SIGPIPE ignored" << ::std::endl; // Pipe error ignored
        return;
    }

    // For any other error, exit program
    exit(signum);
}

// Register pipe error signal to handler
signal(SIGPIPE, signal_handler);

// Start a server regularly
TcpServer server;
server.start(8080);

// Do your stuff here ...
```

### [Test execution gets stuck after starting TLS server](https://github.com/nilshenrich/TCP_ServerClient/issues/4)

In some gtest executions, the runtime gets stuck right after starting TLS server, see:
![Unittest_stuck](https://private-user-images.githubusercontent.com/57060396/341879963-e12b0e48-da9e-43e0-9079-d32dea267326.png?jwt=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJnaXRodWIuY29tIiwiYXVkIjoicmF3LmdpdGh1YnVzZXJjb250ZW50LmNvbSIsImtleSI6ImtleTUiLCJleHAiOjE3MzkxODAzMjYsIm5iZiI6MTczOTE4MDAyNiwicGF0aCI6Ii81NzA2MDM5Ni8zNDE4Nzk5NjMtZTEyYjBlNDgtZGE5ZS00M2UwLTkwNzktZDMyZGVhMjY3MzI2LnBuZz9YLUFtei1BbGdvcml0aG09QVdTNC1ITUFDLVNIQTI1NiZYLUFtei1DcmVkZW50aWFsPUFLSUFWQ09EWUxTQTUzUFFLNFpBJTJGMjAyNTAyMTAlMkZ1cy1lYXN0LTElMkZzMyUyRmF3czRfcmVxdWVzdCZYLUFtei1EYXRlPTIwMjUwMjEwVDA5MzM0NlomWC1BbXotRXhwaXJlcz0zMDAmWC1BbXotU2lnbmF0dXJlPTRlM2UxMzMzMGY4M2E5MDNjMGQyMGZlNjMyNDZkNzJhMWVhNjU3MDJlYmQ0ZTA2YTUwNDdjZjIzYThiZWEwNjUmWC1BbXotU2lnbmVkSGVhZGVycz1ob3N0In0.Yw1K45Afv6f0UikjsGqOqfmNKODg3yxnm3YT6cpbKBQ)

### [Segmentation fault on full test execution](https://github.com/nilshenrich/TCP_ServerClient/issues/5)

When the complete gtest is executed with JSON output, the runtime crashes with segmentation fault after not finding an available port in SetUp section.
