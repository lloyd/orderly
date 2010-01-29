/*
 * Copyright 2009, 2010, Lloyd Hilaiel.
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

#include "api/node.h"
#include "orderly_alloc.h"
#include "orderly_json.h"

#include <stdlib.h>
#include <string.h>

void orderly_free_node(const orderly_alloc_funcs * alloc,
                       orderly_node ** node)
{
    if (node && *node) {
        if ((*node)->name) OR_FREE(alloc, (void *)((*node)->name));
        if ((*node)->values) orderly_free_json(alloc, &((*node)->values));
        if ((*node)->default_value) {
            orderly_free_json(alloc, &((*node)->default_value));
        }
        if ((*node)->requires) {
            const char ** p;
            for (p = (*node)->requires; *p; p++) {
                OR_FREE(alloc, (void *) *p);
            }
            OR_FREE(alloc, (void *)((*node)->requires));
        }
        if ((*node)->regex) OR_FREE(alloc, (void *)((*node)->regex));
        if ((*node)->passthrough_properties) {
            orderly_free_json(alloc, &((*node)->passthrough_properties));
        }
        if ((*node)->child) orderly_free_node(alloc, &((*node)->child));
        if ((*node)->sibling) orderly_free_node(alloc, &((*node)->sibling));
        OR_FREE(alloc, *node);
        *node = NULL;
    }
    
}

orderly_node * orderly_alloc_node(const orderly_alloc_funcs * alloc,
                                  orderly_node_type t)
{
    orderly_node * n = (orderly_node *)
        OR_MALLOC(alloc, sizeof(orderly_node));
    memset((void *) n, 0, sizeof(orderly_node));
    n->t = t;
    return n;
}

const char * orderly_node_type_to_string(orderly_node_type t)
{
    const char * type = NULL;
    switch (t) {
        case orderly_node_empty: type = "empty"; break;
        case orderly_node_null: type = "null"; break;
        case orderly_node_string: type = "string"; break;
        case orderly_node_boolean: type = "boolean"; break;
        case orderly_node_any: type = "any"; break;
        case orderly_node_integer: type = "integer"; break;
        case orderly_node_number: type = "number"; break;
        case orderly_node_object: type = "object"; break;
        case orderly_node_array: type = "array"; break;
        case orderly_node_union: type = "union"; break;
    }
    return type;
}

orderly_node_type orderly_string_to_node_type(const char * type,
                                              unsigned int typeLen)
{
    orderly_node_type t = orderly_node_empty;
    if (type == NULL || typeLen == 0) return t;

    if (!strncmp(type, "empty", typeLen)) {
        t = orderly_node_null;
    } else if (!strncmp(type, "null", typeLen)) {
        t = orderly_node_null;
    } else if (!strncmp(type, "string", typeLen)) {
        t = orderly_node_string;
    } else if (!strncmp(type, "boolean", typeLen)) {
        t = orderly_node_boolean;
    } else if (!strncmp(type, "any", typeLen)) {
        t = orderly_node_any;
    } else if (!strncmp(type, "integer", typeLen)) {
        t = orderly_node_integer;
    } else if (!strncmp(type, "number", typeLen)) {
        t = orderly_node_number;
    } else if (!strncmp(type, "object", typeLen)) {
        t = orderly_node_object;
    } else if (!strncmp(type, "array", typeLen)) {
        t = orderly_node_array;
    } else if (!strncmp(type, "union", typeLen)) {
        t = orderly_node_union;
    }
    return t;
}
