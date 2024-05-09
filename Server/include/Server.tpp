template <class SocketType, class SocketDeleter>
int Server<SocketType, SocketDeleter>::start(
    const int port,
    const char *const pathToCaCert,
    const char *const pathToCert,
    const char *const pathToPrivKey)
{
    using namespace std;

    // If the listener is already running, return error
    if (running)
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Listener already running" << endl;
#endif // DEVELOP

        return -1;
    }

    // Check if the port number is valid
    if (1 > port || 65535 < port)
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": The port " << port << " couldn't be used" << endl;
#endif // DEVELOP

        return SERVER_ERROR_START_WRONG_PORT;
    }

    // Initialize the listener and return error if it fails
    int initCode{init(pathToCaCert, pathToCert, pathToPrivKey)};
    if (initCode)
        return initCode;

    // Create the TCP socket for the listener to accept new connections.
    // Return error if it fails
    tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == tcpSocket)
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Error when creating TCP socket to listen on" << endl;
#endif // DEVELOP

        // Stop the listener
        stop();

        return SERVER_ERROR_START_CREATE_SOCKET;
    }

    // Set options on the TCP socket for the listener to accept new connections.
    // (Reuse address)
    // Return error if it fails
    int opt{0};
    if (setsockopt(tcpSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Error when setting TCP socket options" << endl;
#endif // DEVELOP

        // Stop the listener
        stop();

        return SERVER_ERROR_START_SET_SOCKET_OPT;
    }

    // Initialize the socket address for the listener.
    memset(&socketAddress, 0, sizeof(socketAddress));
    socketAddress.sin_family = AF_INET;
    socketAddress.sin_addr.s_addr = INADDR_ANY;
    socketAddress.sin_port = htons(port);

    // Bind the TCP socket for the listener to accept new connections to the socket address.
    // Return error if it fails
    if (bind(tcpSocket, (struct sockaddr *)&socketAddress, sizeof(socketAddress)))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Error when binding listener to port " << port << endl;
#endif // DEVELOP

        // Stop the listener
        stop();

        return SERVER_ERROR_START_BIND_PORT;
    }

    // Start listening on the TCP socket for the listener to accept new connections.
    if (listen(tcpSocket, SOMAXCONN))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Error when starting listening" << endl;
#endif // DEVELOP

        // Stop the listener
        stop();

        return SERVER_ERROR_START_LISTENER;
    }

    // Start the thread to accept new connections
    if (accHandler.joinable())
        throw Server_error("Start listener thread failed: Thread is already running"s);
    accHandler = thread{&Server::listenerAccept, this};

    // Listener is now running
    running = true;

#ifdef DEVELOP
    cout << typeid(this).name() << "::" << __func__ << ": Listener started on port " << port << endl;
#endif // DEVELOP

    return initCode;
}

template <class SocketType, class SocketDeleter>
void Server<SocketType, SocketDeleter>::stop()
{
    using namespace std;

    // Stop the listener
    running = false;

    // Block listening TCP socket to abort all reads
    int shut{shutdown(tcpSocket, SHUT_RDWR)};

    // Wait for the accept thread to finish
    if (accHandler.joinable())
        accHandler.join();

    // If shutdown failed, abort stop here
    if (shut)
        return;

    // Close listening TCP socket
    close(tcpSocket);

#ifdef DEVELOP
    cout << typeid(this).name() << "::" << __func__ << ": Listener stopped" << endl;
#endif // DEVELOP

    return;
}

template <class SocketType, class SocketDeleter>
bool Server<SocketType, SocketDeleter>::sendMsg(const int clientId, const std::string &msg)
{
    using namespace std;

    if (MESSAGE_FRAGMENTATION_ENABLED)
    {
        // Check if message doesn't contain delimiter
        if (msg.find(DELIMITER_FOR_FRAGMENTATION) != string::npos)
        {
#ifdef DEVELOP
            cerr << typeid(this).name() << "::" << __func__ << ": Message contains delimiter" << endl;
#endif // DEVELOP

            return false;
        }

        // Check if message is too long
        if (msg.length() > MAXIMUM_MESSAGE_LENGTH_FOR_FRAGMENTATION)
        {
#ifdef DEVELOP
            cerr << typeid(this).name() << "::" << __func__ << ": Message is too long" << endl;
#endif // DEVELOP

            return false;
        }
    }

    // Extend message with start and end characters and send it
    lock_guard<mutex> lck{activeConnections_m};
    if (activeConnections.find(clientId) != activeConnections.end())
        return writeMsg(clientId, MESSAGE_FRAGMENTATION_ENABLED ? msg + string{DELIMITER_FOR_FRAGMENTATION} : msg);

#ifdef DEVELOP
    cerr << typeid(this).name() << "::" << __func__ << ": Client " << clientId << " is not connected" << endl;
#endif // DEVELOP

    return false;
}

