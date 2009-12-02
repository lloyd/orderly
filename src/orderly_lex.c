/*
 * Copyright 2009, Lloyd Hilaiel.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 * 
 *  3. Neither the name of Lloyd Hilaiel nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */ 

#include "orderly_lex.h"
#include "orderly_alloc.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

struct orderly_lexer_t {
    unsigned int previousOffset;
    /* error */
    orderly_lex_error error;
    orderly_alloc_funcs * alloc;
};

orderly_lexer
orderly_lex_alloc(orderly_alloc_funcs * alloc)
{
    static orderly_alloc_funcs orderlyAllocFuncBuffer;
    static orderly_alloc_funcs * orderlyAllocFuncBufferPtr = NULL;

    orderly_lexer lxr = NULL;

    if (orderlyAllocFuncBufferPtr == NULL) {
        orderly_set_default_alloc_funcs(&orderlyAllocFuncBuffer);
        orderlyAllocFuncBufferPtr = &orderlyAllocFuncBuffer;
    }

    if (alloc == NULL) alloc = orderlyAllocFuncBufferPtr;

    assert( alloc != NULL );

    lxr = (orderly_lexer) OR_MALLOC(alloc, sizeof(struct orderly_lexer_t));
    memset((void *) lxr, 0, sizeof(struct orderly_lexer_t));
    lxr->alloc = alloc;
    lxr->previousOffset = 0;

    return lxr;
}

void
orderly_lex_free(orderly_lexer lxr)
{
    OR_FREE(lxr->alloc, lxr);
    return;
}

/* given a string of characters, is it a keyword or a property name (default)
 */
static orderly_tok
keywordCheck(const unsigned char * str, unsigned int len)
{
    static struct keywords_t {
        const char * kw;
        orderly_tok tok;
    } keywords[] = {
        { "any", orderly_tok_kw_any },
        { "array", orderly_tok_kw_array },
        { "boolean", orderly_tok_kw_boolean },
        { "integer", orderly_tok_kw_integer },
        { "null", orderly_tok_kw_null },
        { "number", orderly_tok_kw_number },
        { "object", orderly_tok_kw_object },
        { "string", orderly_tok_kw_string },
        { "union", orderly_tok_kw_union }
    };
    unsigned int i;

    for (i = 0; i < sizeof(keywords)/sizeof(keywords[0]); i++) {
        if (strlen(keywords[i].kw) == len &&
            !strncmp(keywords[i].kw, (const char *) str,
                     strlen(keywords[i].kw)))
        {
            break;
        }
    }

    if (i < sizeof(keywords)/sizeof(keywords[0])) {
        return keywords[i].tok;
    }
    
    return orderly_tok_property_name;
}


#define OLC_READ_NEXT_CHAR if (*offset >= schemaTextLen) return orderly_tok_eof; c = schemaText[(*offset)++];

/* lexing of JSON numbers */
static orderly_tok
orderly_lex_number(orderly_lexer lexer, const unsigned char * schemaText,
                   unsigned int schemaTextLen, unsigned int * offset)
{
    unsigned char c;
    orderly_tok tok = orderly_tok_json_integer;

    OLC_READ_NEXT_CHAR;
 
    /* optional leading minus */
    if (c == '-') {
        OLC_READ_NEXT_CHAR;
    }
    
    /* a single zero, or a series of integers */
    if (c == '0') {
        OLC_READ_NEXT_CHAR;
    }
    else if (c >= '1' && c <= '9') {
        do {
            OLC_READ_NEXT_CHAR;
        } while (c >= '0' && c <= '9');
    }
    else {
        (*offset)--;
        lexer->error = orderly_lex_missing_integer_after_minus;
        return orderly_tok_error;
    }
    
    /* optional fraction (indicates this is floating point) */
    if (c == '.') {
        int numRd = 0;
        
        OLC_READ_NEXT_CHAR;

        while (c >= '0' && c <= '9') {
            numRd++;
            OLC_READ_NEXT_CHAR;
        }
 
        if (!numRd) {
            (*offset)--;
            lexer->error = orderly_lex_missing_integer_after_decimal;
            return orderly_tok_error;
        }
        
        tok = orderly_tok_json_number;
    }
    
 
    /* optional exponent (indicates this is floating point) */
    if (c == 'e' || c == 'E')
    {
        OLC_READ_NEXT_CHAR;
 
        /* optional sign */
        if (c == '+' || c == '-') {
            OLC_READ_NEXT_CHAR;
        }
 
        if (c >= '0' && c <= '9') {
            do {
                OLC_READ_NEXT_CHAR;
            } while (c >= '0' && c <= '9');
        }
        else {
            (*offset)--;
            lexer->error = orderly_lex_missing_integer_after_exponent;
            return orderly_tok_error;
        }
        tok = orderly_tok_json_number;
    }
    
    /* we always go "one too far" */
    (*offset)--;
    
    return tok;
    
}


