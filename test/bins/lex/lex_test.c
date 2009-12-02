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

#include "../../../src/orderly_lex.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define MAX_INPUT_TEXT (1 << 20)

static const char * tokToStr(orderly_tok t)
{
    switch (t) {
        case orderly_tok_error: return "error";
        case orderly_tok_eof: return "eof";
        case orderly_tok_semicolon:     return "semicolon";
        case orderly_tok_left_curly: return "l_curly";
        case orderly_tok_right_curly: return "r_curly";
        case orderly_tok_lt:     return "lt";
        case orderly_tok_gt: return "gt";
        case orderly_tok_comma: return "comma";
        case orderly_tok_kw_string: return "kw_string";
        case orderly_tok_kw_integer: return "kw_integer";
        case orderly_tok_kw_number: return "kw_number";
        case orderly_tok_kw_boolean: return "kw_boolean";
        case orderly_tok_kw_null: return "kw_null";
        case orderly_tok_kw_any: return "kw_any";
        case orderly_tok_kw_array: return "kw_array";
        case orderly_tok_kw_object: return "kw_object";
        case orderly_tok_kw_union: return "kw_union";
        case orderly_tok_property_name: return "property_name";
        case orderly_tok_json_string: return "json_string";
        case orderly_tok_json_array: return "json_array";
        case orderly_tok_json_number: return "json_number";
        case orderly_tok_json_integer: return "json_integer";
        case orderly_tok_optional_marker: return "optional_marker";
        case orderly_tok_default_value: return "default_value";
        case orderly_tok_regex: return "regex";
    }
    return "unknown";
}

/* horribly inefficient, but this is a test program, WE DONT CARE */
static void
currentLineAndChar(const char * buf, unsigned int off,
                   unsigned int * l, unsigned int * c)
{
    unsigned int i;

    *l = 1; *c = 0;
    
    for (i=0; i < off; i++) {
        if (buf[i] == '\n') {
            *l += 1;
            *c = 0;
        } else {
            *c +=1;
        }
    }
}

int
main(int argc, char ** argv) 
{
    /* this is not a stream lexer, let's read as much as we can up to a
     * reasonable limit  (1 meg) */
    static char inbuf[MAX_INPUT_TEXT];
    size_t tot = 0, rd;
    while (0 < (rd = read(0, (void *) (inbuf + tot), MAX_INPUT_TEXT)))
        tot += rd;

    {
        orderly_tok t;
        unsigned int off = 0;
        orderly_lexer lexer = orderly_lex_alloc(NULL);
        const unsigned char * outBuf = NULL;
        unsigned int outLen = 0;

        do {
            unsigned int l, c;

            t = orderly_lex_lex(lexer, (const unsigned char *) inbuf,
                                tot, &off, &outBuf, &outLen);
            currentLineAndChar(inbuf, off, &l, &c);
            printf("(%3u,%3u): '", l, c);
            fwrite(outBuf, sizeof(char), outLen, stdout);
            printf("' %s\n", tokToStr(t));
            
        } while (t != orderly_tok_error && t != orderly_tok_eof);
    }
    
    return 0;
}
