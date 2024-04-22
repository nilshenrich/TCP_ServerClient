# TCP_ServerClient

Installable packages for TCP based client-server communication.\
TLS encryption with two-way authentication is supported.

<details>
<summary>Table of contents</summary>

- [TCP\_ServerClient](#tcp_serverclient)
  - [Summary](#summary)
    - [Limitations](#limitations)
  - [System requirements](#system-requirements)
  - [Installation](#installation)
  - [Usage](#usage)
    - [Message modes](#message-modes)
      - [Fragmented](#fragmented)
      - [Continuous](#continuous)
    - [Server](#server)
      - [Create instance](#create-instance)
      - [Define and link worker methods](#define-and-link-worker-methods)
      - [Server methods](#server-methods)
    - [Client](#client)
      - [Create instance](#create-instance-1)
      - [Define and link worker methods](#define-and-link-worker-methods-1)
      - [Client methods](#client-methods)
  - [Known issues](#known-issues)
    - [Pipe error if client sends immediately after exiting start](#pipe-error-if-client-sends-immediately-after-exiting-start)
  
</details>

## Summary

This project provides installable C++ libraries for setting up TCP based client-server communications.\
Such a connection can be:
1. Unencrypted plain text TCP
1. Encrypted with TLS (TLSv1.3 and two-way authentication)

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
  For checking, just call the method `max_size()` on any variable of type string. For most modern systems, this value is such high that it can be treated as infinity.*

## System requirements

This library is developed on a debian based system, so this manual is specific to it.\
Nevertheless, it only depends on the C++17 standard and the standard library, so it should be possible to use it on other systems.

For the hardware I'm not giving any limitations. It is usable on low level hardware as well as on high performance systems.

## Installation

The following steps can be applied for the entire project if you want to install both, the server and the client part and it can be applied for each part separately.

1. Install necessary third party software

    ```console
    sudo apt install build-essential cmake libssl-dev
    ```

    * **build-essential** contains the GNU C++ compiler as well as the GNU C++ standard library.
    * **cmake** is used to create a makefile from the more general CMakeLists.txt.
    * **libssl-dev** contains additional C++ libraries for SSL/TLS encryption.

    For creating self-signed certificates and running the tests or examples, the **openssl** tool is required:

    ```console
    sudo apt install openssl
    ```

2. Create a build directory and move to it
   
    ```console
    mkdir build
    cd build
    ```

3. Build the library

    ```console
    cmake ..
    make
    ```

4. Install the library
   
    ```console
    sudo make install
    ```

5. [optional] Fix shared library dependencies

    If you get the following error message when running an application:

    ```console
    error while loading shared libraries: libds18b20.so.1: cannot open shared object file: No such file or directory
    ```

    running the following command can solve it:

    ```console
    sudo /sbin/ldconfig
    ```

    [see details](https://itsfoss.com/solve-open-shared-object-file-quick-tip/)

## Usage

To see a basci example that shows you all functionality, please build and run the [example](example) project:

1. Build both applications

    ```console
    mkdir build
    cd build
    cmake ..
    make
    ```

2. Create self-signed certificates for TLS encryption

    ```console
    ./CreateCerts.sh
    ```

3. Start the server and client

    Open a terminal and start the server
    ```console
    ./server
    ```

    Open a second terminal and start the client
    ```console
    ./Client
    ```

The following linker flags are mandatory to be set to tell the system what libraries to use:

* **-lTcpServer**   if the TCP server is used
* **-lTcpClient**   if the TCP client is used
* **-lTlsServer**   if the TLS server is used
* **-lTlsClient**   if the TLS client is used
* **-lcrypto**      if the TLS encryption is used
* **-lssl**         if the TLS encryption is used
* **-pthread**      always needed

### Message modes

For both, an unencrypted TCP and encrypted TLS connection, one of two modes can be selected for exchanging messages.

#### Fragmented

In the fragmented mode, all messages are text packages with a finite length. When receiving a message, the message is buffered in a string variable that can be processed in the receive worker method. To separate messages, a delimiter must be defined to separate individual messages on the network stream. Please make sure that the delimiter is not part of any message.

#### Continuous

In the continuous mode, a continuous stream of data is sent and received. To work on incoming data, a outgoing stream must be defined. For a client, that holds just one active connection to a server, a pointer to an outgoing stream must be defined. For a server, that holds multiple connections, a method returning a pointer to an outgoing stream must be defined based on the sending client ID.

### Server

The following examples are done for a TCP server, but they can be used for a TLS server as well.

#### Create instance

```cpp
TcpServer server; // Constructor with no arguments gives a server in continuous mode
TcpServer server{'|'}; // Constructor with delimiter argument gives a server in fragmented mode
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
ofstream *genertor_outStream(int clientId)
{
    // Stream must be generated with new
    // This example uses file stream but any other ostream could be used
    // (clientId could be changed if needed)
    return new ofstream{"FileForClient_"s + to_string(clientId)};
}

// Link worker functions using the following linker methods:
tcpServer.setWorkOnEstablished(&worker_established)
tcpServer.setWorkOnClosed(&worker_closed)
tcpServer.setWorkOnMessage(&worker_message)
tcpServer.setCreateForwardStream(&genertor_outStream)
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

    The **start**-method is used to start a TCP or TLS listener. When this method returns 0, the listener runs in the background. If the return value is other that 0, please see [Defines.h](include/Defines.h) for definition of error codes.\
    If your class derived from both **TcpServer** and **TlsServer**, the class name must be specified when calling **start()**:

    ```cpp
    TcpServer::start(8081);
    TlsServer::start(8082, "ca_cert.pem", "server_cert.pem", "server_key.pem");
    ```

1. stop():

    The **stop**-method stops a running listener.\
    As for **start()**, if your class derived from both **TcpServer** and **TlsServer**, the class name must be specified when calling **stop()**:

    ```cpp
    TcpServer::stop();
    TlsServer::stop();
    ```

1. sendMsg():

    The **sendMsg**-method sends a message to a connected client (over TCP or TLS). If the return value is **true**, the sending was successful, if it is **false**, not.\
    As for **start()**, if your class derived from both **TcpServer** and **TlsServer**, the class name must be specified when calling **sendMsg()**:

    ```cpp
    TcpServer::sendMsg(4, "example message over TCP");
    TlsServer::sendMsg(5, "example message over TLS");
    ```

    Please make sure to only use **TcpServer::sendMsg()** for TCP connections and **TlsServer::sendMsg()** for TLS connection.

1. getClientIp():

    The **getClientIp**-method returns the IP address of a connected client (TCP or TLS) identified by its TCP ID. If no client with this ID is connected, the string **"Failed Read!"** is returned.

1. TlsServer::getSubjPartFromClientCert():

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
    getSubjPartFromClientCert(4, nullptr, NID_localityName);
    ```

    will return "Stuttgart" if this is the client's city name.

1. isRunning():

    The **isRunning**-method returns the running flag of the NetworkListener.\
    **True** means: *The listener is running*\
    **False** means: *The listener is not running*

### Client

The following examples are done for a TCP client, but they can be used for a TLS client as well.

#### Create instance

```cpp
myStream ofstream("MyFile.txt");

TcpClient client; // Constructor with no arguments gives a client in continuous mode forwarding to stdout
TcpClient client{myStream}; // Constructor with stream argument gives a client in continuous mode forwarding to a file
TcpClient client{'|'}; // Constructor with delimiter argument gives a client in fragmented mode
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

    The **start**-method is used to start a TCP or TLS client. When this method returns 0, the client runs in the background. If the return value is other that 0, please see [Defines.h](include/Defines.h) for definition of error codes.\
    If your class derived from both **TcpClient** and **TlsClient**, the class name must be specified when calling **start()**:

    ```cpp
    TcpClient::start("serverHost", 8081);
    TlsClient::start("serverHost", 8082, "ca_cert.pem", "client_cert.pem", "client_key.pem");
    ```

1. stop():

    The **stop**-method stops a running client.\
    As for **start()**, if your class derived from both **TcpClient** and **TlsClient**, the class name must be specified when calling **stop()**:

    ```cpp
    TcpClient::stop();
    TlsClient::stop();
    ```

1. sendMsg():

    The **sendMsg**-method sends a message to the server (over TCP or TLS). If the return value is **true**, the sending was successful, if it is **false**, not.\
    As for **start()**, if your class derived from both **TcpClient** and **TlsClient**, the class name must be specified when calling **sendMsg()**:

    ```cpp
    TcpClient::sendMsg("example message over TCP");
    TlsClient::sendMsg("example message over TLS");
    ```

    Please make sure to only use **TcpClient::sendMsg()** for TCP connections and **TlsClient::sendMsg()** for TLS connection.

1. isRunning():

    The **isRunning**-method returns the running flag of the NetworkClient.\
    **True** means: *The client is running*\
    **False** means: *The client is not running*

## Known issues

### [Pipe error if client sends immediately after exiting start](https://github.com/nilshenrich/TCP_ServerClient/issues/1)

When a client sends a message to the listener immediately after the NetworkClient::start() method returned, the listener program throws a pipe error.

Waiting for a short time after connecting to server will fix it on client side.

To prevent a program crash on server side, a pipe error can simply be ignored:

```cpp
// Handle pip error
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

// Register pip error signal to handler
signal(SIGPIPE, signal_handler);

// Start a server regularly
TcpServer server;
server.start(8080);

// Do your stuff here ...
```
