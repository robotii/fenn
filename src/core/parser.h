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
    uint32_t flags;
    size_t start;
    Consumer consumer;
};

/* The Fenn parser structure */
struct Parser {
    const char *error;   // The current error
    ParseState *states;  // Store the stack of ParseStates

    size_t statecount;   // Number of states on the stack
    size_t statecap;     // Number of states allocated
    uint8_t *buffer;     // The buffer we are currently parsing
    size_t  buffercount; // Size of the current buffer
    size_t  buffercap;   // Capacity of the buffer
    int offset;          // Stores the current offset into the buffer we are parsing
    int lineno;          // The current line number
    int colno;           // The current column number
};

/* Flags */
#define PFLAG_CONTAINER     ((uint32_t)0x100)
#define PFLAG_BUFFER        ((uint32_t)0x200)
#define PFLAG_PARENS        ((uint32_t)0x400)
#define PFLAG_SQRBRACKETS   ((uint32_t)0x800)
#define PFLAG_CURLYBRACKETS ((uint32_t)0x1000)
#define PFLAG_STRING        ((uint32_t)0x2000)
#define PFLAG_LONGSTRING    ((uint32_t)0x4000)
#define PFLAG_READERMAC     ((uint32_t)0x8000)
#define PFLAG_ATSYM         ((uint32_t)0x10000)
#define PFLAG_INSTRING      ((uint32_t)0x100000)
#define PFLAG_END_CANDIDATE ((uint32_t)0x200000)

/* Function declarations */
int is_whitespace(uint8_t);
int is_symbol_char(uint8_t);

void pushstate(Parser *, Consumer, int);
void pushbuffer(Parser *, uint8_t);

/* Consumers */
int expression(Parser *, ParseState *, uint8_t);
int stringchar(Parser *, ParseState *, uint8_t);
int escape(Parser *, ParseState *, uint8_t);
int escapehex(Parser *, ParseState *, uint8_t);

/* Utility functions */
int checkescape(uint8_t);
int stringend(Parser *, ParseState *);

/* Creation and destruction */
void parser_init(Parser *);
void parser_destroy(Parser *);

#endif
