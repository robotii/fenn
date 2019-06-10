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
#include "tuple.h"

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

/* Get hex digit from a letter */
int hex(uint8_t c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'A' && c <= 'F') {
        return 10 + c - 'A';
    } else if (c >= 'a' && c <= 'f') {
        return 10 + c - 'a';
    } else {
        return -1;
    }
}

int validate_utf8(const uint8_t *str, int32_t len) {
    // TODO: Validate the UTF-8
    (void)str;
    return len;
}

/* Parser Utility functions */

/* Push the current consumer onto the state stack */
void pushstate(Parser *p, Consumer consumer, int flags) {
    ParseState s;
    s.counter = 0;
    s.argn = 0;
    s.flags = flags;
    s.consumer = consumer;
    s.start = p->offset;
    s.startline = p->lineno;
    s.startcol = p->colno;

    // Push the state
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
    p->states[oldcount] = s;
    p->statecount = newcount;
}

void popstate(Parser *p, FennObject value) {
    for (;;) {
        ParseState top = p->states[--p->statecount];
        ParseState *newtop = p->states + p->statecount - 1;
        if (newtop->flags & FLAG_CONTAINER) {
            /* Source mapping info */
            if (fenn_checktype(value, FENN_TUPLE)) {
                fenn_tuple_sm_start(fenn_unwrap_tuple(value)) = (int32_t) top.start;
                fenn_tuple_sm_startline(fenn_unwrap_tuple(value)) = top.startline;
                fenn_tuple_sm_startcol(fenn_unwrap_tuple(value)) = top.startcol;
                fenn_tuple_sm_end(fenn_unwrap_tuple(value)) = (int32_t) p->offset;
                fenn_tuple_sm_endline(fenn_unwrap_tuple(value)) = p->lineno;
                fenn_tuple_sm_endcol(fenn_unwrap_tuple(value)) = p->colno;
            }
            newtop->argn++;
            /* Keep track of number of values in the root state */
            if (p->statecount == 1) p->pending++;
            pushvalue(p, value);
            return;
        } else if (newtop->flags & FLAG_READERMAC) {
            FennObject *t = fenn_tuple_begin(2);
            int c = newtop->flags & 0xFF;
            const char *which =
                    (c == '\'') ? "quote" :
                    (c == ',') ? "unquote" :
                    (c == ';') ? "splice" :
                    (c == '~') ? "quasiquote" : "<unknown>";
            // TODO: Lookup symbol from which
            t[0] = (FennObject)NULL;
            t[1] = value;
            /* Quote source mapping info */
            fenn_tuple_sm_start(t) = (int32_t) newtop->start;
            fenn_tuple_sm_startline(t) = top.startline;
            fenn_tuple_sm_startcol(t) = top.startcol;
            fenn_tuple_sm_end(t) = (int32_t) p->offset;
            fenn_tuple_sm_endline(t) = p->lineno;
            fenn_tuple_sm_endcol(t) = p->colno;
            value = fenn_wrap_tuple(fenn_tuple_end(t));
        } else {
            return;
        }
    }
}

/* Push a character onto the end of the buffer */
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

/* Push a value onto the stack */
void pushvalue(Parser *p, FennObject x) {
    size_t oldcount = p->valuecount;
    size_t newcount = oldcount + 1;
    if (newcount > p->valuecap) {
        FennObject *next;
        size_t newcap = 2 * newcount;
        next = realloc(p->values, sizeof(FennObject) * newcap);
        if (NULL == next) {
            // TODO: Handle Out Of Memory error
        }
        p->values = next;
        p->valuecap = newcap;
    }
    p->values[oldcount] = x;
    p->valuecount = newcount;
}

/* Consumer functions */

/* Parses a single expression - returns the number of characters consumed */
int expression(Parser *p, ParseState *state, uint8_t c) {
    switch (c) {
        // Special characters
        case '\'': // quote
            // fallthrough
        case ',':  // unquote
            // fallthrough
        case ';':  // splice
            // fallthrough
        case '`':  // quasi-quote
            pushstate(p, expression, FLAG_READERMAC | c);
            return 1;

        // String
        case '"':
            pushstate(p, stringchar, FLAG_STRING);
            return 1;

        // Comment until end of line
        case '#':
            pushstate(p, linecomment, 0);
            return 1;

        // Mutable operator
        case '@':
            pushstate(p, atsymbol, 0);
            return 1;

        // Open brackets
        case '(':
            pushstate(p, expression, FLAG_CONTAINER | FLAG_PARENS);
            return 1;
        case '[':
            pushstate(p, expression, FLAG_CONTAINER | FLAG_SQRBRACKETS);
            return 1;
        case '{':
            pushstate(p, expression, FLAG_CONTAINER | FLAG_CURLYBRACKETS);
            return 1;

        // Close brackets
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
            pushstate(p, token, 0);
            return 0;
    }
}

