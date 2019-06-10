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

#ifndef STRING_H
#define STRING_H

typedef struct FennStringHead FennStringHead;

struct FennStringHead {
    FennGCObject gc;
    int32_t length;
    int32_t hash;
    const uint8_t data[];
};

#define fenn_string_head(s) ((FennStringHead *)((char *)s - offsetof(FennStringHead, data)))
#define fenn_string_length(s) (fenn_string_head(s)->length)
#define fenn_string_hash(s) (fenn_string_head(s)->hash)

uint8_t *fenn_string_begin(int32_t);
const uint8_t *fenn_string_end(uint8_t *);
const uint8_t *fenn_string(const uint8_t *, int32_t);
int fenn_string_compare(const uint8_t *, const uint8_t *);
int fenn_string_equalconst(const uint8_t *lhs, const uint8_t *, int32_t, int32_t);
int fenn_string_equal(const uint8_t *, const uint8_t *);
const uint8_t *fenn_cstring(const char *);

#endif
