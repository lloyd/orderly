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

#include "orderly_json_parse.h"

#include <yajl/yajl_parse.h>

#include <stdio.h>

static int js_parse_null(void * ctx)
{
    printf("js_parse_null\n");
    return 1;
}

 
static int js_parse_boolean(void * ctx, int boolean)
{
    printf("js_parse_boolean\n");
    return 1;
}

 
static int js_parse_number(void * ctx, const char * s, unsigned int l)
{
    printf("js_parse_number\n");
    return 1;
}

 
static int js_parse_string(void * ctx, const unsigned char * stringVal,
                           unsigned int stringLen)
{
    printf("js_parse_string\n");
    return 1;
}

 
static int js_parse_map_key(void * ctx, const unsigned char * stringVal,
                            unsigned int stringLen)
{
    printf("js_parse_map\n");
    return 1;
}

 
static int js_parse_start_map(void * ctx)
{
    printf("js_parse_start\n");
    return 1;
}
 
 
static int js_parse_end_map(void * ctx)
{
    printf("js_parse_end\n");
    return 1;
}

 
static int js_parse_start_array(void * ctx)
{
    printf("js_parse_map\n");
    return 1;
}

 
static int js_parse_end_array(void * ctx)
{
    printf("js_parse_map\n");
    return 1;
}


orderly_json_parse_status
orderly_json_parse(orderly_alloc_funcs * alloc,
                   const unsigned char * schemaText,
                   const unsigned int schemaTextLen,
                   orderly_node ** n)
{
    static yajl_callbacks callbacks = {
        js_parse_null,
        js_parse_boolean,
        NULL,
        NULL,
        js_parse_number,
        js_parse_string,
        js_parse_start_map,
        js_parse_map_key,
        js_parse_end_map,
        js_parse_start_array,
        js_parse_end_array
    };


    yajl_handle hand;
    yajl_status stat;
    /* allow comments! */
    yajl_parser_config cfg = { 1, 1 };
    
    /* allocate a parser */
    hand = yajl_alloc(&callbacks, &cfg,
                      (const yajl_alloc_funcs *) alloc,
                      NULL);
        
    /* read file data, pass to parser */
    stat = yajl_parse(hand, schemaText, schemaTextLen);
 
    if (stat == yajl_status_insufficient_data)
    {
    }
    else if (stat != yajl_status_ok)
    {
        unsigned char * str = yajl_get_error(hand, 1, schemaText, schemaTextLen);
        fprintf(stderr, (const char *) str);
        yajl_free_error(hand, str);
    }
    else 
    {
        /* we're ok! */
    }
    
    
    return orderly_json_parse_s_ok;
}