/* Parse the "@" prefix */
int atsymbol(Parser *p, ParseState *state, uint8_t c) {
    (void) state;
    p->statecount--; // Discard the current state
    switch (c) {
        case '{':
            pushstate(p, expression, FLAG_CONTAINER | FLAG_CURLYBRACKETS | FLAG_ATSYM);
            return 1;
        case '"':
            pushstate(p, stringchar, FLAG_BUFFER | FLAG_STRING);
            return 1;
        case '[':
            pushstate(p, expression, FLAG_CONTAINER | FLAG_SQRBRACKETS | FLAG_ATSYM);
            return 1;
        case '(':
            pushstate(p, expression, FLAG_CONTAINER | FLAG_PARENS | FLAG_ATSYM);
            return 1;
        default:
            pushstate(p, token, 0);
            pushbuffer(p, '@'); // Push the leading '@', as it is the start of a symbol
            return 0;
    }
}

/* Parse a single token */
int token(Parser *p, ParseState *state, uint8_t c) {
    FennObject value = (FennObject)NULL;
    double numval; // Holds the number we have parsed
    int32_t blen;
    if (is_symbol_char(c)) {
        pushbuffer(p, (uint8_t) c);
        if (c > 127) {
            state->argn = 1; // Used to indicate non ascii character detected
        }
        return 1;
    }
    // Token finished
    blen = (int32_t) p->buffercount;
    int start_dig = p->buffer[0] >= '0' && p->buffer[0] <= '9';
    int start_num = start_dig || p->buffer[0] == '-' || p->buffer[0] == '+' || p->buffer[0] == '.';

    if (p->buffer[0] == ':') {
        // Return keyword
    } else if (start_num /*&& we are able to convert to number */) {
        // Return number
    } else if (0 /* check if is "nil" */) {
        // Return nil
    } else if (0 /* check if is "false" */) {
        // Return false
    } else if (0 /* check if is "true" */) {
        // Return true
    } else if (p->buffer) {
        if (start_dig) {
            p->error = "symbol literal cannot start with a digit";
            return 0;
        } else {
            // Validate utf-8
            int valid = (!state->argn) || validate_utf8(p->buffer, blen);
            if (!valid) {
                p->error = "invalid utf-8";
                return 0;
            }
            // Return the symbol
        }
    } else {
        p->error = "empty symbol";
        return 0;
    }
    p->buffercount = 0;
    popstate(p,value);
    return 0;
}

