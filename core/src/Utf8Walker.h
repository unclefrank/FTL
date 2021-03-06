/*
 * Copyright (C) 2007-2015 Frank Mertens.
 *
 * Use of this source is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef FLUX_UTF8WALKER_H
#define FLUX_UTF8WALKER_H

#include <flux/types>

namespace flux {

/** \brief Iterating UTF-8 encoded strings
  *
  * The Utf8Walker allows iterating Unicode characters over any zero terminated byte sequence.
  * The walker will always halt at the string boundaries. If stepping over the string boundary
  * the walker will switch to valid() == false and start delivering zero characters.
  *
  * If placed at a string's terminating zero character it is possible to step backwards
  * into the string. In other words '(Utf8Walter(data, data + len) - 1).getChar()' is a valid
  * construct and will deliver the last Unicode character of the string.
  *
  * Illegal code sequences are overrun without error. In forward iteration corrupted code prefixes
  * may lead to at maximum one correct character overrun and up to 3 additional corrupt characters
  * delivered. Any bit error outside the code prefixes will lead to at most one broken character
  * delivered, with one exception: Switching to all bits zero. A zero byte is always string
  * terminating.
  *
  * A walker object equals in size the size of 2 pointers and can be passed by value.
  */
class Utf8Walker
{
public:
    // Initialize new walker
    // \arg data byte string
    // \arg pos position in string
    Utf8Walker(const char *data = 0, const char *pos = 0)
        : s_(reinterpret_cast<const uint8_t*>(data)),
          p_(reinterpret_cast<const uint8_t*>(pos))
    {
        if (!p_) p_ = (s_) ? s_ : beforeBegin();
    }

    Utf8Walker(const Utf8Walker &b)
        : s_(b.s_),
          p_(b.p_)
    {}

    // prefix increment
    inline Utf8Walker &operator++() {
        uint8_t ch = *p_;
        if (ch) {
            ++p_;
            // step forwards shifting out all prefix bits of first byte
            while ((0xBF < ch) && (*p_)) {
                ch <<= 1;
                ++p_;
            }
        }
        return *this;
    }

    // prefix decrement
    inline Utf8Walker &operator--() {
        bool canStepBack = (s_ < p_);
        // if at zero character figure out if before begin or at end or empty string
        if (!*p_) canStepBack = (p_ != s_) ? *(p_ - 1) : false;
        if (canStepBack) {
            --p_;
            // step backwards while code prefix equals (10)2
            while ((s_ < p_) && ((*p_ & 0xC0) == 0x80)) --p_;
        }
        else
            p_ = beforeBegin();
        return *this;
    }

    // postfix increment
    inline Utf8Walker operator++(int) {
        Utf8Walker it = *this;
        ++(*this);
        return it;
    }

    // postfix decrement
    inline Utf8Walker operator--(int) {
        Utf8Walker it = *this;
        --(*this);
        return it;
    }

    inline Utf8Walker &operator+=(int n) {
        while (n > 0) { ++(*this); --n; }
        while (n < 0) { --(*this); ++n; }
        return *this;
    }

    inline Utf8Walker &operator-=(int n) {
        while (n > 0) { --(*this); --n; }
        while (n < 0) { ++(*this); ++n; }
        return *this;
    }

    inline bool valid() const { return *p_; }
    inline operator bool() const { return *p_; }

    inline Utf8Walker operator+(int delta) const {
        Utf8Walker it = *this;
        return it += delta;
    }
    inline Utf8Walker operator-(int delta) const {
        Utf8Walker it = *this;
        return it -= delta;
    }

    // decode unicode point
    inline uint32_t getChar() const {
        // merging extra bytes by incrementally reducing the code prefix of the first byte
        // prefix bits => extra bytes: (110)2 => 1 eb, (1110)2 => 2 eb, (11110)2 => 3 eb
        uint32_t ch = *p_;
        if (0x7F < ch) {
            const uint8_t *p = p_;
            uint32_t bit = 0x80;
            ch ^= bit; // clear first prefix bit
            bit >>= 1; // take next prefix bit
            while (((ch & bit) != 0) && (*p)) { // yet another prefix bit?
                ch ^= bit; // clear prefix bit of first byte
                ch <<= 6; // make space
                bit <<= 5; // take next prefix bit
                ch |= (*(++p)) & 0x3F; // merge code bits
            }
        }
        return ch;
    }

    inline const char *data() const { return reinterpret_cast<const char*>(s_); }
    inline const char *pos() const { return reinterpret_cast<const char*>(p_); }

    inline bool operator==(const Utf8Walker &b) const { return (p_ == b.p_) && (s_ == b.s_); }
    inline bool operator!=(const Utf8Walker &b) const { return (p_ != b.p_) || (s_ != b.s_); }

    // distance in number of characters
    inline int operator-(const Utf8Walker &b) const {
        int n = 0;
        if (s_ == b.s_) {
            if (p_ <= b.p_) {
                Utf8Walker w(*this);
                while (w.p_ != b.p_) {
                    ++w;
                    --n;
                }
            }
            else
                n = -(b - *this);
        }
        return n;
    }

    // byte position in string
    inline operator int() const { return p_ - s_; }

    static int length(const char *s) {
        Utf8Walker walker(s);
        int n_ = 0;
        while (walker.valid()) { ++walker; ++n_; }
        return n_;
    }

private:
    static inline const uint8_t *beforeBegin() {
        return reinterpret_cast<const uint8_t*>("\0") + 1; // extra zero to signal before begin position
    }

    const uint8_t *s_; // zero-terminated byte array
    const uint8_t *p_; // position in byte array
};

} // namespace flux

#endif // FLUX_UTF8WALKER_H
