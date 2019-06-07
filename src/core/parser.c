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
#include <parser.h>

/* First we have the utility functions to check the types of characters */

/* Checks if a character is whitespace - whitespace is ignored */
int is_whitespace(uint8_t c) {
    return (c == ' '
            || c == '\t'
            || c == '\n'
            || c == '\r'
            || c == '\0'
            || c == '\v'
            || c == '\f');
}

/* These characters can be used in symbols/identifiers */
int is_symbol_char(uint8_t c) {
    if (c & 0x80) return 1;
    if (c >= 'a' && c <= 'z') return 1;
    if (c >= 'A' && c <= 'Z') return 1;
    if (c >= '0' && c <= '9') return 1;
    return (c == '!' ||
            c == '$' ||
            c == '%' ||
            c == '&' ||
            c == '*' ||
            c == '+' ||
            c == '-' ||
            c == '.' ||
            c == '/' ||
            c == ':' ||
            c == '<' ||
            c == '?' ||
            c == '=' ||
            c == '>' ||
            c == '@' ||
            c == '^' ||
            c == '_' ||
            c == '|');
}

/* Parser Utility functions */
void _pushstate(Parser *p,  ParseState ps) {
    size_t oldcount = p->statecount;
    size_t newcount = oldcount + 1;
    if (newcount > p->statecap) {
        ParseState *next;
        size_t newcap = 2 * newcount;
        next = realloc(p->states, sizeof(ParseState) * newcap);
        if (NULL == next) {
            // TODO: Handle Out of Memory error
        }
        p->states = next;
        p->statecap = newcap;
    }
    p->states[oldcount] = ps;
    p->statecount = newcount;
}


void pushstate(Parser *p, Consumer consumer, int flags) {
    ParseState s;
    s.counter = 0;
    s.argn = 0;
    s.flags = flags;
    s.consumer = consumer;
    s.start = p->offset;
    _pushstate(p, s);
}


/* Parses a single expression - returns the number of characters consumed */
int expression(Parser *p, ParseState *state, uint8_t c) {
    switch (c) {
        /* Special characters */
        case '\'': // quote
        // fallthrough
        case ',':  // unquote
        // fallthrough
        case ';':  // splice
        // fallthrough
        case '`':  // quasiquote
            return 1;

        /* String */
        case '"':
            // Read string
            return 1;

        /* Comment until end of line */
        case '#':
            return 1;
        // Mutable operator
        case '@':
            return 1;

        /* Open brackets */
        case '(':
            return 1;
        case '[':
            return 1;
        case '{':
            return 1;

        /* Close brackets */
        case ')':
        case ']':
        case '}':
            return 1;

        // Check for whitespace or identifier characters
        default:
            if (is_whitespace(c))
                return 1;
            if (!is_symbol_char(c)) {
                p->error = "don't know what to do with this character";
                return 1;
            }
            return 0;

    }
}

// Public functions
void parser_init(Parser *parser) {
    parser->error = NULL;
    parser->offset = 0;
    parser->lineno = 1;
    parser->colno = 1;

    /* States */
    parser->states = NULL;
    parser->statecount = 0;
    parser->statecap = 0;
}

void parser_destroy(Parser *parser) {
    // Free memory for buffer
    free(parser->buffer);
    // Free memory for states
    free(parser->states);
}
