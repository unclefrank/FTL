/*
 * Copyright (C) 2007-2015 Frank Mertens.
 *
 * Use of this source is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include <math.h>
#include <flux/List>
#include <flux/Unicode>
#include <flux/Utf8Source>
#include <flux/Utf8Sink>
#include <flux/Utf16Source>
#include <flux/Utf16Sink>
#include <flux/Format>
#include <flux/ByteArray>

namespace flux {

Ref<ByteArray> ByteArray::create(int size)
{
    if (size <= 0) return new ByteArray();
    char *data = new char[size + 1];
    return new ByteArray(data, size);
}

class RawByteArray: public ByteArray
{
public:
    RawByteArray(char *data, int size):
        ByteArray(data, size)
    {}
    bool isZeroTerminated() const { return false; }
};

Ref<ByteArray> ByteArray::allocate(int size)
{
    if (size <= 0) return new ByteArray();
    char *data = new char[size];
    return new RawByteArray(data, size);
}

Ref<ByteArray> ByteArray::copy(const char *data, int size)
{
    if (!data) return new ByteArray();
    if (size < 0) size = strlen(data);
    if (size <= 0) return new ByteArray();
    char *newData = new char[size + 1];
    newData[size] = 0;
    memcpy(newData, data, size);
    return new ByteArray(newData, size);
}

Ref<ByteArray> ByteArray::join(const StringList *parts, const char *sep)
{
    if (parts->count() == 0)
        return ByteArray::create();

    int sepSize = strlen(sep);
    int size = 0;
    for (int i = 0; i < parts->count(); ++i)
        size += parts->at(i)->count();
    size += (parts->count() - 1) * sepSize;
    Ref<ByteArray> result = ByteArray::create(size);
    char *p = result->data_;
    for (int i = 0; i < parts->count(); ++i) {
        ByteArray *part = parts->at(i);
        memcpy(p, part->data_, part->size_);
        p += part->size_;
        if (i + 1 < parts->count()) {
            memcpy(p, sep, sepSize);
            p += sepSize;
        }
    }
    FLUX_ASSERT(p == result->data_ + result->size_);
    return result;
}

Ref<ByteArray> ByteArray::join(const StringList *parts, char sep)
{
    char h[2];
    h[0] = sep;
    h[1] = 0;
    return join(parts, h);
}

Ref<ByteArray> ByteArray::join(const StringList *parts, String sep)
{
    return join(parts, sep->chars());
}

ByteArray::ByteArray():
    size_(0),
    data_(const_cast<char *>("")),
    destroy_(doNothing)
{}

ByteArray::ByteArray(const char *data, int size, Destroy destroy):
    size_(0),
    data_(const_cast<char *>("")),
    destroy_(doNothing)
{
    if (!data) return;
    if (size < 0) size = strlen(data);
    if (size > 0) {
        size_ = size;
        data_ = const_cast<char *>(data);
        destroy_ = destroy;
    }
}

ByteArray::ByteArray(ByteArray *parent, int i0, int i1):
    destroy_(doNothing),
    parent_(parent)
{
    if (i0 < 0) i0 = 0;
    else if (i0 > parent->size_) i0 = parent_->size_;
    if (i1 < i0) i1 = i0;
    else if (i1 > parent->size_) i1 = parent_->size_;
    size_ = i1 - i0;
    data_ = parent_->data_ + i0;
}

ByteArray::ByteArray(const ByteArray &b):
    size_(0),
    data_(const_cast<char *>("")),
    destroy_(doNothing)
{
    if (b.size_ > 0) {
        size_ = b.size_;
        data_ = new char[b.size_ + 1];
        destroy_ = 0;
        memcpy(data_, b.data_, b.size_);
        data_[size_] = 0;
    }
}

ByteArray::~ByteArray()
{
    destroy();
}

void ByteArray::doNothing(ByteArray *)
{}

void ByteArray::destroy()
{
    if (destroy_) destroy_(this);
    else delete[] data_;
}

ByteArray *ByteArray::clear(char zero)
{
    memset(data_, zero, size_);
    return this;
}

ByteArray *ByteArray::truncate(int newSize)
{
    if (newSize < size_) {
        if (newSize < 0) newSize = 0;
        if (newSize > size_) newSize = size_;
        size_ = newSize;
        data_[size_] = 0;
    }
    return this;
}

ByteArray *ByteArray::resize(int newSize)
{
    if (newSize <= size_) return truncate(newSize);

    if (newSize > 0) {
        char *newData = new char[newSize + 1];
        memcpy(newData, data_, size_);
        newData[newSize] = 0;
        destroy();
        size_ = newSize;
        data_ = newData;
        destroy_ = 0;
    }
    else {
        size_ = 0;
        data_ = const_cast<char *>("");
        destroy_ = doNothing;
    }

    return this;
}

ByteArray &ByteArray::operator=(const ByteArray &b)
{
    int n = (size_ < b.size_) ? size_ : b.size_;
    memcpy(data_, b.data_, n);
    return *this;
}

ByteArray &ByteArray::operator^=(const ByteArray &b)
{
    int n = (size_ < b.size_) ? size_ : b.size_;
    for (int i = 0; i < n; ++i)
        bytes_[i] ^= b.bytes_[i];
    return *this;
}

Ref<ByteArray> ByteArray::copy(int i0, int i1) const
{
    if (i0 < 0) i0 = 0;
    if (i0 > size_) i0 = size_;
    if (i1 < 0) i1 = 0;
    if (i1 > size_) i1 = size_;
    int newSize = i1 - i0;
    if (newSize <= 0) return create();
    char *newData = new char[newSize + 1];
    memcpy(newData, data_ + i0, newSize);
    newData[newSize] = 0;
    return new ByteArray(newData, newSize);
}

Ref<ByteArray> ByteArray::paste(int i0, int i1, String text) const
{
    Ref<StringList> parts = StringList::create();
    parts->append(copy(0, i0));
    parts->append(text);
    parts->append(copy(i1, size_));
    return parts->join();
}

int ByteArray::countCharsIn(const char *set)
{
    int n = 0;
    for (const char *p = data_; *p; ++p) {
        for (const char *s = set; *s; ++s)
            n += (*p == *s);
    }
    return n;
}

bool ByteArray::startsWith(const char *s) const
{
    for (int i = 0; i < size_; ++i) {
        if (chars_[i] != s[i])
            return false;
    }
    return true;
}

bool ByteArray::endsWith(const char *s) const
{
    int n = strlen(s);
    if (n > size_) return false;
    for (int i = size_ - n, j = 0; i < size_; ++i, ++j) {
        if (chars_[i] != s[j])
            return false;
    }
    return true;
}

int ByteArray::find(const char *pattern, int i) const
{
    if (!has(i)) return size_;
    if (!pattern[0]) return size_;
    for (int j = i, k = 0; j < size_;) {
        if (data_[j++] == pattern[k]) {
            ++k;
            if (!pattern[k])
                return j - k;
        }
        else {
            k = 0;
        }
    }
    return size_;
}

int ByteArray::find(String pattern, int i) const
{
    return find(pattern->chars(), i);
}

bool ByteArray::contains(String pattern) const
{
    return contains(pattern->chars());
}

Ref<StringList> ByteArray::split(char sep) const
{
    Ref<StringList> parts = StringList::create();
    for (int i = 0; i < size_;) {
        int j = find(sep, i);
        parts->append(copy(i, j));
        i = j + 1;
    }
    return parts;
}

Ref<StringList> ByteArray::split(const char *sep) const
{
    Ref<StringList> parts = StringList::create();
    int i0 = 0;
    int sepLength = strlen(sep);
    while (i0 < size_) {
        int i1 = find(sep, i0);
        if (i1 == size_) break;
        parts->append(copy(i0, i1));
        i0 = i1 + sepLength;
    }
    if (i0 < size_)
        parts->append(copy(i0, size_));
    else
        parts->append(String());
    return parts;
}

Ref<StringList> ByteArray::breakUp(int chunkSize) const
{
    Ref<StringList> parts = StringList::create();
    int i0 = 0;
    while (i0 < size_) {
        int i1 = i0 + chunkSize;
        if (i1 > size_) i1 = size_;
        parts->append(copy(i0, i1));
        i0 += chunkSize;
    }
    return parts;
}

ByteArray *ByteArray::replaceInsitu(const char *pattern, const char *replacement)
{
    int patternLength = strlen(pattern);
    int replacementLength = strlen(replacement);
    if (patternLength < replacementLength) {
        *this = *replace(pattern, replacement);
    }
    else if (patternLength > 0) {
        int i = 0, j = 0, k = 0, n = size_;
        while (i < n) {
            char ch = data_[i++];
            if (j < i) data_[j++] = ch;
            if (ch == pattern[k]) {
                ++k;
                if (k == patternLength) {
                    j -= patternLength;
                    for (k = 0; k < replacementLength; ++k)
                        data_[j++] = replacement[k];
                    k = 0;
                }
            }
            else {
                k = 0;
            }
        }
        truncate(j);
    }
    return this;
}

Ref<ByteArray> ByteArray::replace(const char *pattern, const char *replacement) const
{
    return join(split(pattern), replacement);
}

Ref<ByteArray> ByteArray::replace(const char *pattern, String replacement) const
{
    return replace(pattern, replacement->chars());
}

Ref<ByteArray> ByteArray::replace(String pattern, String replacement) const
{
    return replace(pattern->chars(), replacement->chars());
}

int ByteArray::scanString(String *x, const char *termination, int i0, int i1) const
{
    if (i1 < 0 || i1 > size_) i1 = size_;
    if (i0 > i1) i0 = i1;
    int i = i0;
    for (; i < i1; ++i) {
        const char *p = termination;
        if (!at(i)) break;
        for(; *p; ++p) {
            if (at(i) == *p) break;
        }
        if (*p) break;
    }
    *x = copy(i0, i);
    return i;
}

ByteArray *ByteArray::downcaseInsitu()
{
    for (int i = 0; i < size_; ++i)
        chars_[i] = flux::downcase(chars_[i]);
    return this;
}

ByteArray *ByteArray::upcaseInsitu()
{
    for (int i = 0; i < size_; ++i)
        data_[i] = flux::upcase(data_[i]);
    return this;
}

Ref<ByteArray> ByteArray::escape() const
{
    Ref<StringList> parts;
    int i = 0, i0 = 0;
    for (; i < size_; ++i) {
        char ch = chars_[i];
        if (ch < 32 && ch >= 0) {
            if (!parts) parts = StringList::create();
            if (i0 < i) parts->append(copy(i0, i));
            i0 = i + 1;
            if (ch == 0x08) parts->append("\\b");
            else if (ch == 0x09) parts->append("\\t");
            else if (ch == 0x0A) parts->append("\\n");
            else if (ch == 0x0D) parts->append("\\r");
            else if (ch == 0x0C) parts->append("\\f");
            else {
                String s = "\\u00XX";
                const char *hex = "0123456789ABCDEF";
                s->at(s->count() - 2) = hex[ch / 16];
                s->at(s->count() - 1) = hex[ch % 16];
                parts->append(s);
            }
        }
    }
    if (!parts) return const_cast<ByteArray *>(this);

    if (i0 < i) parts->append(copy(i0, i));

    return parts->join();
}

ByteArray *ByteArray::unescapeInsitu()
{
    if (!contains('\\')) return this;
    int j = 0;
    uint32_t hs = 0; // high surrogate, saved
    Ref<ByteArray> ec; // buffer for encoded character
    for (int i = 0, n = size_; i < n;) {
        char ch = data_[i++];
        if ((ch == '\\') && (i < n)) {
            ch = data_[i++];
            if ((ch == 'u') && (i <= n - 4)) {
                uint32_t x = 0;
                for (int k = 0; k < 4; ++k) {
                    int digit = data_[i++];
                    if (('0' <= digit) && (digit <= '9')) digit -= '0';
                    else if (('a' <= digit) && (digit <= 'f')) digit = digit - 'a' + 10;
                    else if (('A' <= digit) && (digit <= 'F')) digit = digit - 'A' + 10;
                    x = (x * 16) + digit;
                }
                if ((0xB800 <= x) && (x < 0xDC00)) {
                    // save the high surrogate, do not output anything
                    hs = x;
                }
                else {
                    if ((0xDC00 <= x) && (x < 0xE000) && (hs != 0)) {
                        // combine the high and low surrogate
                        x = ((hs - 0xD800) << 10) | (x - 0xDC00);
                        x += 0x10000;
                        hs = 0;
                    }
                    if (!ec) ec = ByteArray::create(4);
                    Ref<Utf8Sink> sink = Utf8Sink::open(ec);
                    sink->write(x);
                    int el = utf8::encodedSize(x);
                    for (int k = 0; k < el; ++k)
                        data_[j++] = ec->at(k);
                }
            }
            else if ((ch == 'x') && (i <= n - 2)) {
                uint8_t x = 0;
                for (int k = 0; k < 2; ++k) {
                    int digit = data_[i++];
                    if (('0' <= digit) && (digit <= '9')) digit -= '0';
                    else if (('a' <= digit) && (digit <= 'f')) digit = digit - 'a' + 10;
                    else if (('A' <= digit) && (digit <= 'F')) digit = digit - 'A' + 10;
                    x = (x * 16) + digit;
                }
                data_[j++] = (char)x;
            }
            else {
                hs = 0;
                if (ch == 'b') data_[j++] = 0x08;
                else if (ch == 't') data_[j++] = 0x09;
                else if (ch == 'n') data_[j++] = 0x0A;
                else if (ch == 'r') data_[j++] = 0x0D;
                else if (ch == 'f') data_[j++] = 0x0C;
                else if (ch == 's') data_[j++] = 0x20;
                else data_[j++] = ch;
            }
        }
        else if (j < i) {
            hs = 0;
            data_[j++] = ch;
        }
        else {
            hs = 0;
            ++j;
        }
    }
    return truncate(j);
}

ByteArray *ByteArray::trimInsitu(const char *leadingSpace, const char *trailingSpace)
{
    if (!trailingSpace) trailingSpace = leadingSpace;
    int i0 = 0, i1 = size_;
    while (i0 < i1) {
        const char *p = leadingSpace;
        for (; *p; ++p)
            if (data_[i0] == *p) break;
        if (!*p) break;
        ++i0;
    }
    while (i0 < i1) {
        const char *p = trailingSpace;
        for (; *p; ++p)
            if (data_[i1 - 1] == *p) break;
        if (!*p) break;
        --i1;
    }
    if (i0 > 0 && i0 < i1) memmove(data_, data_ + i0, i1 - i0);
    return truncate(i1 - i0);
}

ByteArray *ByteArray::simplifyInsitu(const char *space)
{
    int j = 0;
    for (int i = 0, s = 0; i < size_; ++i) {
        const char *p = space;
        for (; *p; ++p)
            if (data_[i] == *p) break;
        s = (*p) ? s + 1 : 0;
        data_[j] = (*p) ? ' ' : data_[i];
        j += (s < 2);
    }
    truncate(j);
    return trimInsitu(space);
}

Ref<ByteArray> ByteArray::normalize(bool nameCase) const
{
    for (int i = 0; i < size_; ++i) {
        if ((0 <= data_[i]) && (data_[i] < 32))
            data_[i] = 32;
    }
    Ref<StringList> parts = split(" ");
    for (int i = 0; i < parts->count(); ++i) {
        String s = parts->at(i);
        if (s->count() == 0) {
            parts->remove(i);
        }
        else {
            if (nameCase) {
                s = s->downcase();
                s->at(0) = flux::upcase(s->at(0));
                parts->at(i) = s;
            }
            ++i;
        }
    }
    return join(parts, " ");
}

Ref<ByteArray> ByteArray::stripTags() const
{
    Ref<StringList> parts = StringList::create();
    int i = 0, j = 0;
    while (i < size_) {
        char ch = data_[i];
        if (ch == '<') {
            if (j < i) parts->append(copy(j, i));
            for (; i < size_; ++i) if (data_[i] == '>') break;
            i += (i != size_);
            j = i;
        }
        else if (ch == '&') {
            if (j < i) parts->append(copy(j, i));
            for (; i < size_; ++i) if (data_[i] == ';') break;
            i += (i != size_);
            j = i;
        }
        else {
            ++i;
        }
    }
    if (j < i) parts->append(copy(j, i));
    return join(parts);
}

/** \brief Map a byte offset to editor coordinates.
  * \arg offset byte offset
  * \arg line n-th line starting with 1
  * \arg pos position on line starting with 0 (in bytes)
  * \return true if offset is within valid range
  */
