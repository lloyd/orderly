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

#ifndef __ORDERLY_PARSE_H__
#define __ORDERLY_PARSE_H__

#include "api/common.h"
#include "api/node.h"

typedef enum {
    orderly_parse_s_ok,
    /** An error arose due to the fact that the orderly parser is not completely implemented */
    orderly_parse_s_not_implemented,
    /** A property name was expected, but none was found */
    orderly_parse_s_prop_name_expected,
    /** A '>' was expected (as after specification of a 'required' parameter: 'string foo <bar>;') */
    orderly_parse_s_gt_expected,
    /** A syntax error was discovered in the name of a property */
    orderly_parse_s_prop_name_syntax_error,
    /** A schema entry was expected (an element description such as 'string foo;',
     *  but there was none */
    orderly_parse_s_expected_schema_entry,
    /** After a successful parse we found something other than end of input */
    orderly_parse_s_junk_at_end_of_input,
    /** Range specification (i.e {0,10}) had a syntax error */    
    orderly_parse_s_malformed_range,
    /** encountered an integer too large to be parsed */    
    orderly_parse_s_integer_overflow,
    /** encountered a floating point number that couldn't be parsed */    
    orderly_parse_s_numeric_parse_error,
    /** A '{' was expected (as in an object specification: 'object { string foo; } bar;') */
    orderly_parse_s_left_curly_expected,
    orderly_parse_s_right_curly_expected,
    orderly_parse_s_right_bracket_expected,
    orderly_parse_s_invalid_json,
    orderly_parse_s_backtick_expected,
    /** error codes of 1000 or greater represent lexographic errors */
    orderly_parse_s_lex_error = 1000,
    /** error codes of 10000 or greater represent jsonschema parsing errors */
    orderly_parse_s_jsonschema_error = 10000,
    /** error codes of 20000 or greater represent regex parsing errors */
    orderly_parse_s_regex_error = 20000

} orderly_parse_status;

orderly_parse_status
orderly_parse(orderly_alloc_funcs * alloc,
              const unsigned char * schemaText,
              const unsigned int schemaTextLen,
              const char **error_message,
              orderly_node ** n,
              unsigned int * final_offset);


#endif
