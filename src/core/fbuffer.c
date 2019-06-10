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
#include "fbuffer.h"

#include "gc.h"
#include "util.h"
#include "fstring.h"

/* Initialize a buffer */
FennBuffer *fenn_buffer_init(FennBuffer *buffer, int32_t capacity) {
    uint8_t *data = NULL;
    if (capacity > 0) {
        data = malloc(sizeof(uint8_t) * capacity);
        if (NULL == data) {
            // TODO: Handle Out Of Memory
        }
    }
    buffer->count = 0;
    buffer->capacity = capacity;
    buffer->data = data;
    return buffer;
}

/* Deinitialize a buffer (free data memory) */
void fenn_buffer_deinit(FennBuffer *buffer) {
    free(buffer->data);
}

/* Initialize a buffer */
FennBuffer *fenn_buffer(int32_t capacity) {
    FennBuffer *buffer = fenn_gcalloc(FENN_MEMORY_BUFFER, sizeof(FennBuffer));
    return fenn_buffer_init(buffer, capacity);
}

/* Ensure that the buffer has enough internal capacity */
void fenn_buffer_ensure(FennBuffer *buffer, int32_t capacity, int32_t growth) {
    uint8_t *new_data;
    uint8_t *old = buffer->data;
    if (capacity <= buffer->capacity) return;
    int64_t big_capacity = capacity * growth;
    capacity = big_capacity > INT32_MAX ? INT32_MAX : (int32_t) big_capacity;
    new_data = realloc(old, capacity * sizeof(uint8_t));
    if (NULL == new_data) {
        // TODO: Handle Out Of Memory
    }
    buffer->data = new_data;
    buffer->capacity = capacity;
}

/* Ensure that the buffer has enough internal capacity */
void fenn_buffer_setcount(FennBuffer *buffer, int32_t count) {
    if (count < 0)
        return;
    if (count > buffer->count) {
        int32_t oldcount = buffer->count;
        fenn_buffer_ensure(buffer, count, 1);
        memset(buffer->data + oldcount, 0, count - oldcount);
    }
    buffer->count = count;
}

/* Adds capacity for enough extra bytes to the buffer. Ensures that the
 * next n bytes pushed to the buffer will not cause a reallocation */
void fenn_buffer_extra(FennBuffer *buffer, int32_t n) {
    /* Check for buffer overflow */
    if ((int64_t)n + buffer->count > INT32_MAX) {
        // TODO: handle buffer overflow
    }
    int32_t new_size = buffer->count + n;
    if (new_size > buffer->capacity) {
        int32_t new_capacity = new_size * 2;
        uint8_t *new_data = realloc(buffer->data, new_capacity * sizeof(uint8_t));
        if (NULL == new_data) {
            // TODO: Handle Out Of Memory
        }
        buffer->data = new_data;
        buffer->capacity = new_capacity;
    }
}

/* Push a cstring to buffer */
void fenn_buffer_push_cstring(FennBuffer *buffer, const char *cstring) {
    int32_t len = 0;
    while (cstring[len]) ++len;
    fenn_buffer_push_bytes(buffer, (const uint8_t *) cstring, len);
}

/* Push multiple bytes into the buffer */
void fenn_buffer_push_bytes(FennBuffer *buffer, const uint8_t *string, int32_t length) {
    fenn_buffer_extra(buffer, length);
    memcpy(buffer->data + buffer->count, string, length);
    buffer->count += length;
}

void fenn_buffer_push_string(FennBuffer *buffer, const uint8_t *string) {
    fenn_buffer_push_bytes(buffer, string, fenn_string_length(string));
}

/* Push a single byte to the buffer */
void fenn_buffer_push_u8(FennBuffer *buffer, uint8_t byte) {
    fenn_buffer_extra(buffer, 1);
    buffer->data[buffer->count] = byte;
    buffer->count++;
}

/* Push a 16 bit unsigned integer to the buffer */
void fenn_buffer_push_u16(FennBuffer *buffer, uint16_t x) {
    fenn_buffer_extra(buffer, 2);
    buffer->data[buffer->count] = x & 0xFF;
    buffer->data[buffer->count + 1] = (x >> 8) & 0xFF;
    buffer->count += 2;
}

/* Push a 32 bit unsigned integer to the buffer */
void fenn_buffer_push_u32(FennBuffer *buffer, uint32_t x) {
    fenn_buffer_extra(buffer, 4);
    buffer->data[buffer->count] = x & 0xFF;
    buffer->data[buffer->count + 1] = (x >> 8) & 0xFF;
    buffer->data[buffer->count + 2] = (x >> 16) & 0xFF;
    buffer->data[buffer->count + 3] = (x >> 24) & 0xFF;
    buffer->count += 4;
}

/* Push a 64 bit unsigned integer to the buffer */
void fenn_buffer_push_u64(FennBuffer *buffer, uint64_t x) {
    fenn_buffer_extra(buffer, 8);
    buffer->data[buffer->count] = x & 0xFF;
    buffer->data[buffer->count + 1] = (x >> 8) & 0xFF;
    buffer->data[buffer->count + 2] = (x >> 16) & 0xFF;
    buffer->data[buffer->count + 3] = (x >> 24) & 0xFF;
    buffer->data[buffer->count + 4] = (x >> 32) & 0xFF;
    buffer->data[buffer->count + 5] = (x >> 40) & 0xFF;
    buffer->data[buffer->count + 6] = (x >> 48) & 0xFF;
    buffer->data[buffer->count + 7] = (x >> 56) & 0xFF;
    buffer->count += 8;
}