bool ByteArray::offsetToLinePos(int offset, int *line, int *pos) const
{
    bool valid = true;
    if (offset < 0) {
        valid = false;
        offset = 0;
    }
    if (count() <= offset) {
        valid = false;
        offset = count();
    }
    int y = 1, x = 0;
    for (int i = 0; i < offset; ++i) {
        if (data_[i] == '\n') {
            ++y;
            x = 0;
        }
        else {
            ++x;
        }
    }
    if (line) *line = y;
    if (pos) *pos = x;
    return valid;
}

/** \brief Map editor coordinates to a byte offset
  * \arg line n-th line starting with 1
  * \arg pos position on line starting with 1 (in bytes)
  * \arg offset byte offset
  * \return true if successful
  */
bool ByteArray::linePosToOffset(int line, int pos, int *offset) const
{
    if (line <= 0) return false;
    int i = 0;
    for (int y = 1; y < line && i < size_; ++i)
        if (data_[i] == '\n') ++y;
    if (i + pos >= size_)
        return false;
    if (offset) *offset = i + pos;
    return true;
}

Ref<ByteArray> ByteArray::fromUtf16(ByteArray *utf16, int endian)
{
    if (utf16->count() == 0) return ByteArray::create();

    Ref<ByteArray> out;
    {
        int n = 0;
        Ref<Utf16Source> source = Utf16Source::open(utf16, endian);
        for (uchar_t ch; source->read(&ch);)
            n += utf8::encodedSize(ch);
        out = ByteArray::create(n);
    }

    Ref<Utf16Source> source = Utf16Source::open(utf16, endian);
    Ref<Utf8Sink> sink = Utf8Sink::open(out);
    for (uchar_t ch; source->read(&ch);)
        sink->write(ch);

    return out;
}

