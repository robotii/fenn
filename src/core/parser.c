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

/* Parses a single expression */
int expression(FennParser *p, uint8_t c) {
    switch(c) {

        default:
            if(is_whitespace(c))
                return 1;
            if(!is_symbol_char(c))
                return 1;
            return 0;

            /* String */
        case '"':
            return 0;

    }
}

// Public functions

void fenn_parser_init(FennParser *parser) {
    parser->error = NULL;
    parser->offset = 0;
    parser->lineno = 1;
    parser->colno = 1;
}

void fenn_parser_destroy(FennParser *parser) {
    // Free memory for buffer
    free(parser->buffer);
}
