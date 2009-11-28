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

#include "orderly_parse.h"
#include "orderly_lex.h"
#include "orderly_alloc.h"

#include <stdlib.h>
#include <string.h>

#define BUF_STRDUP(dst, a, ob, ol)               \
    (dst) = OR_MALLOC((a), (ol) + 1);            \
    memcpy((void *)(dst), (void *) (ob), (ol));  \
    ((char *) (dst))[(ol)] = 0;


static orderly_parse_status
orderly_parse_definition_suffix(orderly_alloc_funcs * alloc,
                                const unsigned char * schemaText,
                                const unsigned int schemaTextLen,
                                orderly_lexer lxr,
                                unsigned int * offset,
                                orderly_node * n)
{
    orderly_tok t;
    const unsigned char * outBuf = NULL;
    unsigned int outLen = 0;

    t = orderly_lex_lex(lxr, schemaText, schemaTextLen, offset, &outBuf, &outLen);    

    /* optional_enum_values? */
    if (t == orderly_tok_json_array) {
        BUF_STRDUP(n->values, alloc, outBuf, outLen);
        t = orderly_lex_lex(lxr, schemaText, schemaTextLen, offset, &outBuf, &outLen);    
    }

    /* optional_default_value? */
    if (t == orderly_tok_default_value) {
        BUF_STRDUP(n->default_value, alloc, outBuf, outLen);
        t = orderly_lex_lex(lxr, schemaText, schemaTextLen, offset, &outBuf, &outLen);    
    }

    /* optional_default_value? */
    if (t == orderly_tok_lt) {
        t = orderly_lex_lex(lxr, schemaText, schemaTextLen, offset, &outBuf, &outLen);            
        if (t != orderly_tok_property_name) {
            return orderly_parse_s_prop_name_expected;
        }
        BUF_STRDUP(n->default_value, alloc, outBuf, outLen);
        t = orderly_lex_lex(lxr, schemaText, schemaTextLen, offset, &outBuf, &outLen);            
        if (t != orderly_tok_gt) {
            return orderly_parse_s_gt_expected;
        }
        t = orderly_lex_lex(lxr, schemaText, schemaTextLen, offset, &outBuf, &outLen);            
    }

    /* optional_default_value? */    
    if (t == orderly_tok_optional_marker) {    
        n->optional = 1;
        t = orderly_lex_lex(lxr, schemaText, schemaTextLen, offset, &outBuf, &outLen);            
    }

    /* "unlex" the last token.  let higher level code deal with it */
    offset -= outLen;
    
    return orderly_parse_s_ok;
}


static orderly_parse_status
orderly_parse_property_name(orderly_alloc_funcs * alloc,
                            const unsigned char * schemaText,
                            const unsigned int schemaTextLen,
                            orderly_lexer lxr,
                            unsigned int * offset,
                            orderly_node * n)
{
    const unsigned char * outBuf = NULL;
    unsigned int outLen = 0;
    orderly_tok t;

    if (n == NULL) return 0;
    t = orderly_lex_lex(lxr, schemaText, schemaTextLen, offset, &outBuf, &outLen);
    
    if (t != orderly_tok_property_name) {
        return orderly_parse_s_prop_name_expected;
    }

    BUF_STRDUP(n->name, alloc, outBuf, outLen);

    return orderly_parse_s_ok;
}


static orderly_parse_status
orderly_parse_named_entry(orderly_alloc_funcs * alloc,
                          const unsigned char * schemaText,
                          const unsigned int schemaTextLen,
                          orderly_lexer lxr,
                          unsigned int * offset,
                          orderly_node ** n)
{
    orderly_tok t;
    orderly_parse_status s;

    t = orderly_lex_lex(lxr, schemaText, schemaTextLen, offset, NULL, NULL);
    
    if (t == orderly_tok_kw_string) {
        return orderly_parse_s_not_implemented;
    } else if (t == orderly_tok_kw_null) {
        *n = orderly_alloc_node(alloc, orderly_node_null);
        if ((s = orderly_parse_property_name(alloc, schemaText, schemaTextLen,
                                             lxr, offset, *n)) || 
            (s = orderly_parse_definition_suffix(alloc, schemaText, schemaTextLen,
                                                 lxr, offset, *n)))
        {
            orderly_free_node(alloc, n);
        }
    } else {
        s = orderly_parse_s_not_implemented;        
    }

    return s;
}

orderly_parse_status
orderly_parse(orderly_alloc_funcs * alloc,
              const unsigned char * schemaText,
              const unsigned int schemaTextLen,
              orderly_node ** n)
{
    unsigned int offset = 0;
    orderly_parse_status s = orderly_parse_s_ok;
    orderly_lexer lxr;

    {
        static orderly_alloc_funcs orderlyAllocFuncBuffer;
        static orderly_alloc_funcs * orderlyAllocFuncBufferPtr = NULL;

        if (orderlyAllocFuncBufferPtr == NULL) {
            orderly_set_default_alloc_funcs(&orderlyAllocFuncBuffer);
            orderlyAllocFuncBufferPtr = &orderlyAllocFuncBuffer;
        }
        if (alloc == NULL) alloc = orderlyAllocFuncBufferPtr;
    }

    *n = NULL;
    
    lxr = orderly_lex_alloc(alloc);
    s = orderly_parse_named_entry(alloc, schemaText, schemaTextLen, lxr,
                                  &offset, n);
    orderly_lex_free(lxr);
    
    return s;
}
