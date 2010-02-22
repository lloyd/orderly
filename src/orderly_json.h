/*
 * Copyright 2010, Greg Olszewski and Lloyd Hilaiel.
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
 *  3. Neither the name of Greg Olszewski and Lloyd Hilaiel nor the names of its
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

/* a lightweight C representation of json data */

#ifndef __ORDERLY_JSON_IMPL_H__
#define __ORDERLY_JSON_IMPL_H__

#include "api/json.h"
#include "api/common.h"
#include "orderly_ptrstack.h"
#include "orderly_buf.h"

#include <yajl/yajl_gen.h>

/* a high level interface to non-stream based json parsing */
orderly_json * orderly_read_json(orderly_alloc_funcs * alloc,
                                 const char * jsonText,
                                 unsigned int * len);

/* a high level interface to non-stream based json parsing */
void orderly_write_json(const orderly_alloc_funcs * alloc,
                        const orderly_json * json,
                        orderly_buf buf,
                        int pretty);

/* a low level interface to dumping a json object into a yajl
 * generator */
int orderly_write_json2(yajl_gen g, const orderly_json * j);

void orderly_free_json(const orderly_alloc_funcs * alloc, orderly_json ** node);

orderly_json * orderly_alloc_json(const orderly_alloc_funcs * alloc,
                                  orderly_json_type t);

/* make a deep copy of a json object, copying everything except sibling
 * pointers */
orderly_json * orderly_clone_json(const orderly_alloc_funcs * alloc,
                                  orderly_json * j);

/* callbacks capable of building up orderly_json objects from yajl parse
 * events, used by orderly_json_parse to build json structures directly out
 * of the parse event stream */
typedef struct o_json_parse_context_t 
{
    /* context for parsing nothing more than a stack and allocation
     * functions */
    orderly_alloc_funcs * alloc;
    orderly_ptrstack nodeStack;
    orderly_ptrstack keyStack;
} o_json_parse_context;
    
int o_json_parse_start_array(void * ctx);
int o_json_parse_end_array(void * ctx);
int o_json_parse_start_map(void * ctx);
int o_json_parse_map_key(void * ctx, const unsigned char * v,
                         unsigned int l);
int o_json_parse_end_map(void * ctx);
int o_json_parse_string(void * ctx, const unsigned char * v, unsigned int l);
int o_json_parse_integer(void * ctx, long l);
int o_json_parse_double(void * ctx, double d);
int o_json_parse_null(void * ctx);
int o_json_parse_boolean(void * ctx, int val);

#endif
