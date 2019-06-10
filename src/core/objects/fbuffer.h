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

#ifndef BUFFER_H
#define BUFFER_H


typedef struct FennBuffer FennBuffer;

struct FennBuffer {
    FennGCObject gc;
    int32_t count;
    int32_t capacity;
    uint8_t *data;
};

/* Functions */
FennBuffer *fenn_buffer_init(FennBuffer *, int32_t);
void fenn_buffer_destroy(FennBuffer *);
FennBuffer *fenn_buffer(int32_t);
void fenn_buffer_ensure(FennBuffer *, int32_t, int32_t);
void fenn_buffer_setcount(FennBuffer *, int32_t);
void fenn_buffer_extra(FennBuffer *, int32_t n);
void fenn_buffer_push_cstring(FennBuffer *, const char *);
void fenn_buffer_push_bytes(FennBuffer *, const uint8_t *, int32_t);
void fenn_buffer_push_string(FennBuffer *, const uint8_t *);
void fenn_buffer_push_u8(FennBuffer *, uint8_t);
void fenn_buffer_push_u16(FennBuffer *, uint16_t);
void fenn_buffer_push_u32(FennBuffer *, uint32_t);
void fenn_buffer_push_u64(FennBuffer *, uint64_t);

#endif
