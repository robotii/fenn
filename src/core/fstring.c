/*
* Copyright (c) 2019 Peter Arthur
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to
* deal in the Software without restriction, including without limitation the
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*/

#include <fenn.h>
#include <string.h>

#include "gc.h"
#include "util.h"

#include "fstring.h"

/* Begin building a string */
uint8_t *fenn_string_begin(int32_t length) {
    FennStringHead *head = fenn_gcalloc(FENN_MEMORY_STRING, sizeof(FennStringHead) + length + 1);
    head->length = length;
    uint8_t *data = (uint8_t *)head->data;
    data[length] = 0;
    return data;
}

/* Finish building a string */
const uint8_t *fenn_string_end(uint8_t *str) {
    fenn_string_hash(str) = fenn_string_calchash(str, fenn_string_length(str));
    return str;
}

/* Load a buffer as a string */
const uint8_t *fenn_string(const uint8_t *buf, int32_t len) {
    FennStringHead *head = fenn_gcalloc(FENN_MEMORY_STRING, sizeof(FennStringHead) + len + 1);
    head->length = len;
    head->hash = fenn_string_calchash(buf, len);
    uint8_t *data = (uint8_t *)head->data;
    memcpy(data, buf, len);
    data[len] = 0;
    return data;
}

/* Compare two strings */
int fenn_string_compare(const uint8_t *lhs, const uint8_t *rhs) {
    int32_t xlen = fenn_string_length(lhs);
    int32_t ylen = fenn_string_length(rhs);
    int32_t len = xlen > ylen ? ylen : xlen;
    int res = memcmp(lhs, rhs, len);
    if (res) return res;
    if (xlen == ylen) return 0;
    return xlen < ylen ? -1 : 1;
}

/* Compare a fenn string with a piece of memory */
int fenn_string_equalconst(const uint8_t *lhs, const uint8_t *rhs, int32_t rlen, int32_t rhash) {
    int32_t lhash = fenn_string_hash(lhs);
    int32_t llen = fenn_string_length(lhs);
    if (lhs == rhs)
        return 1;
    if (lhash != rhash || llen != rlen)
        return 0;
    return !memcmp(lhs, rhs, rlen);
}

/* Check if two strings are equal */
int fenn_string_equal(const uint8_t *lhs, const uint8_t *rhs) {
    return fenn_string_equalconst(lhs, rhs,
                                   fenn_string_length(rhs), fenn_string_hash(rhs));
}

/* Load a c string */
const uint8_t *fenn_cstring(const char *str) {
    return fenn_string((const uint8_t *)str, (int32_t)strlen(str));
}