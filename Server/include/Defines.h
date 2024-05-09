/**
 * @file Defines.h
 * @author Nils Henrich
 * @brief Basic definitions for the server.
 * @version 1.0
 * @date 2021-12-27
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef SERVERDEFINES_H_INCLUDED
#define SERVERDEFINES_H_INCLUDED

namespace tcp
{
    enum : int
    {
        SERVER_START_OK = 0,                     // Server started successfully
        SERVER_ERROR_START_WRONG_PORT = 10,      // Server could not start because of wrong port number
        SERVER_ERROR_START_SET_CONTEXT = 20,     // Server could not start because of SSL context error
        SERVER_ERROR_START_WRONG_CA_PATH = 30,   // Server could not start because of wrong path to CA cert file
        SERVER_ERROR_START_WRONG_CERT_PATH = 31, // Server could not start because of wrong path to certificate file
        SERVER_ERROR_START_WRONG_KEY_PATH = 32,  // Server could not start because of wrong path to key file
        SERVER_ERROR_START_WRONG_CA = 33,        // Server could not start because of bad CA cert file
        SERVER_ERROR_START_WRONG_CERT = 34,      // Server could not start because of bad certificate file
        SERVER_ERROR_START_WRONG_KEY = 35,       // Server could not start because of bad key file or non matching key with certificate
        SERVER_ERROR_START_CREATE_SOCKET = 40,   // Server could not start because of TCP socket creation error
        SERVER_ERROR_START_SET_SOCKET_OPT = 41,  // Server could not start because of TCP socket option error
        SERVER_ERROR_START_BIND_PORT = 42,       // Server could not start because of TCP socket bind error
        SERVER_ERROR_START_SERVER = 43           // Server could not start because of TCP socket listen error
    };
}

#endif // SERVERDEFINES_H_INCLUDED
