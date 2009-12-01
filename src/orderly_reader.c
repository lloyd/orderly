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

struct orderly_reader_t 
{
    orderly_alloc_funcs alloc;
    orderly_node * node;
    orderly_parse_status status;
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
                              len, &(r->node));
    
    return r->node;
}
