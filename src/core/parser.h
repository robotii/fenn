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
typedef enum ParserStatus ParserStatus;
typedef int (*Consumer)(Parser *p, ParseState *state, uint8_t c);

enum ParserStatus {
    PARSE_OK,
    PARSE_ERROR,
    PARSE_PENDING,
    PARSE_DEAD
};

struct ParseState {
    int32_t counter;
    int32_t argn;
    uint32_t flags;
    size_t start;
    int32_t startline;
    int32_t startcol;
    Consumer consumer;
};

/* The Fenn parser structure */
struct Parser {
    const char *error;   // The current error

    // State
    ParseState *states;  // Store the stack of ParseStates
    size_t statecount;   // Number of states on the stack
    size_t statecap;     // Number of states allocated
    int offset;          // Stores the current offset into the buffer we are parsing
    int lineno;          // The current line number
    int colno;           // The current column number
    int finished;        // Flag to show if we are finished parsing
    int pending;         // How many values we have pending

    // Buffer
    uint8_t *buffer;     // The buffer we are currently parsing into
    size_t  buffercount; // Size of the current buffer
    size_t  buffercap;   // Capacity of the buffer

    // Values
    FennObject *values;  // Stack of values to return
    size_t valuecount;   // Number of values present on the stack
    size_t valuecap;     // Capacity of the value stack

    uint8_t current;     // The current character being processed
};

/* Flags */
#define FLAG_CONTAINER     ((uint32_t)0x100)
#define FLAG_BUFFER        ((uint32_t)0x200)
#define FLAG_PARENS        ((uint32_t)0x400)
#define FLAG_SQRBRACKETS   ((uint32_t)0x800)
#define FLAG_CURLYBRACKETS ((uint32_t)0x1000)
#define FLAG_STRING        ((uint32_t)0x2000)
#define FLAG_LONGSTRING    ((uint32_t)0x4000)
#define FLAG_READERMAC     ((uint32_t)0x8000)
#define FLAG_ATSYM         ((uint32_t)0x10000)
#define FLAG_INSTRING      ((uint32_t)0x100000)
#define FLAG_END_CANDIDATE ((uint32_t)0x200000)

/* Function declarations */

/* Character utilities */
int is_whitespace(uint8_t);
int is_symbol_char(uint8_t);
int hex(uint8_t);
int checkescape(uint8_t);
int validate_utf8(const uint8_t *, int32_t);

int check_str_const(const char *, const uint8_t *, int32_t);

/* State management */
void pushstate(Parser *, Consumer, int);
void popstate(Parser *, FennObject);
void pushbuffer(Parser *, uint8_t);
void pushvalue(Parser *, FennObject);

/* Parser utility functions */
ParserStatus parser_status(Parser *);
int stringend(Parser *, ParseState *);
void parser_ok(Parser *);
void parser_consume(Parser *, uint8_t);
void parser_eof(Parser *);
void parser_flush(Parser *parser);
const char *parser_error(Parser *parser);
FennObject parser_produce(Parser *parser);

/* Consumers */
int expression(Parser *, ParseState *, uint8_t);
int stringchar(Parser *, ParseState *, uint8_t);
int escape(Parser *, ParseState *, uint8_t);
int escapehex(Parser *, ParseState *, uint8_t);
int linecomment(Parser *, ParseState *, uint8_t);
int atsymbol(Parser *, ParseState *, uint8_t);
int token(Parser *, ParseState *, uint8_t);

/* Creation and destruction */
void parser_init(Parser *);
void parser_destroy(Parser *);

#endif


