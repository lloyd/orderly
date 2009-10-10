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

#ifndef __ORDERLY_LEX_H__
#define __ORDERLY_LEX_H__

#include "api/orderly_common.h"

typedef enum {
    orderly_tok_error,
    orderly_tok_eof,
    orderly_tok_semicolon,    
    orderly_tok_left_bracket,
    orderly_tok_left_curly,
    orderly_tok_right_bracket,
    orderly_tok_right_curly,
    orderly_tok_lt,    
    orderly_tok_gt
} orderly_tok;

typedef struct orderly_lexer_t * orderly_lexer;

orderly_lexer orderly_lex_alloc(orderly_alloc_funcs * alloc);

void orderly_lex_free(orderly_lexer lexer);

orderly_tok orderly_lex_lex(orderly_lexer lexer,
                            const unsigned char * schemaText,
                            unsigned int schemaTextLen,
                            unsigned int * offset,
                            const unsigned char ** outBuf,
                            unsigned int * outLen);

/** have a peek at the next token, but don't move the lexer forward */
orderly_tok orderly_lex_peek(orderly_lexer lexer,
                             const unsigned char * jsonText,
                             unsigned int jsonTextLen,
                             unsigned int offset);


typedef enum {
    orderly_lex_e_ok = 0,
    orderly_lex_invalid_char = 1
} orderly_lex_error;

const char * orderly_lex_error_to_string(orderly_lex_error error);

/** allows access to more specific information about the lexical
 *  error when orderly_lex_lex returns orderly_tok_error. */
orderly_lex_error orderly_lex_get_error(orderly_lexer lexer);

/** get the current offset into the most recently lexed json string. */
unsigned int orderly_lex_current_offset(orderly_lexer lexer);

/** get the number of lines lexed by this lexer instance */
unsigned int orderly_lex_current_line(orderly_lexer lexer);

/** get the number of chars lexed by this lexer instance since the last
 *  \n or \r */
unsigned int orderly_lex_current_char(orderly_lexer lexer);

#endif
