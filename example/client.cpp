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

#include <tcp/TcpClient.hpp>
#include <tcp/TlsClient.hpp>

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
        ::tcp_serverclient::TcpClient tcpClient_fragmented{'\n'}; // newline as delimiter
        ::tcp_serverclient::TcpClient tcpClient_continuous{tcp_continuous_stream};
        ::tcp_serverclient::TlsClient tlsClient_fragmented{'\n'}; // newline as delimiter
        ::tcp_serverclient::TlsClient tlsClient_continuous{tls_continuous_stream};

        // Link all worker methods and forwarding streams
        tcpClient_fragmented.setWorkOnMessage(tcp_fragmented_workOnMessage);
        tlsClient_fragmented.setWorkOnMessage(tls_fragmented_workOnMessage);

        // Get user input
        ::std::cout << "What mode shall be used to send messages?" << ::std::endl
                    << "    c: Continuous mode" << ::std::endl
                    << "    f: Fragmented mode" << ::std::endl
                    << "    q: Quit" << ::std::endl;
        char decision;
        ::std::cin >> decision;

        // Start clients
        switch (decision)
        {
        case 'c':
        case 'C':
        {
            int startTcp = tcpClient_continuous.start("localhost", 8082);
            int startTls = tlsClient_continuous.start("localhost", 8084, "../keys/ca/ca_cert.pem", "../keys/client/client_cert.pem", "../keys/client/client_key.pem");
            if (startTcp != 0 || startTls != 0)
            {
                ::std::cout << "Error starting continuous clients." << ::std::endl
                            << "TCP client (continuous) returned " << startTcp << ::std::endl
                            << "TLS client (continuous) returned " << startTls << ::std::endl;
                return -1;
            }
            ::std::this_thread::sleep_for(::std::chrono::duration<double, ::std::milli>(50)); // Wait a short time for connection to be established
            tcpClient_continuous.sendMsg("Hello TCP server! - continuous mode");
            tlsClient_continuous.sendMsg("Hello TLS server! - continuous mode");
            break;
        }

        case 'f':
        case 'F':
        {
            int startTcp = tcpClient_fragmented.start("localhost", 8081);
            int startTls = tlsClient_fragmented.start("localhost", 8083, "../keys/ca/ca_cert.pem", "../keys/client/client_cert.pem", "../keys/client/client_key.pem");
            if (startTcp != 0 || startTls != 0)
            {
                ::std::cout << "Error starting fragmented clients." << ::std::endl
                            << "TCP client (fragmented) returned " << startTcp << ::std::endl
                            << "TLS client (fragmented) returned " << startTls << ::std::endl;
                return -1;
            }
            ::std::this_thread::sleep_for(::std::chrono::duration<double, ::std::milli>(50)); // Wait a short time for connection to be established
            tcpClient_fragmented.sendMsg("Hello TCP server! - fragmented mode");
            tlsClient_fragmented.sendMsg("Hello TLS server! - fragmented mode");
            break;
        }

        default:
            return 0;
        }
    }
}
