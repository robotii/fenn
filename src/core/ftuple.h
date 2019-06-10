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

#ifndef TUPLE_H
#define TUPLE_H

typedef struct FennTupleHead FennTupleHead;

struct FennTupleHead {
    FennGCObject gc;
    int32_t length;
    int32_t hash;
    int32_t sm_start;
    int32_t sm_startline;
    int32_t sm_startcol;
    int32_t sm_end;
    int32_t sm_endline;
    int32_t sm_endcol;
    const FennObject data[];
};

#define fenn_tuple_head(t) ((FennTupleHead *)((char *)t - offsetof(FennTupleHead, data)))
#define fenn_tuple_length(t) (fenn_tuple_head(t)->length)
#define fenn_tuple_hash(t) (fenn_tuple_head(t)->hash)
#define fenn_tuple_sm_start(t) (fenn_tuple_head(t)->sm_start)
#define fenn_tuple_sm_startline(t) (fenn_tuple_head(t)->sm_startline)
#define fenn_tuple_sm_startcol(t) (fenn_tuple_head(t)->sm_startcol)
#define fenn_tuple_sm_end(t) (fenn_tuple_head(t)->sm_end)
#define fenn_tuple_sm_endline(t) (fenn_tuple_head(t)->sm_endline)
#define fenn_tuple_sm_endcol(t) (fenn_tuple_head(t)->sm_endcol)
#define fenn_tuple_flag(t) (fenn_tuple_head(t)->gc.flags)

/* Function declarations */
FENN_API FennObject *fenn_tuple_begin(int32_t);
FENN_API const FennObject *fenn_tuple_end(FennObject *);
FENN_API const FennObject *fenn_tuple_n(const FennObject *, int32_t);
FENN_API int fenn_tuple_equal(const FennObject *, const FennObject *);
FENN_API int fenn_tuple_compare(const FennObject *, const FennObject *);



#endif