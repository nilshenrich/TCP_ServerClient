/**
 * @file NetworkingDefines.h
 * @author Nils Henrich
 * @brief Basic definitions for the network listener.
 * @version 1.0
 * @date 2021-12-27
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef TLSSERVERDEFINES_H_INCLUDED
#define TLSSERVERDEFINES_H_INCLUDED

namespace tcp
{
    enum : int
    {
        SERVER_START_OK = 0,                     // Listener started successfully
        SERVER_ERROR_START_WRONG_PORT = 10,      // Listener could not start because of wrong port number
        SERVER_ERROR_START_SET_CONTEXT = 20,     // Listener could not start because of SSL context error
        SERVER_ERROR_START_WRONG_CA_PATH = 30,   // Listener could not start because of wrong path to CA cert file
        SERVER_ERROR_START_WRONG_CERT_PATH = 31, // Listener could not start because of wrong path to certificate file
        SERVER_ERROR_START_WRONG_KEY_PATH = 32,  // Listener could not start because of wrong path to key file
        SERVER_ERROR_START_WRONG_CA = 33,        // Listener could not start because of bad CA cert file
        SERVER_ERROR_START_WRONG_CERT = 34,      // Listener could not start because of bad certificate file
        SERVER_ERROR_START_WRONG_KEY = 35,       // Listener could not start because of bad key file or non matching key with certificate
        SERVER_ERROR_START_CREATE_SOCKET = 40,   // Listener could not start because of TCP socket creation error
        SERVER_ERROR_START_SET_SOCKET_OPT = 41,  // Listener could not start because of TCP socket option error
        SERVER_ERROR_START_BIND_PORT = 42,       // Listener could not start because of TCP socket bind error
        SERVER_ERROR_START_LISTENER = 43         // Listener could not start because of TCP socket listen error
    };
}

#endif // TLSSERVERDEFINES_H_INCLUDED