/* lexing of perl compatible regular expresions */
static orderly_tok
orderly_lex_regex(orderly_lexer lexer, const unsigned char * schemaText,
                  unsigned int schemaTextLen, unsigned int * offset)
{
    unsigned char c;

    /* TODO: PCRE's must be more complex than this.  How much 
     * regex syntax do we need to understand to successfully
     * scan one? */
    do {
        OLC_READ_NEXT_CHAR;            
        if (c == '\\') {
            OLC_READ_NEXT_CHAR;            
            continue;
        }
    } while(c != '/');
    
    return orderly_tok_regex;
}

/* a very basic extraction of JSON strings, we leave it up to the parser
 * to handle the more complicated bits of JSON, such as unescaping
 * and UTF8 validation
 *
 * precondition: offset should point to the first char *after* a quote ('"').
 */
static orderly_tok
orderly_lex_json_string(orderly_lexer lexer, const unsigned char * schemaText,
                        unsigned int schemaTextLen, unsigned int * offset)
{
    orderly_tok tok = orderly_tok_error;
    do {
        char c = schemaText[*offset];
        if ('\\' == c) {
            if (++(*offset) >= schemaTextLen) break;
        } else if ('"' == c) {
            break;
        }
    } while (++(*offset) < schemaTextLen);
    
    if ('"' == schemaText[*offset]) {
        tok = orderly_tok_json_string;                    
        (*offset)++;
    } else {
        lexer->error = orderly_lex_unterminated_string;
        tok = orderly_tok_error;                    
    }

    return tok;
}

/* a very basic extraction of JSON arrays, as with strings we'll leave it up
 * to the parser to handle the more complicated bits of JSON array parsing.
 *
 * precondition: offset should point to the first char *after* a left brace
 *               ('[').
 */
static orderly_tok
orderly_lex_json_array(orderly_lexer lexer, const unsigned char * schemaText,
                       unsigned int schemaTextLen, unsigned int * offset)
{
    orderly_tok tok = orderly_tok_error;
    unsigned int nesting = 1;
    
    do {
        char c = schemaText[*offset];
        if ('"' == c) {
            tok = orderly_lex_json_string(lexer, schemaText, schemaTextLen,
                                          offset);
            if (tok != orderly_tok_json_string) break;
        } else if ('[' == c) {
            ++nesting;
        } else if (']' == c) {
            if (!(--nesting)) break;
        } else {
            /* ignore! */
        }
    } while (++(*offset) < schemaTextLen);
    
    if (!nesting) {
        tok = orderly_tok_json_array;
        (*offset)++;
    } else {
        lexer->error = orderly_lex_unterminated_array;
        tok = orderly_tok_error;                    
    }

    return tok;
}

/* a very basic extraction of JSON objects, as with strings we'll leave it up
 * to the parser to handle the more complicated bits of JSON array parsing.
 *
 * precondition: offset should point to the first char *after* a left curly
 *               ('{').
 */
static orderly_tok
orderly_lex_json_object(orderly_lexer lexer, const unsigned char * schemaText,
                        unsigned int schemaTextLen, unsigned int * offset)
{
    orderly_tok tok = orderly_tok_error;
    unsigned int nesting = 1;
    
    do {
        char c = schemaText[*offset];
        if ('"' == c) {
            tok = orderly_lex_json_string(lexer, schemaText, schemaTextLen,
                                          offset);
            if (tok != orderly_tok_json_string) break;
        } else if ('{' == c) {
            ++nesting;
        } else if ('}' == c) {
            if (!(--nesting)) break;
        } else {
            /* ignore! */
        }
    } while (++(*offset) < schemaTextLen);
    
    if (!nesting) {
        tok = orderly_tok_json_array;
        (*offset)++;
    } else {
        lexer->error = orderly_lex_unterminated_array;
        tok = orderly_tok_error;                    
    }

    return tok;
}