template <class SocketType, class SocketDeleter>
void Server<SocketType, SocketDeleter>::setWorkOnMessage(std::function<void(const int, const std::string)> worker)
{
    workOnMessage = worker;
}

template <class SocketType, class SocketDeleter>
void Server<SocketType, SocketDeleter>::setCreateForwardStream(std::function<std::ostream *(const int)> creator)
{
    generateNewForwardStream = creator;
}

template <class SocketType, class SocketDeleter>
void Server<SocketType, SocketDeleter>::setWorkOnEstablished(std::function<void(const int)> worker)
{
    workOnEstablished = worker;
}

template <class SocketType, class SocketDeleter>
void Server<SocketType, SocketDeleter>::setWorkOnClosed(std::function<void(const int)> worker)
{
    workOnClosed = worker;
}

template <class SocketType, class SocketDeleter>
std::vector<int> Server<SocketType, SocketDeleter>::getAllClientIds() const
{
    using namespace std;

    vector<int> ret;
    for (auto &v : activeConnections)
        ret.push_back(v.first);
    return ret;
}

template <class SocketType, class SocketDeleter>
std::string Server<SocketType, SocketDeleter>::getClientIp(const int clientId) const
{
    using namespace std;

    struct sockaddr_in addr;
    socklen_t addrSize = sizeof(struct sockaddr_in);
    if (getpeername(clientId, (struct sockaddr *)&addr, &addrSize))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Error reading client " << clientId << "'s IP address" << endl;
#endif // DEVELOP

        return "Failed Read!";
    }

    // Convert the IP address to a string and return it
    return string{inet_ntoa(addr.sin_addr)};
}

template <class SocketType, class SocketDeleter>
void Server<SocketType, SocketDeleter>::listenerAccept()
{
    using namespace std;

    // Get the size of the socket address for the listener (important for connection establishment)
    socklen_t socketAddress_len{sizeof(socketAddress)};

    // Accept new connections while the listener is running
    while (running)
    {
        // Wait for a new connection to accept
        const int newConnection{accept(tcpSocket, (struct sockaddr *)&socketAddress, &socketAddress_len)};

        // If new accepted connection ID is -1, the accept failed
        // In this case, continue with accepting the new connections
        if (-1 == newConnection)
            continue;

#ifdef DEVELOP
        cout << typeid(this).name() << "::" << __func__ << ": New client connected: " << newConnection << endl;
#endif // DEVELOP

        // Initialize the (so far unencrypted) connection
        SocketType *connection_p{connectionInit(newConnection)};
        if (!connection_p)
            continue;

        // Add connection to active connections
        {
            lock_guard<mutex> lck{activeConnections_m};
            activeConnections[newConnection] = unique_ptr<SocketType, SocketDeleter>{connection_p};
        }

        // When a new connection is established, the incoming messages of this connection should be read in a new process
        unique_ptr<RunningFlag> recRunning{new RunningFlag{true}};
        thread rec_t{&Server::listenerReceive, this, newConnection, recRunning.get()};

        // Get all finished receive handlers
        vector<int> toRemove;
        for (auto &flag : recHandlersRunning)
        {
            if (!*flag.second.get())
                toRemove.push_back(flag.first);
        }

        // Remove finished receive handlers
        for (auto &id : toRemove)
        {
            recHandlers[id].join();
            recHandlers.erase(id);
            recHandlersRunning.erase(id);
        }

        // Add new receive handler (Running flag is added inside receive thread)
        recHandlers[newConnection] = move(rec_t);
        recHandlersRunning[newConnection] = move(recRunning);
    }

    // Abort receiving for all active connections by shutting down the read channel
    // Complete shutdown and close is done in receive threads
    {
        lock_guard<mutex> lck{activeConnections_m};
        for (const auto &it : activeConnections)
        {
            shutdown(it.first, SHUT_RD);

#ifdef DEVELOP
            cout << typeid(this).name() << "::" << __func__ << ": Closed connection to client " << it.first << endl;
#endif // DEVELOP
        }
    }

    // Wait for all receive processes to finish
    for (auto &it : recHandlers)
        it.second.join();

    return;
}

