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
#include <chrono>
#include <thread>

#include "TcpClient.h"
#include "TlsClient.h"

// Define worker methods
void tcp_fragmented_workOnMessage(::std::string msg) { ::std::cout << "Fragmented message received from TCP server: " << msg << ::std::endl; }
::std::ofstream tcp_continuous_stream{"Message_Continuous_TCP_Server.txt"};
void tls_fragmented_workOnMessage(::std::string msg) { ::std::cout << "Fragmented message received from TLS server: " << msg << ::std::endl; }
::std::ofstream tls_continuous_stream{"Message_Continuous_TLS_Server.txt"};

int main()
{
    while (true)
    {
        // Create client instances
        ::tcp::TcpClient tcpClient_fragmented{'\n'}; // newline as delimiter
        ::tcp::TcpClient tcpClient_continuous;
        ::tcp::TlsClient tlsClient_fragmented{'\n'}; // newline as delimiter
        ::tcp::TlsClient tlsClient_continuous;

        // Link all worker methods and forwarding streams
        tcpClient_fragmented.setWorkOnMessage(tcp_fragmented_workOnMessage);
        tcpClient_fragmented.setForwardStream(&tcp_continuous_stream);
        tlsClient_fragmented.setWorkOnMessage(tls_fragmented_workOnMessage);
        tlsClient_fragmented.setForwardStream(&tls_continuous_stream);

        // Get user input
        ::std::cout << "What mode shall be used to send messages?" << ::std::endl
                    << "    c: Continuous mode" << ::std::endl
                    << "    f: Fragmented mode" << ::std::endl
                    << "    q: Quit" << ::std::endl;
        char decision;
        ::std::cin >> decision;

        // stop all clients
        tcpClient_fragmented.stop();
        tcpClient_continuous.stop();
        tlsClient_fragmented.stop();
        tlsClient_continuous.stop();

        // Start clients
        switch (decision)
        {
        case 'c':
        case 'C':
            tcpClient_continuous.start("localhost", 8082);
            tlsClient_continuous.start("localhost", 8084, "../keys/ca/ca_cert.pem", "../keys/client/client_cert.pem", "../keys/client/client_key.pem");
            this_thread::sleep_for(50ms); // Wait a short time for connection to be established
            tcpClient.sendMsg("Hello TCP server! - forwarding mode");
            tlsClient.sendMsg("Hello TLS server! - forwarding mode");
            break;

        case 'f':
        case 'F':
            tcpClient_fragmented.start("localhost", 8081);
            tlsClient_fragmented.start("localhost", 8083, "../keys/ca/ca_cert.pem", "../keys/client/client_cert.pem", "../keys/client/client_key.pem");
            this_thread::sleep_for(50ms); // Wait a short time for connection to be established
            tcpClient.sendMsg("Hello TCP server! - fragmented mode");
            tlsClient.sendMsg("Hello TLS server! - fragmented mode");
            break;

        default:
            return 0;
        }
    }
}
