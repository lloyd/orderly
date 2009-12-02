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

#include "api/reader.h"
#include "api/node.h"
#include "orderly_buf.h"


#include "orderly_parse.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct orderly_reader_t 
{
    orderly_alloc_funcs alloc;
    orderly_node * node;
    orderly_parse_status status;
    /* how far we got in the parse, useful in error reporting */
    unsigned int finalOffset;
    /* when the client wants a nice error string, we'll stuff it in the
     * buf */
    orderly_buf errBuf;
};

orderly_reader
orderly_reader_new(const orderly_alloc_funcs * alloc)
{
    orderly_reader rdr = NULL;

    {
        static orderly_alloc_funcs orderlyAllocFuncBuffer;
        static orderly_alloc_funcs * orderlyAllocFuncBufferPtr = NULL;

        if (orderlyAllocFuncBufferPtr == NULL) {
            orderly_set_default_alloc_funcs(&orderlyAllocFuncBuffer);
            orderlyAllocFuncBufferPtr = &orderlyAllocFuncBuffer;
        }
        if (alloc == NULL) alloc = orderlyAllocFuncBufferPtr;
    }

    rdr = OR_MALLOC(alloc, sizeof(struct orderly_reader_t));
    memcpy((void *) (&(rdr->alloc)), (void *) alloc,
           sizeof(orderly_alloc_funcs));
    rdr->node = NULL;
    rdr->status = orderly_parse_s_ok;
    rdr->errBuf = NULL;
    rdr->finalOffset = 0;

    return rdr;
}


void
orderly_reader_free(orderly_reader *r)
{
    if (r && *r) {
        if ((*r)->node) {
            orderly_free_node(&((*r)->alloc), &((*r)->node));
        }
        OR_FREE(&((*r)->alloc), (*r));
        *r = NULL;
    }
}

const orderly_node * 
orderly_read(orderly_reader r, orderly_format fmt,
             const char * schema, unsigned int len)
{
    /* XXX: set an error code? */
    if (r == NULL) return NULL;
    
    /* if this handle has been used before, now is the time
     * to free the parse tree */
    if (r->node) orderly_free_node(&(r->alloc), &(r->node));
    
    r->status = orderly_parse(&(r->alloc), (const unsigned char *) schema,
                              len, &(r->node), &(r->finalOffset));
    
    return r->node;
}


const char *
orderly_get_error(orderly_reader r)
{
    const char * err = "unknown error.";

    if (r) {
        switch(r->status) {
            case orderly_parse_s_ok:
                err = "ok (no error)";
                break;
            case orderly_parse_s_not_implemented:
                err = "internal error (language feature not yet implemented by orderly parser)";
                break;
            case orderly_parse_s_prop_name_expected:
                err = "property name expected";
                break;
            case orderly_parse_s_gt_expected:
                err = "greater than '>' expected";
                break;
            case orderly_parse_s_prop_name_syntax_error:
                err = "syntax error inside property name";
                break;
            case orderly_parse_s_expected_schema_entry:
                err = "expected a schema element (i.e. \"string foo;\")";
                break;
            case orderly_parse_s_junk_at_end_of_input:
                err = "unexpected rubbish at end of input";
                break;
            case orderly_parse_s_malformed_range:
                err = "range malformed";
                break;
            case orderly_parse_s_integer_overflow:
                err = "integer overflow!  woah.  big number!";
                break;
            case orderly_parse_s_numeric_parse_error:
                err = "numeric parse error.  thems some funny lookin' digits.";
                break;
            case orderly_parse_s_left_curly_expected:
                err = "expected a left curly brace '{'";
                break;
            case orderly_parse_s_right_curly_expected:
                err = "expected a right curly brace '}'";
                break;
        }
    }
    
    return err;
}


const char *
orderly_get_error_context(orderly_reader r,
                          const char * schema, unsigned int len)
{

    unsigned int offset = r->finalOffset;
    const char * arrow = "                     (right here) ------^\n";    

    r->errBuf = orderly_buf_alloc(&(r->alloc));

    /* now we append as many spaces as needed to make sure the error
     * falls at char 41, if verbose was specified */
    {
        unsigned int start, end, i;
        unsigned int spacesNeeded;

        spacesNeeded = (offset < 30 ? 40 - offset : 10);
        start = (offset >= 30 ? offset - 30 : 0);
        end = (offset + 30 > len ? len: offset + 30);
    
        for (i=0;i<spacesNeeded;i++) {
            orderly_buf_append_string(r->errBuf, " ");
        }
        
        for (;start < end;start++, i++) {
            if (schema[start] != '\n' && schema[start] != '\r')
            {
                orderly_buf_append(r->errBuf, schema + start, 1);
            }
            else
            {
                orderly_buf_append_string(r->errBuf, " ");
            }
        }
        assert(i <= 71);
        orderly_buf_append_string(r->errBuf, "\n");
        orderly_buf_append_string(r->errBuf, arrow);
    }

    return (const char *) orderly_buf_data(r->errBuf);
}


unsigned int
orderly_get_error_offset(orderly_reader r)
{
    return 0;
}