/** Convert this string to UTF-16 efficiently (local endian).
  * Returns true if the given buffer was suitable to hold the encoded string.
  * The number of bytes required to fully represent the string in UTF-16 is
  * returned with the 'size' argument. Passing a zero for 'size' allows to
  * determine the required buffer size. No zero termination is written or
  * or accounted for.
  */
bool ByteArray::toUtf16(void *buf, int *size)
{
    uint16_t *buf2 = reinterpret_cast<uint16_t*>(buf);
    int j = 0, n = *size / 2;
    Ref<Unicode> chars = Unicode::open(this);
    for (int i = 0; i < chars->count(); ++i) {
        uchar_t ch = chars->at(i);
        if (ch < 0x10000) {
            if (j < n) buf2[j] = ch;
            ++j;
        }
        else if (ch <= 0x10FFFF) {
            if (j + 1 < n) {
                buf2[j] = (ch >> 10) + 0xB800;
                buf2[j + 1] = (ch & 0x3FF) + 0xBC00;
            }
            j += 2;
        }
        else {
            if (j < n) buf2[j] = 0xFFFD/*replacement character*/;
            ++j;
        }
    }
    *size = 2 * j;
    return (j <= n);
}

Ref<ByteArray> ByteArray::toUtf16(int endian)
{
    Ref<ByteArray> out;
    Ref<Unicode> chars = Unicode::open(this);
    {
        int n = 0;
        for (int i = 0; i < chars->count(); ++i)
            n += utf16::encodedSize(chars->at(i));
        out = ByteArray::create(n + 2);
        out->at(n) = 0;
        out->at(n + 1) = 0;
    }
    if (out->count() > 0) {
        Ref<Utf16Sink> sink = Utf16Sink::open(out, endian);
        for (int i = 0; i < chars->count(); ++i)
            sink->write(chars->at(i));
    }
    return out;
}

