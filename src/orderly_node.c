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

#include "api/orderly_node.h"
#include "orderly_alloc.h"

#include <stdlib.h>
#include <string.h>

void orderly_free_node(orderly_alloc_funcs * alloc,
                       orderly_node ** node)
{
    if (node && *node) {
        if ((*node)->name) OR_FREE(alloc, (void *)((*node)->name));
        if ((*node)->values) OR_FREE(alloc, (void *)((*node)->values));
        if ((*node)->default_value) OR_FREE(alloc, (void *)((*node)->default_value));
        if ((*node)->requires) OR_FREE(alloc, (void *)((*node)->requires));
        OR_FREE(alloc, *node);
        *node = NULL;
    }
    
}

orderly_node * orderly_alloc_node(orderly_alloc_funcs * alloc,
                                  orderly_node_type t)
{
    orderly_node * n = (orderly_node *)
        OR_MALLOC(alloc, sizeof(orderly_node));
    memset((void *) n, 0, sizeof(orderly_node));
    n->t = t;
    return n;
}
