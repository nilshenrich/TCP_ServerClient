/**
 * @file algorithms.hpp
 * @author Nils Henrich
 * @brief Collection of basic algorithms used in the project
 * @version 3.0.0
 * @date 2021-12-27
 *
 * @copyright Copyright (c) 2024
 */

#ifndef ALGORITHMS_HPP_
#define ALGORITHMS_HPP_

#include <string>
#include <netinet/in.h>

namespace tcp::algorithms
{
    /**
     * @brief Replace all occurrences of a character in a string with another character
     *        in-place
     * @param str The string to replace in
     * @param c The character to replace
     * @param r The character to replace with
     */
    void replace_allC(::std::string &str, char c, char r)
    {
        for (char &ch : str)
        {
            if (ch == c)
            {
                ch = r;
            }
        }
    }

    /**
     * @brief Get a free TCP port in the given range. Including the range limits.
     *        Return -1 if no free port was found.
     *
     * @param range_min Minimum port number (inclusive)
     * @param range_max Maximum port number (inclusive)
     * @return int
     */
    // TODO: Raise exception if no port is found
    int getFreePort(int range_min, int range_max)
    {
        // First get random number inside port range
        // Then check if port is in use
        //     -> If not, use it
        //     -> If yes, try next number

        int port{rand() % (range_max - range_min) + range_min};
        for (int i{0}; i <= range_max - range_min; i += 1)
        {
            port += 1;
            if (port > range_max)
                port = range_min;

            int sock{socket(AF_INET, SOCK_STREAM, 0)};
            if (-1 == sock)
                return -1;

            struct sockaddr_in sin;
            sin.sin_family = AF_INET;
            sin.sin_addr.s_addr = INADDR_ANY;
            sin.sin_port = htons(port);

            if (!bind(sock, (struct sockaddr *)&sin, sizeof(sin)))
            {
                close(sock);
                return port;
            }
        }

        // If we get here, no free port was found. Return -1.
        return -1;
    }

} // namespace tcp::algorithms

#endif // ALGORITHMS_HPP_