void ByteArray::checkUtf8() const
{
    Ref<Utf8Source> source = Utf8Source::open(const_cast<ByteArray *>(this));
    for (uchar_t ch = 0; source->read(&ch););
}

Ref<ByteArray> ByteArray::hex() const
{
    Ref<ByteArray> s2 = ByteArray::create(size_ * 2);
    int j = 0;
    for (int i = 0; i < size_; ++i) {
        unsigned char ch = bytes_[i];
        int d0 = (ch >> 4) & 0xf;
        int d1 = ch & 0xf;
        if ((0 <= d0) && (d0 < 10)) s2->data_[j++] = d0 + '0';
        else s2->data_[j++] = (d0 - 10) + 'a';
        if ((0 <= d1) && (d1 < 10)) s2->data_[j++] = d1 + '0';
        else s2->data_[j++] = (d1 - 10) + 'a';
    }
    return s2;
}

bool ByteArray::isRootPath() const
{
    return String(this) == "/";
}

bool ByteArray::isRelativePath() const
{
    return !isAbsolutePath();
}

bool ByteArray::isAbsolutePath() const
{
    return (count() > 0) ? (at(0) == '/') : false;
}

Ref<ByteArray> ByteArray::absolutePathRelativeTo(String currentDir) const
{
    if (isAbsolutePath() || (currentDir == "."))
        return const_cast<ByteArray *>(this);

    Ref<StringList> absoluteParts = StringList::create();
    Ref<StringList> parts = split("/");

    int upCount = 0;

    for (int i = 0; i < parts->count(); ++i)
    {
        String c = parts->at(i);
        if (c == ".")
        {}
        else if (c == "..") {
            if (absoluteParts->count() > 0)
                absoluteParts->popBack();
            else
                ++upCount;
        }
        else {
            absoluteParts->append(c);
        }
    }

    String prefix = currentDir->copy();

    while (upCount > 0) {
        prefix = prefix->reducePath();
        --upCount;
    }

    absoluteParts->pushFront(prefix);

    return absoluteParts->join("/");
}