int stringchar(Parser *p, ParseState *state, uint8_t c) {
    if (state->flags & FLAG_INSTRING) {
        // We are inside the long string
        if (c == '"') {
            // We have seen a '"', so we need to check if the string is ending
            state->flags |= FLAG_END_CANDIDATE;
            state->flags &= ~FLAG_INSTRING;
            state->counter = 1; // Use counter to keep track of number of '"' seen
            return 1;
        }
        // Check if we are parsing a long string or short string
        // No escape handling inside long strings
        if (state->flags & FLAG_LONGSTRING) {
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
    } else if (state->flags & FLAG_END_CANDIDATE) {
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
        // so we push the correct number of '"' back into the buffer
        for (i = 0; i < state->counter; i++) {
            pushbuffer(p, '"');
        }
        pushbuffer(p, c);
        state->counter = 0; // Reset the counter of number of '"' seen
        state->flags &= ~FLAG_END_CANDIDATE;
        state->flags |= FLAG_INSTRING;
        return 1;
    } else {
        // We are at the beginning of the string
        state->argn++;
        // If we see anything other than a '"' we are now inside the string
        // and we need to process the character. Likewise if we have already seen
        // 3 '"' characters we need to get started...
        if (c != '"' || state->argn >= 3) {
            // If we have two characters then we have an error
            if(state->argn == 2) {
                p->error = "invalid string";
            }
            state->flags |= FLAG_INSTRING;
            pushbuffer(p, c);
        } else {
            state->flags |= FLAG_LONGSTRING;
            state->flags &= ~FLAG_INSTRING;
        }
        return 1;
    }

}

/* Handle escape sequences */
int checkescape(uint8_t c) {
    switch (c) {
        // Handle hex escapes separately
        case 'x':
        case 'u':
        case 'U':
            return 1;
        case 'a':
            return 7;
        case 'b':
            return 8;
        case 'n':
            return '\n';
        case 't':
            return '\t';
        case 'r':
            return '\r';
        case '0':
            return '\0';
        case 'f':
            return '\f';
        case 'v':
            return '\v';
        case 'e':
            return 27;
        case '"':
            return '"';
        case '\'':
            return '\'';
        case '\\':
            return '\\';
        case '?':
            return '?';
        default:
            return -1;
    }
}

int escape(Parser *p, ParseState *state, uint8_t c) {
    int e = checkescape(c);
    if (e < 0) {
        p->error = "invalid string escape sequence";
        return 1;
    }
    if (c == 'x') { // 2 hex digits
        state->counter = 2;
        state->argn = 0;
        state->consumer = escapehex;
    } else if(c == 'u') { // Unicode 4 hex digits
        state->counter = 4;
        state->argn = 0;
        state->consumer = escapehex;
    } else if(c == 'U') { // Unicode 8 hex digits
        state->counter = 8;
        state->argn = 0;
        state->consumer = escapehex;
    } else {
        pushbuffer(p, (uint8_t) e);
        state->consumer = stringchar;
    }
    return 1;
}

int escapehex(Parser *p, ParseState *state, uint8_t c) {
    int digit = hex(c);
    if (digit < 0) {
        p->error = "invalid hex digit";
        return 1;
    }
    state->argn = (state->argn << 4) + digit;
    state->counter--;
    if (!(state->counter % 2)) {
        pushbuffer(p, (state->argn & 0xFF));
        state->argn = 0;
        if(!state->counter) {
            state->consumer = stringchar;
        }
    }
    return 1;
}

int linecomment(Parser *p, ParseState *state, uint8_t c) {
    (void) state; // Pretend we use the state
    if (c == '\n') {
        p->statecount--; // Pop the state when we see a new line
    }
    return 1;
}

int stringend(Parser *p, ParseState *state) {
    uint8_t *bufstart = p->buffer;
    int32_t buflen = (int32_t) p->buffercount;
    if (state->flags & FLAG_LONGSTRING) {
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

/* Returns the current status of the parser */
ParserStatus parser_status(Parser *parser) {
    if (parser->error) return PARSE_ERROR;
    if (parser->finished) return PARSE_DEAD;
    if (parser->statecount > 1) return PARSE_PENDING;
    return PARSE_OK;
}

// Check if the parser is in a state that allows it to continue
void parser_ok(Parser *parser) {
    if (parser->finished) {
        // TODO: parser is finished
    }
    if (parser->error) {
        // TODO: parser has error
    }
}

/* Flush the parser */
void parser_flush(Parser *parser) {
    parser->statecount = 1;
    parser->buffercount = 0;
}

/* Returns the error string from the parser */
const char *parser_error(Parser *parser) {
    ParserStatus status = parser_status(parser);
    if (status == PARSE_ERROR) {
        const char *e = parser->error;
        parser->error = NULL;
        parser_flush(parser);
        return e;
    }
    return NULL;
}

FennObject parser_produce(Parser *parser) {
    FennObject ret;
    size_t i;
    if (parser->pending == 0) {
        return (FennObject)NULL;
    }
    ret = parser->values[0];
    for (i = 1; i < parser->valuecount; i++) {
        parser->values[i - 1] = parser->values[i];
    }
    parser->pending--;
    parser->valuecount--;
    return ret;
}


// Consumes a single character
void parser_consume(Parser *parser, uint8_t c) {
    int consumed = 0;
    parser_ok(parser);
    parser->offset++;
    if (c == '\n') {
        parser->lineno++;
        parser->colno = 1;
    } else {
        parser->colno++;
    }
    while (!consumed && !parser->error) {
        ParseState *state = parser->states + parser->statecount - 1;
        consumed = state->consumer(parser, state, c);
    }
    parser->current = c;
}

/* Handle the end of the buffer */
void parser_eof(Parser *parser) {
    parser_ok(parser);
    parser_consume(parser, '\n');
    if (parser->statecount > 1) {
        parser->error = "unexpected end of input";
    }
    parser->offset--;
    parser->finished = 1;
}


/* Public functions */

/* Initialise the parser */
void parser_init(Parser *parser) {
    parser->error = NULL;
    parser->offset = 0;
    parser->lineno = 1;
    parser->colno = 1;
    parser->finished = 0;

    /* States */
    parser->states = NULL;
    parser->statecount = 0;
    parser->statecap = 0;

    /* Buffer */
    parser->buffer = NULL;
    parser->buffercount = 0;
    parser->buffercap = 0;

    // Values
    parser->values = NULL;
    parser->valuecount = 0;
    parser->valuecap = 0;
}

/* Free all memory allocated in this parser */
void parser_destroy(Parser *parser) {
    // Free memory for buffer
    free(parser->buffer);
    // Free memory for states
    free(parser->states);
    // Free memory for values
    free(parser->values);
}
