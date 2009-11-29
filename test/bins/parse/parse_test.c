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

#include "../../../src/orderly_parse.h"
#include "../../../src/orderly_alloc.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_TEXT (1 << 20)

static void dumpNode(orderly_node * n, unsigned int indent)
{
    char * indentStr = (char *) malloc(indent * 4 + 1);
    memset((void *) indentStr, ' ', indent);
    indentStr[indent] = 0;

    if (n) {
        const char * type = "unknown";
        switch (n->t) {
            case orderly_node_empty: type = "empty"; break;
            case orderly_node_null: type = "null"; break;
            case orderly_node_string: type = "string"; break;
            case orderly_node_boolean: type = "boolean"; break;
            case orderly_node_any: type = "any"; break;
        }
        printf("%s%s [%s] %s\n", indentStr, n->name, type,
               n->optional ? "OPTIONAL" : "");        
        if (n->default_value) printf("%s--> default: %s\n",
                                        indentStr, n->default_value);        
        if (n->values) printf("%s--> enum: %s\n", indentStr, n->values);        
        if (n->requires) printf("%s--> requires: %s\n", indentStr, n->requires);        
        if (n->regex) printf("%s--> regex: %s\n", indentStr, n->regex);        
    } else {
        printf("%s(null)\n", indentStr);
    }
    free(indentStr);
}

static const char * statusToStr(orderly_parse_status s)
{
    switch (s) {
        case orderly_parse_s_ok: return "ok";
        case orderly_parse_s_not_implemented: return "not_implemented";
        case orderly_parse_s_prop_name_expected: return "prop_name_expected";
        case orderly_parse_s_gt_expected: return "gt_expected";
        case orderly_parse_s_prop_name_syntax_error: return "prop_name_syntax_error";
        case orderly_parse_s_expected_schema_entry: return "expected_schema_entry";
        case orderly_parse_s_junk_at_end_of_input: return "junk_at_end_of_input";
        case orderly_parse_s_malformed_range: return "malformed_range";
    }
    return "unknown";
}


int
main(int argc, char ** argv) 
{
    /* this is not a stream lexer, let's read as much as we can up to a
     * reasonable limit  (1 meg) */
    static unsigned char inbuf[MAX_INPUT_TEXT];
    size_t tot = 0, rd;
    while (0 < (rd = read(0, (void *) (inbuf + tot), MAX_INPUT_TEXT)))
        tot += rd;

    {
        orderly_alloc_funcs oaf;
        orderly_node * n;
        orderly_parse_status s;

        orderly_set_default_alloc_funcs(&oaf);

        s = orderly_parse(&oaf, inbuf, tot, &n);
        printf("parse complete (%s):\n", statusToStr(s));
        dumpNode(n, 0); /* here's where we'll map over and output the returned tree */ 
        /* TODO: ugly alloc routine crap here, perhaps we should give the
         *       ultimate client a parse handle and make the ownership of
         *       the parse tree held by the parse handle? */
        orderly_free_node(&oaf, &n);
    }
    
    return 0;
}
