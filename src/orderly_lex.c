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
    /* the overal line and char offset into the data */
    unsigned int lineOff;
    unsigned int charOff;
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
    /* line offset is base 1 */
    lxr->lineOff = 1;
    return lxr;
}

void
orderly_lex_free(orderly_lexer lxr)
{
    OR_FREE(lxr->alloc, lxr);
    return;
}

/* read a character updating counts as appropriate */

    

orderly_tok
orderly_lex_lex(orderly_lexer lexer, const unsigned char * schemaText,
                unsigned int schemaTextLen, unsigned int * offset,
                const unsigned char ** outBuf, unsigned int * outLen)
{
    orderly_tok tok = orderly_tok_error;
    unsigned char c;
    unsigned int startOffset = *offset;

    *outBuf = NULL;
    *outLen = 0;

    for (;;) {
        assert(*offset <= schemaTextLen);

        if (*offset >= schemaTextLen) {
            tok = orderly_tok_eof;
            goto lexed;
        }

        c = schemaText[(*offset)++];

        switch (c) {
            case '{':
                tok = orderly_tok_left_curly;
                goto lexed;
            case '}':
                tok = orderly_tok_right_curly;
                goto lexed;
            case '[':
                tok = orderly_tok_left_bracket;
                goto lexed;
            case ']':
                tok = orderly_tok_right_bracket;
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
            case '\t': case '\v': case '\f': case '\r': case ' ': 
                lexer->charOff++;
                startOffset++; 
                break; 
            case '\n':
                lexer->charOff = 0;
                lexer->lineOff += 1;
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
                    /* XXX: implement me! */
                    tok = orderly_tok_error;
                    goto lexed;
                }
                /* intentional fallthrough */
            case '#':
                /* comment!  ignore until end-of-line */
                while (++(*offset) < schemaTextLen) {
                    if ('\n' == schemaText[*offset]) break;
                }
                if ('\n' == schemaText[*offset]) break;
 
                /* oops!  we hit eof *before* we hit newline */
                tok = orderly_tok_eof;
                goto lexed;
                
/*             case 't': { */
/*                 const char * want = "rue"; */
/*                 do { */
/*                     if (*offset >= jsonTextLen) { */
/*                         tok = orderly_tok_eof; */
/*                         goto lexed; */
/*                     } */
/*                     c = readChar(lexer, jsonText, offset); */
/*                     if (c != *want) { */
/*                         unreadChar(lexer, offset); */
/*                         lexer->error = orderly_lex_invalid_string; */
/*                         tok = orderly_tok_error; */
/*                         goto lexed; */
/*                     } */
/*                 } while (*(++want)); */
/*                 tok = orderly_tok_bool; */
/*                 goto lexed; */
/*             } */
/*             case 'f': { */
/*                 const char * want = "alse"; */
/*                 do { */
/*                     if (*offset >= jsonTextLen) { */
/*                         tok = orderly_tok_eof; */
/*                         goto lexed; */
/*                     } */
/*                     c = readChar(lexer, jsonText, offset); */
/*                     if (c != *want) { */
/*                         unreadChar(lexer, offset); */
/*                         lexer->error = orderly_lex_invalid_string; */
/*                         tok = orderly_tok_error; */
/*                         goto lexed; */
/*                     } */
/*                 } while (*(++want)); */
/*                 tok = orderly_tok_bool; */
/*                 goto lexed; */
/*             } */
/*             case 'n': { */
/*                 const char * want = "ull"; */
/*                 do { */
/*                     if (*offset >= jsonTextLen) { */
/*                         tok = orderly_tok_eof; */
/*                         goto lexed; */
/*                     } */
/*                     c = readChar(lexer, jsonText, offset); */
/*                     if (c != *want) { */
/*                         unreadChar(lexer, offset); */
/*                         lexer->error = orderly_lex_invalid_string; */
/*                         tok = orderly_tok_error; */
/*                         goto lexed; */
/*                     } */
/*                 } while (*(++want)); */
/*                 tok = orderly_tok_null; */
/*                 goto lexed; */
/*             } */
/*             case '"': { */
/*                 tok = orderly_lex_string(lexer, (const unsigned char *) jsonText, */
/*                                       jsonTextLen, offset); */
/*                 goto lexed; */
/*             } */
/*             case '-': */
/*             case '0': case '1': case '2': case '3': case '4':  */
/*             case '5': case '6': case '7': case '8': case '9': { */
/*                 /\* integer parsing wants to start from the beginning *\/ */
/*                 unreadChar(lexer, offset); */
/*                 tok = orderly_lex_number(lexer, (const unsigned char *) jsonText, */
/*                                       jsonTextLen, offset); */
/*                 goto lexed; */
/*             } */
            default:
                lexer->error = orderly_lex_invalid_char;
                tok = orderly_tok_error;
                goto lexed;
        }
    }


  lexed:
    /* need to append to buffer if the buffer is in use or
     * if it's an EOF token */
    if (tok != orderly_tok_error) {
        *outBuf = schemaText + startOffset;
        *outLen = *offset - startOffset;
        lexer->charOff += *outLen;
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

unsigned int orderly_lex_current_line(orderly_lexer lexer)
{
    return lexer->lineOff;
}

unsigned int orderly_lex_current_char(orderly_lexer lexer)
{
    return lexer->charOff;
}

orderly_tok orderly_lex_peek(orderly_lexer lexer,
                             const unsigned char * jsonText,
                             unsigned int jsonTextLen, unsigned int offset)
{
    const unsigned char * outBuf;
    unsigned int outLen;
    orderly_tok tok;
    
    tok = orderly_lex_lex(lexer, jsonText, jsonTextLen, &offset,
                          &outBuf, &outLen);
    
    return tok;
}