Ref<ByteArray> ByteArray::fileName() const
{
    String name;
    Ref<StringList> parts = split("/");
    if (parts->count() > 0)
        name = parts->at(parts->count() - 1);
    return name;
}

Ref<ByteArray> ByteArray::baseName() const
{
    String name = fileName();
    if (!name->contains('.')) return name;
    Ref<StringList> parts = name->split(".");
    parts->pop(parts->count() - 1);
    return parts->join(".");
}

Ref<ByteArray>  ByteArray::fileSuffix() const
{
    Ref<StringList> parts = fileName()->split(".");
    return parts->at(parts->count() - 1);
}

Ref<ByteArray> ByteArray::reducePath() const
{
    Ref<StringList> parts = split("/");
    while (parts->count() > 0) {
        String component = parts->popBack();
        if (component != "") break;
    }
    String resultPath = parts->join("/");
    if (resultPath == "")
        resultPath = isAbsolutePath() ? "/" : ".";
    return resultPath;
}

Ref<ByteArray> ByteArray::expandPath(String relativePath) const
{
    const char *sep = (endsWith('/') || relativePath->startsWith('/')) ? "" : "/";
    return String(Format() << String(this) << sep << relativePath);
}

Ref<ByteArray> ByteArray::canonicalPath() const
{
    Ref<StringList> parts = split("/");
    Ref<StringList> result = StringList::create();
    for (int i = 0; i < parts->count(); ++i) {
        String part = parts->at(i);
        if (part == "" && i > 0) continue;
        if (part == "" && i == parts->count() - 1) continue;
        if (part == "." && parts->count() > 1) continue;
        if (part == ".." && result->count() > 0) {
            if (result->at(result->count() - 1) != "..") {
                result->popBack();
                continue;
            }
        }
        result->append(part);
    }
    return result->join("/");
}

bool ByteArray::equalsCaseInsensitive(ByteArray *b) const
{
    if (size_ != b->size_) return false;
    for (int i = 0; i < size_; ++i)
        if (flux::downcase(chars_[i]) != flux::downcase(b->chars_[i])) return false;
    return true;
}

bool ByteArray::equalsCaseInsensitive(const char *b) const
{
    int bSize = strlen(b);
    if (size_ != bSize) return false;
    for (int i = 0; i < size_; ++i)
        if (flux::downcase(chars_[i]) != flux::downcase(b[i])) return false;
    return true;
}

double ByteArray::pow(double x, double y) { return ::pow(x, y); }

} // namespace flux
