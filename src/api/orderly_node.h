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

#ifndef __ORDERLY_NODE_H__
#define __ORDERLY_NODE_H__

#include "orderly_common.h"

typedef enum {
    orderly_node_empty,
    orderly_node_null
} orderly_node_type;

typedef struct {
    orderly_node_type t;
    const char * name;
    /* a json array of possible values
     * XXX: sure not doing our validators any favors by leaving
     *      this as a blob of text.  otoh, we'd orderly to be
     *      *dependent* on a means of parsing and representing json
     *      to make this more convenient */
    const char * values;
    /* a json representation of this members default value */
    const char * default_value;
    /* is this node optional? */
    unsigned int optional;
/*
    union 
    {
    } u;
*/
} orderly_node;

void orderly_free_node(orderly_alloc_funcs * alloc,
                       orderly_node ** node);

orderly_node * orderly_alloc_node(orderly_alloc_funcs * alloc,
                                  orderly_node_type t);

#endif
