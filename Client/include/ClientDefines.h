/**
 * @file ClientDefines.h
 * @author Nils Henrich
 * @brief Basic definitions for the network client
 * @version 2.0
 * @date 2021-12-28
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef CLIENTDEFINES_H_INCLUDED
#define CLIENTDEFINES_H_INCLUDED

namespace tcp
{
    enum : int
    {
        CLIENT_START_OK = 0,                     // Client started successfully
        CLIENT_ERROR_START_WRONG_PORT = 10,      // Client could not start because of wrong port number
        CLIENT_ERROR_START_SET_CONTEXT = 20,     // Client could not start because of SSL context error
        CLIENT_ERROR_START_WRONG_CA_PATH = 30,   // Client could not start because of wrong path to CA cert file
        CLIENT_ERROR_START_WRONG_CERT_PATH = 31, // Client could not start because of wrong path to certifcate file
        CLIENT_ERROR_START_WRONG_KEY_PATH = 32,  // Client could not start because of wrong path to key file
        CLIENT_ERROR_START_WRONG_CA = 33,        // Client could not start because of bad CA cert file
        CLIENT_ERROR_START_WRONG_CERT = 34,      // Client could not start because of bad certificate file
        CLIENT_ERROR_START_WRONG_KEY = 35,       // Client could not start because of bad key file or non matching key with certificate
        CLIENT_ERROR_START_CREATE_SOCKET = 40,   // Client could not start because of TCP socket creation error
        CLIENT_ERROR_START_SET_SOCKET_OPT = 41,  // Client could not start because of TCP socket options error
        CLIENT_ERROR_START_CONNECT = 50,         // Client could not start because of TCP socket connection error
        CLIENT_ERROR_START_CONNECT_INIT = 60,    // Client could not start because of an error while initializing the connection
    };
}

#endif // CLIENTDEFINES_H_INCLUDED
