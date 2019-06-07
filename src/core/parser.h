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

#ifndef PARSER_H
#define PARSER_H

// Typedef to make things easier
typedef struct Parser Parser;
typedef struct ParseState ParseState;

typedef int (*Consumer)(Parser *p, ParseState *state, uint8_t c);
struct ParseState {
    int32_t counter;
    int32_t argn;
    int flags;
    size_t start;
    Consumer consumer;
};

/* The Fenn parser structure */
struct Parser {
    const char *error;  // The current error
    ParseState *states; // Store the stack of ParseStates
    uint8_t *buffer;    // The buffer we are currently parsing
    size_t statecount;  // Number of states on the stack
    size_t statecap;    // Amount of memory allocated for states
    int offset;         // Stores the offset into the buffer we are parsing
    int lineno;         // The current line number
    int colno;          // The current column number
};

/* Function declarations */
int is_whitespace(uint8_t);
int is_symbol_char(uint8_t);

void pushstate(Parser *, Consumer, int);

int expression(Parser *, ParseState *, uint8_t);

#endif