template <class SocketType, class SocketDeleter>
void Server<SocketType, SocketDeleter>::listenerReceive(const int clientId, RunningFlag *const recRunning_p)
{
    using namespace std;

    // Mark Thread as running (Add running flag and connect to handler)
    Server_running_manager running_mgr{*recRunning_p};

    // Get connection from map
    SocketType *connection_p;
    {
        lock_guard<mutex> lck{activeConnections_m};
        if (activeConnections.find(clientId) == activeConnections.end())
            return;
        connection_p = activeConnections[clientId].get();
    }

    // Create continuous stream for this connection
    if (generateNewForwardStream)
        forwardStreams[clientId] = unique_ptr<ostream>{generateNewForwardStream(clientId)};

    // Run worker for new established connections
    if (workOnEstablished)
        workOnEstablished(clientId);

    // Vectors of running work handlers and their status flags
    vector<thread> workHandlers;
    vector<unique_ptr<RunningFlag>> workHandlersRunning;

    // Read incoming messages from this connection as long as the connection is active
    string buffer;
    while (1)
    {
        // Wait for new incoming message (implemented in derived classes)
        // If message is empty string, the connection is broken
        // BUG: Execution stucks here if server is stopped immediately after client connection
        string msg{readMsg(connection_p)};
        if (msg.empty())
        {
#ifdef DEVELOP
            cout << typeid(this).name() << "::" << __func__ << ": Connection to client " << clientId << " broken" << endl;
#endif // DEVELOP

            {
                lock_guard<mutex> lck{activeConnections_m};

                // Deinitialize the connection
                connectionDeinit(connection_p);

                // Block the connection from being used anymore
                shutdown(clientId, SHUT_RDWR);

                // Remove connection from active connections
                activeConnections.erase(clientId);
            }

            // Run code to handle the closed connection
            if (workOnClosed)
                workOnClosed(clientId);

            // Close the connection
            close(clientId);

            // Wait for all work handlers to finish
            for (auto &it : workHandlers)
                it.join();

            // Remove continuous stream
            if (forwardStreams.find(clientId) != forwardStreams.end())
                forwardStreams.erase(clientId);

            return;
        }

        // If stream shall be fragmented ...
        if (MESSAGE_FRAGMENTATION_ENABLED)
        {
            // Get raw message separated by delimiter
            // If delimiter is found, the message is split into two parts
            size_t delimiter_pos{msg.find(DELIMITER_FOR_FRAGMENTATION)};
            while (string::npos != delimiter_pos)
            {
                string msg_part{msg.substr(0, delimiter_pos)};
                msg = msg.substr(delimiter_pos + 1);
                delimiter_pos = msg.find(DELIMITER_FOR_FRAGMENTATION);

                // Check if the message is too long
                if (buffer.size() + msg_part.size() > MAXIMUM_MESSAGE_LENGTH_FOR_FRAGMENTATION)
                {
#ifdef DEVELOP
                    cerr << typeid(this).name() << "::" << __func__ << ": Message from client " << clientId << " is too long" << endl;
#endif // DEVELOP

                    buffer.clear();
                    continue;
                }

                buffer += msg_part;

#ifdef DEVELOP
                cout << typeid(this).name() << "::" << __func__ << ": Message from client " << clientId << ": " << buffer << endl;
#endif // DEVELOP

                // Run code to handle the message
                unique_ptr<RunningFlag> workRunning{new RunningFlag{true}};
                thread work_t{[this, clientId](RunningFlag *const workRunning_p, string buffer)
                              {
                                  // Mark Thread as running
                                  Server_running_manager running_mgr{*workRunning_p};

                                  // Run code to handle the incoming message
                                  if (workOnMessage)
                                      workOnMessage(clientId, move(buffer));

                                  return;
                              },
                              workRunning.get(), move(buffer)};

                // Remove all finished work handlers from the vector
                size_t workHandlers_s{workHandlersRunning.size()};
                for (size_t i{0}; i < workHandlers_s; i += 1)
                {
                    if (!*workHandlersRunning[i].get())
                    {
                        workHandlers[i].join();
                        workHandlers.erase(workHandlers.begin() + i);
                        workHandlersRunning.erase(workHandlersRunning.begin() + i);
                        i -= 1;
                        workHandlers_s -= 1;
                    }
                }

                workHandlers.push_back(move(work_t));
                workHandlersRunning.push_back(move(workRunning));
            }
            buffer += msg;
        }

        // If stream shall be forwarded to continuous out stream ...
        else
        {
            // Just forward incoming message to output stream
            if (forwardStreams.find(clientId) != forwardStreams.end())
                *forwardStreams[clientId].get() << msg << flush;
        }
    }
}

template <class SocketType, class SocketDeleter>
bool Server<SocketType, SocketDeleter>::isRunning() const
{
    return running;
}
