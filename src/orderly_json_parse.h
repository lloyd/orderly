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

#ifndef __ORDERLY_JSON_PARSE_H__
#define __ORDERLY_JSON_PARSE_H__

#include "api/common.h"
#include "api/node.h"

typedef enum {
    orderly_json_parse_s_ok = 0,
    orderly_json_parse_s_object_expected,
    orderly_json_parse_s_internal_error,
    orderly_json_parse_s_type_expects_string_or_array,
    orderly_json_parse_s_invalid_type_value,
    orderly_json_parse_s_invalid_properties_value,
    orderly_json_parse_s_invalid_json,
    orderly_json_parse_s_invalid_optional_value,
    orderly_json_parse_s_minimum_requires_number,
    orderly_json_parse_s_maximum_requires_number,
    orderly_json_parse_s_minlength_requires_integer,
    orderly_json_parse_s_maxlength_requires_integer,
    orderly_json_parse_s_minitems_requires_integer,
    orderly_json_parse_s_maxitems_requires_integer
} orderly_json_parse_status;

orderly_json_parse_status
orderly_json_parse(orderly_alloc_funcs * alloc,
                   const unsigned char * schemaText,
                   const unsigned int schemaTextLen,
                   orderly_node ** n,
                   unsigned int * final_offset);

#endif
