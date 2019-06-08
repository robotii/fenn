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

void pushbuffer(Parser *p, uint8_t x) {
    size_t oldcount = p->buffercount;
    size_t newcount = oldcount + 1;
    if (newcount > p->buffercap) {
        uint8_t *next;
        size_t newcap = 2 * newcount;
        next = realloc(p->buffer, sizeof(uint8_t) * newcap);
        if (NULL == next) {
            // TODO: Handle Out of Memory error
        }
        p->buffer = next;
        p->buffercap = newcap;
    }
    p->buffer[oldcount] = x;
    p->buffercount = newcount;
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
        case '`':  // quasi-quote
            return 1;

        /* String */
        case '"':
            pushstate(p, stringchar, PFLAG_STRING);
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

int stringchar(Parser *p, ParseState *state, uint8_t c) {
    if (state->flags & PFLAG_INSTRING) {
        // We are inside the long string
        if (c == '"') {
            // We have seen a '"', so we need to check if the string is ending
            state->flags |= PFLAG_END_CANDIDATE;
            state->flags &= ~PFLAG_INSTRING;
            state->counter = 1; // Use counter to keep track of number of '"' seen
            return 1;
        }
        // Check if we are parsing a long string or short string
        // No escape handling inside long strings
        if(state->flags & PFLAG_LONGSTRING) {
            pushbuffer(p, c);
        } else {
            // Handle escape characters
            if (c == '\\') {
                state->consumer = escape;
                return 1;
            }
            // Handle normal characters - newlines are removed in short strings
            if (c != '\n')
                pushbuffer(p, c);
            return 1;
        }
        return 1;
    } else if (state->flags & PFLAG_END_CANDIDATE) {
        int i;
        // Check for potential end of the string
        if (state->counter == state->argn) {
            stringend(p, state);
            return 0;
        }
        // Keep track of the number of '"' we have seen
        if (c == '"' && state->counter < state->argn) {
            state->counter++;
            return 1;
        }
        // This is not the end of the string
        for (i = 0; i < state->counter; i++) {
            pushbuffer(p, '"');
        }
        pushbuffer(p, c);
        state->counter = 0; // Reset the counter of number of '"' seen
        state->flags &= ~PFLAG_END_CANDIDATE;
        state->flags |= PFLAG_INSTRING;
        return 1;
    } else {
        /* We are at beginning of string */
        state->argn++;
        // If we see anything other than a '"' we are now inside the string
        // and we need to process the character. Likewise if we have already seen
        // 3 '"' characters we need to get started...
        if (c != '"' || state->argn >= 3) {
            state->flags |= PFLAG_INSTRING;
            pushbuffer(p, c);
        } else {
            state->flags |= PFLAG_LONGSTRING;
            state->flags &= ~PFLAG_INSTRING;
        }
        return 1;
    }

}

int checkescape(uint8_t c) {
    // TODO: escape handling
    return c;
}

int escape(Parser *p, ParseState *state, uint8_t c) {
    int e = checkescape(c);
    if (e < 0) {
        p->error = "invalid string escape sequence";
        return 1;
    }
    if (c == 'x') {
        state->counter = 2;
        state->argn = 0;
        state->consumer = escapehex;
    } else {
        pushbuffer(p, (uint8_t) e);
        state->consumer = stringchar;
    }
    return 1;
}

int escapehex(Parser *p, ParseState *state, uint8_t c) {
    return 1;
}

int stringend(Parser *p, ParseState *state) {
    uint8_t *bufstart = p->buffer;
    int32_t buflen = (int32_t) p->buffercount;
    if (state->flags & PFLAG_LONGSTRING) {
        /* Remove leading and trailing newline characters */
        if (bufstart[0] == '\n') {
            bufstart++;
            buflen--;
        }
        if (buflen > 0 && bufstart[buflen - 1] == '\n') {
            buflen--;
        }
        // TODO: Parse this out into a string value
    }
    p->buffercount = 0;
    return 1;
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

    /* Buffer */
    parser->buffer = NULL;
    parser->buffercount = 0;
    parser->buffercap = 0;
}

void parser_destroy(Parser *parser) {
    // Free memory for buffer
    free(parser->buffer);
    // Free memory for states
    free(parser->states);
}