#define CHECK_KEYWORD(_kw)                             \
    if (((*offset) + (strlen(_kw)) < schemaTextLen) && \
        !strncmp((_kw), (const char *) (schemaText + *offset), strlen(_kw))) { \
        *offset += strlen(_kw);                        \
        tok = orderly_tok_default_value;               \
    } 

/* scan a single json value
 * precondition: offset should point to first char of json value */
static orderly_tok
orderly_lex_json_value(orderly_lexer lexer, const unsigned char * schemaText,
                       unsigned int schemaTextLen, unsigned int * offset)
{
    orderly_tok tok = orderly_tok_error;
    switch (schemaText[*offset]) {
        case '"': /* string */
            (*offset)++;
            tok = orderly_lex_json_string(lexer, schemaText,
                                          schemaTextLen, offset);
            break;
        case '[': /* array */
            (*offset)++;
            tok = orderly_lex_json_array(lexer, schemaText,
                                         schemaTextLen, offset);
            break;
        case '{': /* object */
            (*offset)++;
            tok = orderly_lex_json_object(lexer, schemaText,
                                          schemaTextLen, offset);
            break;            
        case '-': /* number */
        case '0': case '1': case '2': case '3': case '4':  
        case '5': case '6': case '7': case '8': case '9': { 
            tok = orderly_lex_number(lexer, (const unsigned char *) schemaText,
                                     schemaTextLen, offset);
            break;
        }
        case 't':
            CHECK_KEYWORD("true");
            break;
        case 'f':            
            CHECK_KEYWORD("false");
            break;
        case 'n':
            CHECK_KEYWORD("null");
            break;
    }
    
    return ((tok == orderly_tok_error || tok == orderly_tok_eof)
            ? orderly_tok_error : orderly_tok_default_value);
}


