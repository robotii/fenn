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

#ifndef FENN_H
#define FENN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <config.h>

/* Standard Libraries */
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>

#ifdef PLATFORM_WINDOWS
#define FENN_API __declspec(dllexport)
#else
#define FENN_API __attribute__((visibility ("default")))
#endif

#if defined(__GNUC__)
#define FENN_THREAD_LOCAL __thread
#elif defined(_MSC_BUILD)
#define FENN_THREAD_LOCAL __declspec(thread)
#else
#define FENN_THREAD_LOCAL
#endif


typedef union FennObject FennObject;

// Universal type for Fenn objects
union FennObject {
    uint64_t u64;
    int64_t i64;
    double num;
    void *ptr;
};

typedef enum FennType FennType;

/* Fenn basic data types */
enum FennType {
    FENN_NUMBER,
    FENN_NIL,
    FENN_BOOL,
    FENN_FIBER,
    FENN_STRING,
    FENN_SYMBOL,
    FENN_KEYWORD,
    FENN_ARRAY,
    FENN_TUPLE,
    FENN_TABLE,
    FENN_STRUCT,
    FENN_BUFFER,
    FENN_FUNCTION,
    FENN_CFUNCTION,
    FENN_ABSTRACT,
    FENN_POINTER
};

typedef struct FennGCObject FennGCObject;

struct FennGCObject {
    int32_t flags;
    FennGCObject *next;
};

#define fenn_u64(x) ((x).u64)
#define fenn_i64(x) ((x).i64)

#define FENN_TAGBITS 0xFFFF800000000000llu
#define FENN_PAYLOAD 0x00007FFFFFFFFFFFllu
#define fenn_lowtag(type) ((uint64_t)(type) | 0x1FFF0)
#define fenn_tag(type) (fenn_lowtag(type) << 47)
#define fenn_type(x) \
    (isnan((x).num) \
        ? (((x).u64 >> 47) & 0xF) \
        : FENN_NUMBER)

#define fenn_checkauxtype(x, type) \
    (((x).u64 & FENN_TAGBITS) == fenn_tag((type)))

#define fenn_isnumber(x) \
    (!isnan((x).num) || fenn_checkauxtype((x), FENN_NUMBER))

#define fenn_checktype(x, t) \
    (((t) == FENN_NUMBER) \
        ? fenn_isnumber(x) \
        : fenn_checkauxtype((x), (t)))

/* Conversion */
FENN_API void *fenn_to_pointer(FennObject);
FENN_API FennObject fenn_from_pointer(void *, uint64_t);
FENN_API FennObject fenn_from_cpointer(const void *, uint64_t);
FENN_API FennObject fenn_from_double(double);
FENN_API FennObject fenn_from_bits(uint64_t);

// All objects except nil and false are truthy
#define fenn_truthy(x) \
    (!jfenn_checktype((x), FENN_NIL) && \
     (!fenn_checktype((x), FENN_BOOL) || ((x).u64 & 0x1)))

#define fenn_from_payload(t, p) \
    fenn_from_bits(fenn_tag(t) | (p))

#define fenn_wrap_(p, t) \
    fenn_from_pointer((p), fenn_tag(t))

#define fenn_wrap_c(p, t) \
    fenn_from_cpointer((p), fenn_tag(t))

/* Wrap the simple types */
#define fenn_wrap_nil() fenn_from_payload(FENN_NIL, 1)
#define fenn_wrap_true() fenn_from_payload(FENN_BOOL, 1)
#define fenn_wrap_false() fenn_from_payload(FENN_BOOL, 0)
#define fenn_wrap_bool(b) fenn_from_payload(FENN_BOOL, !!(b))
#define fenn_wrap_number(r) fenn_from_double(r)

/* Unwrap the simple types */
#define fenn_unwrap_boolean(x) ((x).u64 & 0x1)
#define fenn_unwrap_number(x) ((x).num)

/* Wrap the pointer types */
#define fenn_wrap_struct(s) fenn_wrap_c((s), FENN_STRUCT)
#define fenn_wrap_tuple(s) fenn_wrap_c((s), FENN_TUPLE)
#define fenn_wrap_fiber(s) fenn_wrap_((s), FENN_FIBER)
#define fenn_wrap_array(s) fenn_wrap_((s), FENN_ARRAY)
#define fenn_wrap_table(s) fenn_wrap_((s), FENN_TABLE)
#define fenn_wrap_buffer(s) fenn_wrap_((s), FENN_BUFFER)
#define fenn_wrap_string(s) fenn_wrap_c((s), FENN_STRING)
#define fenn_wrap_symbol(s) fenn_wrap_c((s), FENN_SYMBOL)
#define fenn_wrap_keyword(s) fenn_wrap_c((s), FENN_KEYWORD)
#define fenn_wrap_abstract(s) fenn_wrap_((s), FENN_ABSTRACT)
#define fenn_wrap_function(s) fenn_wrap_((s), FENN_FUNCTION)
#define fenn_wrap_cfunction(s) fenn_wrap_((s), FENN_CFUNCTION)
#define fenn_wrap_pointer(s) fenn_wrap_((s), FENN_POINTER)

/* Unwrap the pointer types */
#define fenn_unwrap_struct(x) ((const FennKV *)fenn_to_pointer(x))
#define fenn_unwrap_tuple(x) ((const FennObject *)fenn_to_pointer(x))
#define fenn_unwrap_fiber(x) ((FennFiber *)fenn_to_pointer(x))
#define fenn_unwrap_array(x) ((FennArray *)fenn_to_pointer(x))
#define fenn_unwrap_table(x) ((FennTable *)fenn_to_pointer(x))
#define fenn_unwrap_buffer(x) ((FennBuffer *)fenn_to_pointer(x))
#define fenn_unwrap_string(x) ((const uint8_t *)fenn_to_pointer(x))
#define fenn_unwrap_symbol(x) ((const uint8_t *)fenn_to_pointer(x))
#define fenn_unwrap_keyword(x) ((const uint8_t *)fenn_to_pointer(x))
#define fenn_unwrap_abstract(x) (fenn_to_pointer(x))
#define fenn_unwrap_pointer(x) (fenn_to_pointer(x))
#define fenn_unwrap_function(x) ((FennFunction *)fenn_to_pointer(x))
#define fenn_unwrap_cfunction(x) ((FennCFunction)fenn_to_pointer(x))


/* Garbage Collection */

typedef enum FennMemoryType FennMemoryType;

enum FennMemoryType {
    FENN_MEMORY_NONE,
    FENN_MEMORY_STRING,
    FENN_MEMORY_SYMBOL,
    FENN_MEMORY_ARRAY,
    FENN_MEMORY_TUPLE,
    FENN_MEMORY_TABLE,
    FENN_MEMORY_STRUCT,
    FENN_MEMORY_FIBER,
    FENN_MEMORY_BUFFER,
    FENN_MEMORY_FUNCTION,
    FENN_MEMORY_ABSTRACT,
    FENN_MEMORY_FUNCENV,
    FENN_MEMORY_FUNCDEF
};

void *fenn_gcalloc(FennMemoryType, size_t);

/* API */

#ifdef __cplusplus
}
#endif

#endif
