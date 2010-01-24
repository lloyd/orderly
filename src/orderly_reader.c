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

#include "api/reader.h"
#include "api/node.h"
#include "orderly_buf.h"

#include "orderly_parse.h"
#include "orderly_json_parse.h"
#include "orderly_lex.h"

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
  /* used to store error messages from orderly_parse */
    const char *error_message;
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
    /* We cannot set set an error code, return NULL */
    if (r == NULL) return NULL;
    
    /* if this handle has been used before, now is the time
     * to free the parse tree */
    if (r->node) orderly_free_node(&(r->alloc), &(r->node));

    if (fmt == ORDERLY_JSONSCHEMA) {
        r->finalOffset = 0;
        r->status = orderly_json_parse(&(r->alloc), (const unsigned char *) schema,
                                       len, 
                                       &(r->error_message),
                                       &(r->node), &(r->finalOffset));
        if (r->status) r->status += orderly_parse_s_jsonschema_error;
    } else if (fmt == ORDERLY_TEXTUAL) {
        r->status = orderly_parse(&(r->alloc), (const unsigned char *) schema,
                                  len, 
                                  &(r->error_message),
                                  &(r->node), &(r->finalOffset));
    } else {
        /** XXX: we'll try orderly first, then jsonschema */ 
        r->status = orderly_parse(&(r->alloc), (const unsigned char *) schema,
                                  len,
                                  &(r->error_message),
                                  &(r->node), &(r->finalOffset));
        if (r->status != orderly_parse_s_ok) {
            r->status = orderly_json_parse(&(r->alloc), (const unsigned char *) schema,
                                           len, 
                                           &(r->error_message),
                                           &(r->node), &(r->finalOffset));
            if (r->status) r->status += orderly_parse_s_jsonschema_error;
        }
    }
    
    return r->node;
}


const char *
orderly_get_error(orderly_reader r)
{
    const char * err = "unknown error.";

    if (r) {
        if (r->status < orderly_parse_s_lex_error) {
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
                case orderly_parse_s_lex_error:
                case orderly_parse_s_jsonschema_error:
                case orderly_parse_s_regex_error:
                    /* outer if should keep this code from ever executing */
                    err = "internal error";
                    break;
                case orderly_parse_s_right_bracket_expected:
                    err = "expected a right square bracket ']'";
                    break;
                case orderly_parse_s_invalid_json:
                    err = "invalid json";
                    break;
                case orderly_parse_s_backtick_expected:
                    err = "expected a closing backtick: '`'";
                    break;
            }
        } else if (r->status < orderly_parse_s_jsonschema_error) {
            switch ((orderly_lex_error) r->status - orderly_parse_s_lex_error) {
                case orderly_lex_invalid_char:
                    err = "invalid character found in input text";
                    break;
                case orderly_lex_not_implemented:
                    err = "internal error (language feature not yet implemented by orderly lexer)";
                    break;
                case orderly_lex_unterminated_string:
                    err = "unterminated string";
                    break;
                case orderly_lex_unterminated_array:
                    err = "unterminated array";
                    break;
                case orderly_lex_missing_integer_after_minus:
                    err = "missing integer after minus sign ('-')";
                    break;
                case orderly_lex_missing_integer_after_decimal:
                    err = "missing integer after decimal point";
                    break;
                case orderly_lex_missing_integer_after_exponent:
                    err = "missing integer after exponent marker ('e' or 'E')";
                    break;
            }
        } else if ( (r->status < orderly_parse_s_regex_error) ) {
            switch ((orderly_json_parse_status) r->status - orderly_parse_s_jsonschema_error) {
                case orderly_json_parse_s_ok:
                    err = "no error, it's all good.";
                    break;
                case orderly_json_parse_s_object_expected:
                    err = "expected a json schema (which is an object)";
                    break;
                case orderly_json_parse_s_internal_error:
                    err = "internal error";
                    break;
                case orderly_json_parse_s_type_expects_string_or_array:
                    err = "the type property requires either a string or array value";
                    break;
                case orderly_json_parse_s_invalid_type_value:
                    err = "invalid type specified";
                    break;
                case orderly_json_parse_s_invalid_properties_value:
                    err = "the 'properties' property expects an object value";
                    break;
                case orderly_json_parse_s_invalid_json:            
                    err = "json syntax error";
                    break;
                case orderly_json_parse_s_invalid_optional_value:
                    err = "'optional' property requires a boolean value";
                    break;
                case orderly_json_parse_s_minimum_requires_number:
                    err = "'minimum' property requires a numeric value";
                    break;
                case orderly_json_parse_s_maximum_requires_number:
                    err = "'maximum' property requires an integer";
                    break;
                case orderly_json_parse_s_minlength_requires_integer:
                    err = "'minLength' property requires an integer";
                    break;
                case orderly_json_parse_s_maxlength_requires_integer:
                    err = "'maxLength' property requires an integer";
                    break;
                case orderly_json_parse_s_minitems_requires_integer:
                    err = "'minItems' property requires an integer";
                    break;
                case orderly_json_parse_s_maxitems_requires_integer:
                    err = "'maxItems' property requires an integer";
                    break;
                case orderly_json_parse_s_items_gets_object_or_array:
                    err = "'items' property requires schema or array of schemas";
                    break;
                case orderly_json_parse_s_addprop_requires_boolean:
                    err = "'additionalProperties' property requires boolean value";
                    break;
                case orderly_json_parse_s_pattern_requires_string:
                    err = "'pattern' property requires a string value";
                    break;
                case orderly_json_parse_s_regex_error:
                    err = "'pattern' property requires a string value";
                    break;
            }
        } else { /** regex error */
          err = r->error_message;
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