orderly_tok
orderly_lex_lex(orderly_lexer lexer, const unsigned char * schemaText,
                unsigned int schemaTextLen, unsigned int * offset,
                const unsigned char ** outBuf, unsigned int * outLen)
{
    orderly_tok tok = orderly_tok_error;
    unsigned char c;
    unsigned int startOffset = *offset;
    unsigned int previousOffset = *offset;

    if (outBuf) *outBuf = NULL;
    if (outLen) *outLen = 0;

    for (;;) {
        assert(*offset <= schemaTextLen);

        if (*offset >= schemaTextLen) {
            tok = orderly_tok_eof;
            goto lexed;
        }

        c = schemaText[(*offset)++];

        switch (c) {
            case ',':
                tok = orderly_tok_comma;
                goto lexed;
            case '{':
                tok = orderly_tok_left_curly;
                goto lexed;
            case '}':
                tok = orderly_tok_right_curly;
                goto lexed;
            case '<':
                tok = orderly_tok_lt;
                goto lexed;
            case '>':
                tok = orderly_tok_gt;
                goto lexed;
            case ';':
                tok = orderly_tok_semicolon;
                goto lexed;
            case '?':
                tok = orderly_tok_optional_marker;
                goto lexed;
            case '\t': case '\v': case '\f': case '\r': case ' ': 
                startOffset++; 
                break; 
            case '\n':
                startOffset++; 
                break; 
            case '/': 
                if (*offset >= schemaTextLen) {
                    tok = orderly_tok_eof;
                    goto lexed;
                }
                if ('/' != schemaText[*offset])
                {
                    /* this must be a regexp */
                    tok = orderly_lex_regex(lexer, schemaText,
                                            schemaTextLen, offset);
                    goto lexed;
                }
                /* intentional fallthrough (this was a '//' comment) */
            case '#': {
                /* comment!  ignore until end-of-line */
                do {
                    if ('\n' == schemaText[*offset]) break;
                } while (++(*offset) < schemaTextLen);
                    
                startOffset = *offset;
                if ('\n' == schemaText[*offset]) break;
 
                /* oops!  we hit eof *before* we hit newline */
                tok = orderly_tok_eof;
                goto lexed;
            }
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
            case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
            case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
            case 's': case 't': case 'u': case 'v': case 'w': case 'x':
            case 'y': case 'z': case 'A': case 'B': case 'C': case 'D':
            case 'E': case 'F': case 'G': case 'H': case 'I': case 'J':
            case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
            case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V':
            case 'W': case 'X': case 'Y': case 'Z': {
                do {
                    char c = schemaText[*offset];
                    if (!(c >= 'a' && c <= 'z') && !(c >= 'A' && c <= 'Z') &&
                        c != '_' && c != '-')
                    {
                        break;
                    }
                } while (++(*offset) < schemaTextLen);

                /* is this a keywords we just lexed?
                 * XXX: optimize this later, perhaps. */ 
                tok = keywordCheck(schemaText + startOffset,
                                   *offset - startOffset);
                goto lexed;
            }
            case '"': {
                tok = orderly_lex_json_string(lexer, schemaText,
                                              schemaTextLen, offset);
                goto lexed;
            }
            case '[': {
                /* A json array! let's scan past the bugger */
                tok = orderly_lex_json_array(lexer, schemaText,
                                             schemaTextLen, offset);
                goto lexed;
            }
            case '=':
                /* '=' is not returned by the lexer to parser */
                startOffset++; 

                /* The assignment operator is followed by a json object.
                 * first, we'll skip whitespace, then we'll pass it to a
                 * function capable of scanning the value */
                while (*offset < schemaTextLen) {
                    c = schemaText[*offset];
                    if (NULL == strchr("\t\v\f\r\n ", c)) {
                        break;
                    }
                    startOffset++; 
                    (*offset)++;
                }
                tok = orderly_lex_json_value(lexer, schemaText,
                                             schemaTextLen, offset);
                goto lexed;
            case '-': 
            case '0': case '1': case '2': case '3': case '4':  
            case '5': case '6': case '7': case '8': case '9': { 
                /* integer parsing wants to start from the beginning */
                (*offset)--;
                tok = orderly_lex_number(lexer,
                                         (const unsigned char *) schemaText,
                                         schemaTextLen, offset);
                goto lexed;
            }
            default:
                lexer->error = orderly_lex_invalid_char;
                tok = orderly_tok_error;
                goto lexed;
        }
    }

  lexed:
    if (tok != orderly_tok_error) {
        unsigned int ol = *offset - startOffset;
        if (outBuf) *outBuf = schemaText + startOffset;
        if (outLen) *outLen = ol;
        lexer->previousOffset = previousOffset;
    }

    return tok;
}

const char *
orderly_lex_error_to_string(orderly_lex_error error)
{
    switch (error) {
        case orderly_lex_e_ok:
            return "ok, no error";
        case orderly_lex_invalid_char:
            return "invalid character in input schema";
        case orderly_lex_not_implemented:
            return "lexing implementation incomplete";
        case orderly_lex_unterminated_string:
            return "unterminated string encountered";
        case orderly_lex_unterminated_array:
            return "unterminated array encountered";
        case orderly_lex_missing_integer_after_minus:
            return "malformed number, a digit is required after the "
                "minus sign.";
        case orderly_lex_missing_integer_after_decimal:
            return "malformed number, a digit is required after the "
                "decimal point.";
        case orderly_lex_missing_integer_after_exponent:
            return "malformed number, a digit is required after the exponent.";
    }
    return "unknown error code";
}


/** allows access to more specific information about the lexical
 *  error when orderly_lex_lex returns orderly_tok_error. */
orderly_lex_error
orderly_lex_get_error(orderly_lexer lexer)
{
    if (lexer == NULL) return (orderly_lex_error) -1;
    return lexer->error;
}

orderly_tok orderly_lex_peek(orderly_lexer lexer,
                             const unsigned char * jsonText,
                             unsigned int jsonTextLen, unsigned int offset)
{
    orderly_tok t;
    unsigned int fo = lexer->previousOffset;
    t = orderly_lex_lex(lexer, jsonText, jsonTextLen, &offset, NULL, NULL);
    lexer->previousOffset = fo;
    return t;
}


unsigned int
orderly_lex_previous_offset(orderly_lexer lexer)
{
    return lexer->previousOffset;
}
