/**
 * @file server.cpp
 * @author Nils Henrich
 * @brief Example how to use the server part
 * @version 1.0
 * @date 2024-04-20
 *
 * @copyright Copyright (c) 2024
 */

#include <iostream>
#include <fstream>

#include "TcpServer/TcpServer.h"
#include "TcpServer/TlsServer.h"

// Global server instances
::tcp::TcpServer tcpServer_fragmented{'\n'}; // newline as delimiter
::tcp::TcpServer tcpServer_continuous;
::tcp::TlsServer tlsServer_fragmented{'\n'}; // newline as delimiter
::tcp::TlsServer tlsServer_continuous;

// Define worker methods
void tcp_fragmented_workOnEstablished(int id) { tcpServer_fragmented.sendMsg(id, "Hello TCP client " + ::std::to_string(id) + " in fragmented mode!"); }
void tcp_continuous_workOnEstablished(int id) { tcpServer_continuous.sendMsg(id, "Hello TCP client " + ::std::to_string(id) + " in continuous mode!"); }
void tcp_fragmented_workOnMessage(int id, std::string msg) { ::std::cout << "Fragmented message received from TCP client " << id << ": " << msg << ::std::endl; }
::std::ofstream *tcp_continuous_createStream(int id)
{
    tcpServer_continuous.sendMsg(id, "Hello TCP client " + ::std::to_string(id) + " in continuous mode!");
    return new ::std::ofstream("Message_Continuous_TCP_Client-" + ::std::to_string(id) + ".txt");
}
void tcp_workOnClosed(int id) { ::std::cout << "TCP client " << id << " closed connection." << ::std::endl; }

void tls_fragmented_workOnEstablished(int id) { tlsServer_fragmented.sendMsg(id, "Hello TLS client " + ::std::to_string(id) + " in fragmented mode!"); }
void tls_continuous_workOnEstablished(int id) { tlsServer_continuous.sendMsg(id, "Hello TLS client " + ::std::to_string(id) + " in continuous mode!"); }
void tls_fragmented_workOnMessage(int id, std::string msg) { ::std::cout << "Fragmented message received from TLS client " << id << ": " << msg << ::std::endl; }
::std::ofstream *tls_continuous_createStream(int id)
{
    tlsServer_continuous.sendMsg(id, "Hello TLS client " + ::std::to_string(id) + " in continuous mode!");
    return new ::std::ofstream("Message_Continuous_TLS_Client-" + ::std::to_string(id) + ".txt");
}
void tls_workOnClosed(int id) { ::std::cout << "TLS client " << id << " closed connection." << ::std::endl; }

int main()
{
    // Link all worker methods
    tcpServer_fragmented.setWorkOnEstablished(tcp_fragmented_workOnEstablished);
    tcpServer_fragmented.setWorkOnMessage(tcp_fragmented_workOnMessage);
    tcpServer_fragmented.setWorkOnClosed(tcp_workOnClosed);

    tcpServer_continuous.setWorkOnEstablished(tcp_continuous_workOnEstablished);
    tcpServer_continuous.setCreateForwardStream(tcp_continuous_createStream);
    tcpServer_continuous.setWorkOnClosed(tcp_workOnClosed);

    tlsServer_fragmented.setWorkOnEstablished(tls_fragmented_workOnEstablished);
    tlsServer_fragmented.setWorkOnMessage(tls_fragmented_workOnMessage);
    tlsServer_fragmented.setWorkOnClosed(tcp_workOnClosed);

    tlsServer_continuous.setWorkOnEstablished(tls_continuous_workOnEstablished);
    tlsServer_continuous.setCreateForwardStream(tls_continuous_createStream);
    tlsServer_continuous.setWorkOnClosed(tcp_workOnClosed);

    // Start servers
    tcpServer_fragmented.start(8081);
    tcpServer_continuous.start(8082);
    tlsServer_fragmented.start(8083, "../keys/ca/ca_cert.pem", "../keys/server/server_cert.pem", "../keys/server/server_key.pem");
    tlsServer_continuous.start(8084, "../keys/ca/ca_cert.pem", "../keys/server/server_cert.pem", "../keys/server/server_key.pem");

    // Halt program
    ::std::cout << "Press any key to exit the program..." << ::std::endl;
    ::std::cin.get();

    return 0;
}