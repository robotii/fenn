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
#include "tuple.h"
#include "util.h"

FennObject *fenn_tuple_begin(int32_t length) {
    size_t size = sizeof(FennTupleHead) + (length * sizeof(FennObject));
    FennTupleHead *head = fenn_gcalloc(FENN_MEMORY_TUPLE, size);
    head->sm_start = -1;
    head->sm_end = -1;
    head->length = length;
    return (FennObject *)(head->data);
}

/* Finish building a tuple */
const FennObject *fenn_tuple_end(FennObject *tuple) {
    fenn_tuple_hash(tuple) = fenn_array_calchash(tuple, fenn_tuple_length(tuple));
    return (const FennObject *)tuple;
}

/* Build a tuple with n values */
const FennObject *fenn_tuple_n(const FennObject *values, int32_t n) {
    FennObject *t = fenn_tuple_begin(n);
    memcpy(t, values, sizeof(FennObject) * n);
    return fenn_tuple_end(t);
}

/* Check if two tuples are equal */
int fenn_tuple_equal(const FennObject *lhs, const FennObject *rhs) {
    int32_t index;
    int32_t llen = fenn_tuple_length(lhs);
    int32_t rlen = fenn_tuple_length(rhs);
    int32_t lhash = fenn_tuple_hash(lhs);
    int32_t rhash = fenn_tuple_hash(rhs);
    if (lhash == 0)
        lhash = fenn_tuple_hash(lhs) = fenn_array_calchash(lhs, llen);
    if (rhash == 0)
        rhash = fenn_tuple_hash(rhs) = fenn_array_calchash(rhs, rlen);
    if (lhash != rhash)
        return 0;
    if (llen != rlen)
        return 0;
    for (index = 0; index < llen; index++) {
        if (!fenn_equals(lhs[index], rhs[index]))
            return 0;
    }
    return 1;
}

/* Compare tuples */
int fenn_tuple_compare(const FennObject *lhs, const FennObject *rhs) {
    int32_t i;
    int32_t llen = fenn_tuple_length(lhs);
    int32_t rlen = fenn_tuple_length(rhs);
    int32_t count = llen < rlen ? llen : rlen;
    for (i = 0; i < count; ++i) {
        int comp = fenn_compare(lhs[i], rhs[i]);
        if (comp != 0) return comp;
    }
    if (llen < rlen)
        return -1;
    else if (llen > rlen)
        return 1;
    return 0;
}
