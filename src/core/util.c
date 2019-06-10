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
#include "util.h"
#include "ftuple.h"
#include "fstring.h"

/* Computes hash of an array of values */
int32_t fenn_array_calchash(const FennObject *array, int32_t len) {
    const FennObject *end = array + len;
    uint32_t hash = 5381;
    while (array < end)
        hash = (hash << 5) + hash + fenn_hash(*array++);
    return (int32_t) hash;
}

int32_t fenn_string_calchash(const uint8_t *str, int32_t len) {
    const uint8_t *end = str + len;
    uint32_t hash = 5381;
    while (str < end)
        hash = (hash << 5) + hash + *str++;
    return (int32_t) hash;
}

/* Check if two values are equal. This is strict equality with no conversion. */
int fenn_equals(FennObject x, FennObject y) {
    int result = 0;
    if (fenn_type(x) != fenn_type(y)) {
        result = 0;
    } else {
        switch (fenn_type(x)) {
            case FENN_NIL:
                result = 1;
                break;
            case FENN_BOOL:
                result = (fenn_unwrap_boolean(x) == fenn_unwrap_boolean(y));
                break;
            case FENN_NUMBER:
                result = (fenn_unwrap_number(x) == fenn_unwrap_number(y));
                break;
            case FENN_STRING:
                result = fenn_string_equal(fenn_unwrap_string(x), fenn_unwrap_string(y));
                break;
            case FENN_TUPLE:
                result = fenn_tuple_equal(fenn_unwrap_tuple(x), fenn_unwrap_tuple(y));
                break;
            case FENN_STRUCT:
                // TODO: Compare structs
                break;
            default:
                /* compare pointers */
                result = (fenn_unwrap_pointer(x) == fenn_unwrap_pointer(y));
                break;
        }
    }
    return result;
}

/* Computes a hash value for a function */
int32_t fenn_hash(FennObject x) {
    int32_t hash = 0;
    switch (fenn_type(x)) {
        case FENN_NIL:
            hash = 0;
            break;
        case FENN_BOOL:
            hash = fenn_unwrap_boolean(x);
            break;
        case FENN_STRING:
        case FENN_SYMBOL:
        case FENN_KEYWORD:
            hash= fenn_string_hash(fenn_unwrap_string(x));
            break;
        case FENN_TUPLE:
            hash = fenn_tuple_hash(fenn_unwrap_tuple(x));
            break;
        case FENN_STRUCT:
            // Struct hash
            break;
        default:
            if (sizeof(double) == sizeof(void *)) {
                /* Assuming 8 byte pointer */
                uint64_t i = fenn_u64(x);
                hash = (int32_t)(i & 0xFFFFFFFF);
                /* Get a bit more entropy by shifting the low bits out */
                hash >>= 3;
                hash ^= (int32_t)(i >> 32);
            } else {
                /* Assuming 4 byte pointer (or smaller) */
                hash = (int32_t)((char *)fenn_unwrap_pointer(x) - (char *)0);
                hash >>= 2;
            }
            break;
    }
    return hash;
}

/* Compares x to y. If they are equal returns 0. If x is less, returns -1.
 * If y is less, returns 1. All types are comparable
 * and should have strict ordering. */
int fenn_compare(FennObject x, FennObject y) {
    if (fenn_type(x) == fenn_type(y)) {
        switch (fenn_type(x)) {
            case FENN_NIL:
                return 0;
            case FENN_BOOL:
                return fenn_unwrap_boolean(x) - fenn_unwrap_boolean(y);
            case FENN_NUMBER:
                // Check for NaNs to ensure total order
                if (fenn_unwrap_number(x) != fenn_unwrap_number(x))
                    return fenn_unwrap_number(y) != fenn_unwrap_number(y)
                           ? 0
                           : -1;
                if (fenn_unwrap_number(y) != fenn_unwrap_number(y))
                    return 1;

                if (fenn_unwrap_number(x) == fenn_unwrap_number(y)) {
                    return 0;
                } else {
                    return fenn_unwrap_number(x) > fenn_unwrap_number(y) ? 1 : -1;
                }
            case FENN_STRING:
            case FENN_SYMBOL:
            case FENN_KEYWORD:
                return fenn_string_compare(fenn_unwrap_string(x), fenn_unwrap_string(y));
            case FENN_TUPLE:
                return fenn_tuple_compare(fenn_unwrap_tuple(x), fenn_unwrap_tuple(y));
            case FENN_STRUCT:
                // Compare structs
            default:
                if (fenn_unwrap_string(x) == fenn_unwrap_string(y)) {
                    return 0;
                } else {
                    return fenn_unwrap_string(x) > fenn_unwrap_string(y) ? 1 : -1;
                }
        }
    }
    return (fenn_type(x) < fenn_type(y)) ? -1 : 1;
}

FennObject fenn_from_cpointer(const void *p, uint64_t tagmask) {
    FennObject ret;
    ret.ptr = (void *)p;
    ret.u64 |= tagmask;
    return ret;
}

FennObject fenn_from_pointer(void *p, uint64_t tagmask) {
    FennObject ret;
    ret.ptr = p;
    ret.u64 |= tagmask;
    return ret;
}

void *fenn_to_pointer(FennObject x) {
    x.u64 &= FENN_PAYLOAD;
    return x.ptr;
}