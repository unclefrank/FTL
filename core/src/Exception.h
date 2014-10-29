/*
 * Copyright (C) 2007-2014 Frank Mertens.
 *
 * Use of this source is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef FLUX_EXCEPTION_H
#define FLUX_EXCEPTION_H

#include <exception>
#include <flux/String>

namespace flux {

/*! \brief Root class in the exception class hierarchy
 * \sa exceptions
 */
class Exception: public std::exception
{
public:
    ~Exception() throw() {}

    virtual String message() const = 0;

private:
    const char *what() const throw() {
        static String h;
        h = message();
        return h;
    }
};

inline String str(const Exception &ex) { return ex.message(); }

} // namespace flux

#endif // FLUX_EXCEPTION_H
