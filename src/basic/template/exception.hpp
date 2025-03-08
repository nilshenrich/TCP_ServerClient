/**
 * @file exceptions.hpp
 * @author Nils Henrich
 * @brief Basic exceptions for the project.
 *        - General exception (Exception): Used for general errors.
 * @version 3.2.1
 * @date 2025-02-25
 *
 * @copyright Copyright (c) 2025
 */

#ifndef EXCEPTIONS_HPP_
#define EXCEPTIONS_HPP_

#include <exception>
#include <string>

namespace tcp
{
    class Error : public ::std::exception
    {
    public:
        Error(::std::string msg = "unexpected error") : msg{msg} {}
        virtual ~Error() {}

        const char *what() const noexcept override
        {
            return msg.c_str();
        }

    private:
        const ::std::string msg;

        // Delete default constructor
        Error() = delete;

        // Disallow copy
        Error(const Error &) = delete;
        Error &operator=(const Error &) = delete;
    };
}

#endif // EXCEPTIONS_HPP_
