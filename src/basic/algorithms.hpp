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
} // namespace tcp::algorithms

#endif // ALGORITHMS_HPP_
